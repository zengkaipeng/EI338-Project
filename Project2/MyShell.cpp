#include <bits/stdc++.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;
typedef vector<char>::iterator vchit;
char skip_space()
{
	char ch = getchar();
	while(ch == ' ') ch = getchar();
	return ch;
}
bool Divide_command(
	const vector<char>&Buffer,
	vector<char> &Pre, vector<char> &Suf,
	int &lspace, int &rspace 
)
{
	int Blen = Buffer.size();
	vector<char> *Now = &Pre;
	int *space_cnt = &lspace;
	bool Is_quo = false, Is_pipe = false;
	for(int i = 0; i < Blen; ++i)
	{
		Now -> push_back(Buffer[i]);
		if(Buffer[i] == ' ' && !Is_quo)
		{
			while((i + 1) != Blen && Buffer[i + 1] == ' ') ++i;
			*space_cnt += 1;
		}
		else
		{
			if(Buffer[i] == '|')
			{
				Now -> pop_back();
				Is_pipe = true; Now = &Suf;
				space_cnt = &rspace;
			}
			Is_quo ^= (Buffer[i] == '"');
		}
	}
	return Is_pipe;
}
inline int convert_to_argv(
	const vector<char>& Buffer, char **argvs, 
	bool &IInput, bool &IOutput,
	char *&Ipdir, char *&Opdir,
	bool &Iswait
)
{
	int currcnt = 0, Blen = Buffer.size(), lst = 0;
	for(int i = 0; i < Blen; ++i)
	{
		if(Buffer[i] == ' ')
		{
			if(lst > 0)
			{
				argvs[currcnt] = new char[lst + 5];
				for(int j = 0; j < lst; ++j)
					argvs[currcnt][j] = Buffer[j + i - lst];
				argvs[currcnt++][lst] = '\0';
			}
			lst = 0;
		}
		else if(Buffer[i] == '"')
		{
			int far = i;
			while(far + 1 < Blen && Buffer[far + 1] != '"') ++far;
			argvs[currcnt] = new char[far - i + 10];
			for(int j = i + 1; j <= far; ++j)
				argvs[currcnt][j - i - 1] = Buffer[j];
			argvs[currcnt][far - i] = '\0';
			++currcnt; i = far + 1; lst = 0;
		}
		else
		{
			if(Buffer[i] == '>')
			{
				IOutput = true;
				if(lst > 0)
				{
					argvs[currcnt] = new char[lst + 5];
					for(int j = 0; j < lst; ++j)
						argvs[currcnt][j] = Buffer[j + i - lst];
					argvs[currcnt++][lst] = '\0';
				}
				while(i + 1 < Blen && Buffer[i + 1] == ' ') ++i; 
				if(++i == Blen) break;
				bool Is_quo = false; lst = 0;
				while(i + lst < Blen)
				{
					if(Buffer[i + lst] == ' ' && !Is_quo) break;
					Is_quo ^= Buffer[i + lst] == '"'; ++lst;
				}
				int ni = i, nlst = lst;
				if(Buffer[i] == '"')
					++ni, nlst -= 2;
				Opdir = new char[lst + 5];
				for(int j = 0; j < nlst; ++j)
					Opdir[j] = Buffer[ni + j];
				Opdir[nlst] = '\0'; 
				i += lst - 1; lst = 0; 
			}
			else if(Buffer[i] == '<')
			{
				IInput = true;
				if(lst > 0)
				{
					argvs[currcnt] = new char[lst + 5];
					for(int j = 0; j < lst; ++j)
						argvs[currcnt][j] = Buffer[j + i - lst];
					argvs[currcnt++][lst] = '\0';
				}
				while(i + 1 < Blen && Buffer[i + 1] == ' ') ++i;
				if(++i == Blen) break;
				bool Is_quo = false; lst = 0;
				while(i + lst < Blen)
				{
					if(Buffer[i + lst] == ' ' && !Is_quo) break;
					Is_quo ^= Buffer[i + lst] == '"', ++lst;
				}
				int ni = i, nlst = lst;
				if(Buffer[i] == '"')
					++ni, nlst -= 2;
				Ipdir = new char[nlst + 5];
				for(int j = 0; j < nlst; ++j)
					Ipdir[j] = Buffer[ni + j];
				Ipdir[nlst] = '\0';
				i += lst - 1; lst = 0; 
			}
			else if(Buffer[i] == '&')
			{
				if(lst > 0)
				{
					argvs[currcnt] = new char[lst + 5];
					for(int j = 0; j < lst; ++j)
						argvs[currcnt][j] = Buffer[j + i - lst];
					argvs[currcnt++][lst] = '\0';
				}
				lst = 0; Iswait = false;
			}
			else lst += 1;
		}
	}
	if(lst > 0)
	{
		argvs[currcnt] = new char[lst + 5];
		for(int j = 0; j < lst; ++j)
			argvs[currcnt][j] = Buffer[j + Blen - lst];
		argvs[currcnt][lst] = '\0'; currcnt += 1;
	}
	argvs[currcnt] = NULL;
	return currcnt;
}
void run_core(
	bool &should_run, bool &run_last,
	bool Is_Input, bool Is_Output,
	int fin, int fout, char **argv,
	bool Is_wait
)
{
	if(strcmp(argv[0], "exit") == 0)
		should_run = false;
	else if(strcmp(argv[0], "!!") == 0)
		run_last = true;
	else
	{
		pid_t pid = fork();
		if(pid == 0)
		{
			if(Is_Input) dup2(fin, STDIN_FILENO);
			if(Is_Output) dup2(fout, STDOUT_FILENO);
			execvp(argv[0], argv);
			exit(0);
		}
		else if(Is_wait) waitpid(pid, NULL, 0);
	}
}
void run_single(
	const vector<char>& Buffer_pre,
	int lspace, bool &should_run,
	bool &run_last
)
{
	bool Is_Input(false), Is_Output(false), Is_wait(true);
	char **argv = new char* [lspace + 15], *opdir = NULL, *ipdir = NULL;
	int cmd_len = convert_to_argv(
		Buffer_pre, argv, Is_Input, 
		Is_Output, ipdir, opdir, Is_wait
	);
	if(Is_Input && ipdir == NULL)
		printf("syntax error near unexpected token `newline'\n");
	else if(Is_Output && opdir == NULL)
		printf("syntax error near unexpected token `newline'\n");
	else
	{
		/*
		cout << cmd_len << endl;
		for(int j = 0; j < cmd_len; ++j)
			printf("argv[%d]: %s\n", j, argv[j]);
		*/
		int fin(-2), fout(-2);
		if(Is_Input) fin = open(ipdir, O_RDONLY);
		if(Is_Output) fout = open(opdir, O_WRONLY | O_CREAT | O_TRUNC, STDIN_FILENO);
		if(fin == -1 || fout == -1)
			printf("File operation Error!\n");
		else run_core(
			should_run, run_last, Is_Input, 
			Is_Output, fin, fout, argv, Is_wait
		);
		if(fin != -1 && fin != -2) close(fin);
		if(fout != -1 && fout != -2) close(fout);
	}
	for(int i = 0; i < cmd_len ;++i)
		delete []argv[i];
	if(Is_Input && ipdir != NULL) delete []ipdir;
	if(Is_Output && opdir != NULL) delete []opdir;
	delete []argv;
}
void run_pipe(
	const vector<char> &Buffer_pre,
	const vector<char> &Buffer_suf,
	int lspace, int rspace,
	bool &run_last, bool &should_run
)
{
	bool Is_Input(false), Is_Output(false), Is_wait(true);
	char **argv = new char* [lspace + 15], *opdir = NULL, *ipdir = NULL;
	int cmd_len = convert_to_argv(
		Buffer_pre, argv, Is_Input, 
		Is_Output, ipdir, opdir, Is_wait
	);
	char **argv2 = new char*[rspace + 15];
	int cmd_len2 = convert_to_argv(
		Buffer_suf, argv2, Is_Input,
		Is_Output, ipdir, opdir, Is_wait
	);
	/*
	cout << cmd_len << endl;
	for(int j = 0; j < cmd_len; ++j)
		printf("argv[%d]: %s\n", j, argv[j]);
	cout << cmd_len2 << endl;
	for(int j = 0; j < cmd_len2; ++j)
		printf("argv2[%d]: %s\n", j, argv2[j]);
	*/
	if(strcmp(argv[0], "exit") == 0)
		should_run = false;
	else if(strcmp(argv[0], "!!") == 0)
		run_last = true;
	else
	{
		pid_t pid = fork();
		if(pid == 0)
		{
			int fd[2] = {0};
			if(pipe(fd) < 0)
				printf("Pipe Error\n");
			else
			{
				pid_t pid2 = fork();
				if(pid2 == 0)
				{
					dup2(fd[1], STDOUT_FILENO);
					execvp(argv[0], argv);
					exit(0);
				}
				else
				{
					wait(NULL); close(fd[1]);
					fcntl(fd[0], F_SETFL, O_NONBLOCK);
					if(strcmp(argv2[0], "exit") == 0)
						should_run = false;
					else if(strcmp(argv2[0], "!!") != 0)
					{
						dup2(fd[0], STDIN_FILENO);
						execvp(argv2[0], argv2);
					}
					exit(0);
				}
			}
		}
		else wait(NULL);
	}
	
	for(int i = 0; i < cmd_len ;++i)
		delete []argv[i];
	for(int i = 0; i < cmd_len2; ++i)
		delete []argv2[i];
	if(Is_Input && ipdir != NULL) delete []ipdir;
	if(Is_Output && opdir != NULL) delete []opdir;
	delete []argv;
}
int main()
{
	bool should_run = true, run_last = false;
	vector<char> Buffer, Buffer_pre, Buffer_suf, Buffer_last;
	for(;should_run; )
	{
		if(!run_last)
		{
			printf("osh>");
			fflush(stdout);
			char temp_char = skip_space();
			if(temp_char == '\n') continue;
			bool Is_quo = temp_char == '"';
			if(temp_char == '|')
			{
				printf("syntax error near unexpected token `|'\n");
				while(true)
				{
					if(temp_char == '\n' && !Is_quo) break;
					temp_char = getchar();
					Is_quo ^= (temp_char == '"');
				}
				continue;
			}
			while(true)
			{
				if(temp_char == '\n' && !Is_quo) break;
				Buffer.push_back(temp_char);
				temp_char = getchar();
				Is_quo ^= (temp_char == '"');
			}
		}
		else
		{
			run_last = false;
			if(Buffer_last.size() == 0)
			{
				printf("No commands in history.\n");
				continue;
			}
			for(vchit it = Buffer_last.begin(); it != Buffer_last.end(); ++it)
			{
				Buffer.push_back(*it);
				putchar(*it);
			}
			puts("");
		}
		/*
		cout << "Buffer: ";
		for(vchit it = Buffer.begin(); it != Buffer.end(); ++it)
			cout << (*it);
		cout << endl;
		*/
		int lspace(0), rspace(0);
		bool Is_pipe = Divide_command(
			Buffer, Buffer_pre, Buffer_suf,
			lspace, rspace
		);
		/*
		cout << "Buffer_Pre: ";
		for(vchit it = Buffer_pre.begin(); it != Buffer_pre.end(); ++it)
			cout << (*it);
		cout << endl;
		*/
		if(Is_pipe)
			run_pipe(Buffer_pre, Buffer_suf, lspace, rspace, run_last, should_run);
		else
			run_single(Buffer_pre, lspace, should_run, run_last);
		if(!run_last)
		{
			Buffer_last.clear();
			for(vchit it = Buffer.begin(); it != Buffer.end(); ++it)
				Buffer_last.push_back(*it);
		}
		Buffer.clear(), Buffer_pre.clear(), Buffer_suf.clear(); 
	}
	return 0;
}
