#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE 256

inline int Is_Seperation(const char *x)
{
	if(*x == ' ' || *x == '|') return 1;
	if(*x == '<' || *x == '>') return 1;
	return 0;
}
char Skip_Space(void)
{
	char ch;
	while((ch = getchar()) == ' ');
	return ch;
}
int main(void)
{
	char *args[MAX_LINE / 2 + 1], *targs[MAX_LINE / 2 + 1];
	char **current_args = args, **last_args = targs;
	int should_run = 1;
	args[0] = NULL;
	while(should_run)
	{
		printf("osh>");
		fflush(stdout);
		for(int i = 0; current_args[i] != NULL; ++i)
			free(current_args[i]);
		char first_char = Skip_Space();
		int Is_quo = (first_char == '"');
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
		
		
		char **temp_args = current_args;
		current_args = last_args;
		last_args = temp_args;
	}
	return 0;
}