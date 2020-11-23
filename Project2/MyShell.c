#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256

int Is_Operator(const char *x)
{
	if((*x) == '<' || (*x) == '>')return 1;
	if((*x) == '&' || (*x) == '|')return 1;
	return 0;
}
char Skip_Space(void)
{
	char ch;
	while((ch = getchar()) == ' ');
	return ch;
}
int Convert_To_Args(
	int *iswait, char *cmdline, char *args[],
	char *secargs[], char *ipdir, char *opdir
)
{
	char **curr = args;
	int lspace = 0, rspace = 0, lst = 0;
	int *cspace = &lspace, lasti;
	for(int i = 0; cmdline[i] != '\0'; lasti = ++i)
	{
		if(cmdline[i] == '|')
		{
			curr[*cspace] = NULL, ++i;
			curr = secargs, cspace = &rspace;
		}
		else if(cmdline[i] == '<' || cmdline[i] == '>')
		{
			char *udir = cmdline[i] == '<' ? ipdir : opdir;
			if(cmdline[i += 2] == '\0')
			{
				args[lspace] = secargs[rspace] = NULL;
				return 1;
			}
			if(cmdline[i] == '"')
			{
				for(lst = 1; cmdline[i + lst] != '"'; ) ++lst;
				for(int j = 1; j < lst; ++j) udir[j - 1] = cmdline[i + j];
				udir[lst - 1] = '\0', i = i + lst, lst = 0;
			}
			else
			{
				for(lst = 0; cmdline[i + lst] != ' ' && cmdline[i + lst] != '\0'; ) ++lst;
				for(int j = 0; j < lst; ++j) udir[j] = cmdline[i + j];
				udir[lst] = '\0', i = i + lst - 1, lst = 0;
			}
		}
		else if(cmdline[i] == '"')
		{
			for(lst = 1; cmdline[i + lst] != '"'; ) ++lst;
			curr[*cspace] = (char *) malloc(sizeof(char) * (lst + 5));
			for(int j = 1; j < lst; ++j)
				curr[*cspace][j - 1] = cmdline[i + j];
			curr[*cspace][lst - 1] = '\0', *cspace += 1, i = lst + i, lst = 0; 
		}
		else if(cmdline[i] == ' ')
		{
			if(lst > 0)
			{
				curr[*cspace] = (char *) malloc(sizeof(char) * lst + 5);
				for(int j = lst; j >= 1; --j)
					curr[*cspace][lst - j] = cmdline[i - j];
				curr[*cspace][lst] = '\0', *cspace += 1, lst = 0;
			}
		}
		else if(cmdline[i] == '&')
			*iswait = 0, lst = 0;
		else lst += 1;
	}
	if(lst > 0)
	{
		curr[*cspace] = (char *) malloc(sizeof(char) * lst + 5);
		for(int j = lst; j >= 1; --j)
			curr[*cspace][lst - j] = cmdline[lasti - j];
		curr[*cspace][lst] = '\0', *cspace += 1, lst = 0;
	}
	curr[*cspace] = NULL;
	return 0;
}
int main(void)
{
	char *args[MAX_LINE / 2 + 1], *targs[MAX_LINE / 2 + 1];
	char *second_args[MAX_LINE / 2 + 1], *tsecond_args[MAX_LINE / 2 + 1];
	char **current_args = args, **last_args = targs;
	char linput_dir[MAX_LINE + 1] = {'\0'}, loutput_dir[MAX_LINE + 1] = {'\0'};
	char **csecond_args = second_args, **lsecond_args = tsecond_args, lcmd[MAX_LINE + 1] = {'\0'};
	char cmdline[MAX_LINE + 1], input_dir[MAX_LINE + 1] = {'\0'}, output_dir[MAX_LINE + 1] = {'\0'};
	int should_run = 1, last_wait = 1;
	second_args[0] = tsecond_args[0] = targs[0] = args[0] = NULL;
	while(should_run)
	{
		printf("osh>");
		fflush(stdout);
		for(int i = 0; current_args[i] != NULL; ++i)
			free(current_args[i]);
		for(int i = 0; csecond_args[i] != NULL; ++i)
			free(csecond_args[i]);
		current_args[0] = csecond_args[0] = NULL;
		input_dir[0] = output_dir[0] = '\0';
		/*
		Simple syntax error detection
		The useage of quotation marks are supported
		*/
		
		char first_char = Skip_Space(), *curr = cmdline;
		int Is_quo = (first_char == '"'), iswait = 1, toswap = 1;
		if(first_char == '|')
		{
			for(;;)
			{
				if(first_char == '\n' && Is_quo == 0) break;
				Is_quo ^= ((first_char = getchar()) == '"');
			}
			printf("syntax error near unexpected token `|'\n");
		}
		if(first_char == '|' || first_char == '\n') continue;
		
		/*
		Input Part
		*/
		
		for(char ch = first_char; ;)
		{
			if(ch == '\n' && Is_quo == 0) break;
			*(curr++) = ch;
			if(Is_quo == 1)
			{
				while((ch = getchar()) != '"')
					*(curr++) = ch;
				*(curr++) = ch; ch = ' '; Is_quo = 0;
			}
			else if(ch == ' ')
				Is_quo ^= ((ch = Skip_Space()) == '"');
			else
				Is_quo ^= ((ch = getchar()) == '"');
			
			if(Is_Operator(&ch))
			{
				if(curr != cmdline && *(curr - 1) != ' ')
					(*curr++) = ' ';
				*(curr++) = ch; ch = ' ';
			}
		}
		(*curr++) = '\0';
		printf("%s\n", cmdline);
		
		int synerr = Convert_To_Args(
			&iswait, cmdline, current_args, 
			csecond_args, input_dir, output_dir
		);
		
		for(int i = 0; current_args[i] != NULL; ++i)
			printf("ttt: %s\n", args[i]);
		printf("Ip: %s\n", input_dir);
		printf("Op: %s\n", output_dir);
		if(synerr)
			printf("syntax error near unexpected token `newline'\n");
		else
		{
			if(strcmp(current_args[0], "exit") == 0)
				should_run = 0;
			else
			{
				char **uarg = current_args, **uarg2 = csecond_args;
				int exeable = 1;
				if(strcmp(current_args[0], "!!") == 0)
				{
					if(strlen(lcmd) == 0) exeable = 0;
					else printf("%s\n", lcmd);
					iswait = last_wait; toswap = 0; uarg = last_args; uarg2 = lsecond_args; 
					strcpy(input_dir, linput_dir); strcpy(output_dir, loutput_dir);
				}
				if(exeable)
				{
					if(uarg2[0] == NULL)
					{
						int fin = -2, fout = -2;
						if(strlen(input_dir) != 0) fin = open(input_dir, O_RDONLY);
						if(strlen(output_dir) != 0) fout = open(output_dir, O_WRONLY | O_CREAT | O_TRUNC, 0666);
						if(fin == -1 || fout == -1)
							printf("Fail to open file!");
						else
						{
							pid_t pid = fork();
							if(pid < 0)
							{
								printf("Fail to Create subprocess!");
								exit(-1);
							}
							if(pid == 0)
							{
								if(fin != -2) dup2(fin, STDIN_FILENO);
								if(fout != -2) dup2(fout, STDOUT_FILENO);
								execvp(uarg[0], uarg);
								exit(0);
							}
							else if(iswait) waitpid(pid, NULL, 0);
						}
						if(fin != -1 && fin != -2) close(fin);
						if(fout != -1 && fout != -2) close(fout);
					}
					else
					{
						pid_t pid = fork();
						if(pid < 0)
						{
							printf("Fail to Create subprocess!");
							exit(-1);
						}
						else if(pid == 0)
						{
							int fd[2] = {0};
							if(pipe(fd) < 0)
								printf("Pipe Error!\n");
							else
							{
								pid_t pid2 = fork();
								if(pid2 == 0)
								{
									dup2(fd[1], STDOUT_FILENO);
									execvp(uarg[0], uarg);
									exit(0);
								}
								if(pid2 > 0)
								{
									wait(NULL); close(fd[1]);
									fcntl(fd[0], F_SETFL, O_NONBLOCK);
									if(strcmp(uarg2[0], "exit") == 0)
										should_run = 0;
									else
									{
										dup2(fd[0], STDIN_FILENO);
										execvp(uarg2[0], uarg2);
										exit(0);
									}
								}
							}
						}
						else waitpid(pid, NULL, 0);
					}
				}
				else printf("No commands in history.\n");
			}
		}
		// Swap current and last
		if(toswap)
		{
			char **temp_args; strcpy(lcmd, cmdline);
			temp_args = current_args; current_args = last_args; last_args = temp_args;
			temp_args = csecond_args; csecond_args = lsecond_args; lsecond_args = temp_args;
			strcpy(linput_dir, input_dir); strcpy(loutput_dir, output_dir); last_wait = iswait;
		}
	}
	return 0;
}