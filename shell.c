#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define HISTORY_SIZE 20

// Prototypes
int execute(char* line); // Execute the command in the string
void printHistory(char* hist[], int start); // Show [HISTORY_SIZE] latest command
void callHistory(char* str, char* hist[], int hc); // Execute command in the history

int main() {
	char* hist[HISTORY_SIZE]; // Circular history array
	int hc = 0; // History counter
	char c; // Dummy character for dumping stdin
	char* line = malloc(0); // String of the whole command including arguments
	int run = 1;
	// Main loop
	while (run) {
		line = realloc(line, 100*sizeof(char)); // Reallocating new memory for the string
		printf("\n# ");
		scanf("%[^\n]%*c", line);// Receiving the whole line, dumping \n
		// Exiting the shell
		if (strcmp(line,"exit") == 0) {
			// Freeing memory before exit
			int i = 0;
			while (i < hc && i < HISTORY_SIZE) {
				free(hist[i++]);
			}
			free(line);
			run = 0;
		}
		// Calling command from history
		else if (line[0] == '!') callHistory((line+1),hist,hc);
		// Executing command
		else {
			// Allocating memory for history
			if (hc < HISTORY_SIZE) hist[hc%HISTORY_SIZE] = malloc(sizeof(char) * (strlen(line)+1));
			// Remove the oldest history and make space for a new one
			else hist[hc%HISTORY_SIZE] = realloc(hist[hc%HISTORY_SIZE], sizeof(char) * (strlen(line)+1));
			// Adding the command into the history array
			strcpy(hist[hc%HISTORY_SIZE],line);
			hc++;
			// Calling history
			if (strcmp(line,"history") == 0) printHistory(hist,hc);
			// Exit if something's wrong
			else if (execute(line) != 0) {
				fprintf(stderr, "Something freaky happened!\n");
				return 1;
			}
		}
	}
	return 0;
}

int execute(char* line) {
	char* token[10]; // Token array for the arguments
	int i = 0;
	// Exploding line into array by space
	token[i] = strtok(line," ");
	while (token[i] != NULL) {
		token[++i] = strtok(NULL," ");
	}
	// Forking the process
	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Fork Failed\n");
		return 1;
	}
	// Executing the command
	else if (pid == 0) {
		// Overwriting child process with command
		execvp(token[0],token);
		// Show error message
		perror("");
		// End process
		exit(0);
	}
	else {
		// Wait for the child process to finish and back to the main loop
		wait(NULL);
		return 0;
	}
}

void printHistory(char* hist[], int start) {
	int i = 0;
	if (start < HISTORY_SIZE) {
		 for (i = 0; i < start; i++) {
			 printf("%d. %s\n", i+1, hist[i]);
		 }
	}
	else {
		 for (i = 0; i < HISTORY_SIZE; i++) {
			 printf("%d. %s\n", i+1, hist[start++ % HISTORY_SIZE]);
		 }
	}
}

void callHistory(char* str, char* hist[], int hc) {
	if (strcmp(str,"!") == 0) {
		if (hc != 0) {
			if (strcmp(hist[hc%HISTORY_SIZE-1],"history") == 0) printHistory(hist,hc);
			else {
				// Copy the last command from the history to a new string and execute
				char* command = malloc(sizeof(hist[hc%HISTORY_SIZE-1]));
				strcpy(command, hist[hc%HISTORY_SIZE-1]);
				printf("%s\n",command);
				execute(command);
				free(command);
			}
		}
		else fprintf(stderr, "There's no command in the history!\n");
	}
	else {
		// Change the number from string to integer
		int num = atoi(str);
		if (num > 0 && num <= HISTORY_SIZE && num <= hc) {
			if (strcmp(hist[num-1],"history") == 0) printHistory(hist,hc);
			// Copy the nth command from the history to a new string and execute
			else {
				char* command;
				if (hc < HISTORY_SIZE) {
					command = malloc(sizeof(hist[num-1]));
					strcpy(command, hist[num-1]);
				}
				else {
					command = malloc(sizeof(hist[(hc%HISTORY_SIZE+num-1)%HISTORY_SIZE]));
					strcpy(command, hist[(hc%HISTORY_SIZE+num-1)%HISTORY_SIZE]);
				}
				execute(command);
				free(command);
			}
		}
		else fprintf(stderr, "Invalid!\n");
	}
}