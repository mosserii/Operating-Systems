#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// arglist - a list of char* arguments (words) provided by the user
// it contains count+1 items, where the last item (arglist[count]) and *only* the last is NULL
// RETURNS - 1 if should continue, 0 otherwise
int process_arglist(int count, char** arglist);

// prepare and finalize calls for initialization and destruction of anything required
int prepare(void);
int finalize(void);

int main(void)
{
	if (prepare() != 0)
		exit(1);
	
	while (1)
	{
		char** arglist = NULL;
		char* line = NULL;
		size_t size;
		int count = 0;

        /*putting line from stdin into line*/
		if (getline(&line, &size, stdin) == -1) {
			free(line);
			break;
		}

        /*creating an array of words*/
		arglist = (char**) malloc(sizeof(char*));
		if (arglist == NULL) {
			printf("malloc failed: %s\n", strerror(errno));
			exit(1);
		}
		arglist[0] = strtok(line, " \t\n");/*splitting the line into words and putting it in arglist[0]*/
    
		while (arglist[count] != NULL) {
			++count;
			arglist = (char**) realloc(arglist, sizeof(char*) * (count + 1));/*"making" count+1 spaces in arglist*/
			if (arglist == NULL) {
				printf("realloc failed: %s\n", strerror(errno));
				exit(1);
			}
      
			arglist[count] = strtok(NULL, " \t\n");/*NULL at the end of arglist*/
		}
    
		if (count != 0) {
			if (!process_arglist(count, arglist)) {
				free(line);
				free(arglist);
				break;
			}
		}
    
		free(line);
		free(arglist);
	}
	
	if (finalize() != 0)
		exit(1);

	return 0;
}
