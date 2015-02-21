#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    //for fork
#include <sys/types.h> //for waitpid
#include <sys/wait.h>  //for waitpid
#include <signal.h>    //for kill
#include <time.h> 	   //for logging system time
#include <math.h> 	   //for the calculator

/**
 * linked list struct for logging command history in each slot
 * We typedef it so it is easier to declare and use it
 */
typedef struct _linked_list {
	char * data;				//the command that was run
	char * timeRun; 			//the time at which it was run
	char * output;				//the output to stdout of the process
	int userTime;   			//the amount of CPU time the user took
	int sysTime;    			//the amount of CPU time the system took
	int status;     			//the current status - running = -1, etc
	struct _linked_list * next; //the next link
} linked_list;

//history of commands
linked_list * history[4];

//queue for 'w' command
linked_list * queue;

/**
 * Create 4 FILEs that will serve as logs of command output
 * Can see contents by running 'l <N>' in shpar
 */
FILE * logFile[4];

//Most recent time stats
struct rusage usage;

//the array of pid_t's that represents the child processes
pid_t processes[4];

//a function to save and reopen a file
void reopen(int i) {
	if (i == 0)
		logFile[0] = fopen("logs/slot_1.log", "a");
	else if (i == 1)
		logFile[1] = fopen("logs/slot_2.log", "a");
	else if (i == 2)
		logFile[2] = fopen("logs/slot_3.log", "a");
	else
		logFile[3] = fopen("logs/slot_4.log", "a");
}

void handle(int signal) {
	pid_t currProcess;

	int status;

	//Use WUNTRACED so it stops on stopped processes and exited ones
	if ((currProcess = wait3(&status, WUNTRACED, &usage)) < 0) {
		puts("WAIT3 ERROR");
	}

	//check for process id
	for (int i = 0; i < 4; i++) {
		if (currProcess == processes[i]) {
			
			//check if first slot in history is empty
			linked_list * curr = history[i];
			
			//traverse the linked list
			while (curr->next != NULL) 
				curr = curr->next;

			//store usage times
			curr->userTime = usage.ru_utime.tv_usec;
			curr->sysTime = usage.ru_stime.tv_usec;

			//store status
			curr->status = status;

			//print process information to correct logfile
			fprintf(logFile[i], "trash~$ %s\n", curr->data);
			fprintf(logFile[i], "\tRun:    %s", curr->timeRun);
			fprintf(logFile[i], "\tUser:   %dms\n\tSystem: %dms\n", curr->userTime, curr->sysTime);
			fprintf(logFile[i], "\tOutput: ");
			
			FILE * temp = fopen("temp.out", "r");
			char * tempLine = (char *) malloc(45);
			size_t tempLineSize;
			
			//get the first line
			if (getline(&tempLine, &tempLineSize, temp) > -1) {
				fprintf(logFile[i], "%s", tempLine);

				curr->output = (char *) malloc(strlen(tempLine));
				strcat(curr->output, tempLine);
			}
			else {
				fprintf(logFile[i], "\n");

				curr->output = (char *) malloc(19);
				strcat(curr->output, "No output to show!\n");
			}

			//get the rest of the output (if it exists)
			while (getline(&tempLine, &tempLineSize, temp) > -1) {
				fprintf(logFile[i], "\t\t%s", tempLine);

				curr->output = (char *) realloc(curr->output, strlen(curr->output) + strlen(tempLine));
				strcat(curr->output, tempLine);
			}

			//print status
			if (WIFSIGNALED(curr->status)) {
				fprintf(logFile[i], "\tStatus: Exited due to signal %d", WTERMSIG(curr->status));

				//segfault
				if (WTERMSIG(curr->status) == SIGSEGV)
					fprintf(logFile[i], ": Segmentation Fault\n\n");
				else
					fprintf(logFile[i], "\n\n");
			}
			else if (WIFEXITED(curr->status)) 
				fprintf(logFile[i], "\tStatus: Exited with code %d\n\n", curr->status);
			else if (WIFSTOPPED(curr->status))
				fprintf(logFile[i], "\tStatus: Stopped\n\n");
			
			fclose(logFile[i]);
			reopen(i); 
		}
	}
}

//get the current system time
char * getTime() {
	time_t rawTime;
	struct tm * timeInfo;
	time(&rawTime);
	timeInfo = localtime(&rawTime);

	return asctime(timeInfo);
}

//remove the head of linked_list 'queue'
void remove_Head() {
	linked_list * temp = queue;

	queue = queue->next;

	free(temp);
}

int main() {
	/**
	 * Initialize the linked_list structs that represent the history
	 * of commands for each respective process slot
	 */
	for (int i = 0; i < 4; i++) {
		history[i] = (linked_list *) malloc(sizeof(linked_list));
		history[i]->data = "";
		history[i]->next = NULL;
		history[i]->userTime = -1;
		history[i]->sysTime = -1;
		history[i]->output = (char *) malloc(0);
	}

	//open up the FILE streams
	logFile[0] = fopen("logs/slot_1.log", "a");
	logFile[1] = fopen("logs/slot_2.log", "a");
	logFile[2] = fopen("logs/slot_3.log", "a");
	logFile[3] = fopen("logs/slot_4.log", "a");

	//initialize the process queue
	queue = NULL;

	/**
	 * No arguments required because we are just starting up the 
	 * shell. So just start the while loop where we will
	 * parse the commands and perform actions.
	 */
	int running = 1;
	char * command = NULL;
	size_t commandSize = 0;

	while (running) {
		printf("trash-0.2a~$ ");
		//the current command
		getline(&command, &commandSize, stdin);

		//remove the newline character at end of string if it exists
		if (command[strlen(command) - 1] == '\n')
			command[strlen(command) - 1] = 0;

		/** 
		 * Here we parse the command to see what the user wants to do
		 * For a list of available commands, see the 'h' (Help) command
		 * below
		 */
		//QUIT COMMAND
		if (!strcmp(command, "q") || !strcmp(command, "Q")) {
			running = 0;

			//close all files
			for (int i = 0; i < 4; i++)
				fclose(logFile[i]);

			//fork a process to remove 'temp.out'
			int status;
			pid_t child = fork();

			if (child == -1)
				return 1;
			else if (child > 0) {
				pid_t parent = waitpid(child, &status, 0);

				return 0;
			}
			else {
				execlp("bash", "bash", "-c", "rm temp.out", (char *) NULL);
				puts("You shouldn't reach here");
			}
		}
		else if (!strcmp(command, "ls") || !strcmp(command, "LS")) {
			//we will fork to run 'ls'
			int ls_status;
			pid_t ls_child = fork();

			//check which process we are in
			if (ls_child < 0) {
				puts("Forking error!");
				return 1;
			}
			else if (ls_child > 0) { //parent
				//wait for the child to exec 'ls'
				pid_t ls_parent = waitpid(ls_child, &ls_status, 0);
			}
			else  {				     //child
				//run exec to 'ls'
				execlp("bash", "bash", "-c", "ls", (char *) NULL);
				//shouldn't reach here
				puts("EXEC ERROR - ls");
			}
		}
		//RUN PROGRAM COMMAND
		else if ((command[0] == 'r' || command[0] == 'w') && strcmp(command, "rpg")) {
			/**
			 * We will parse the command
			 * The correct format is:
			 * r <LINENUM> <COMMAND>
			 */
			int slot;
			int parse;
			char * input = (char *) malloc(strlen(command) - 4);

			if (command[0] == 'r')
				parse = sscanf(command, "r %d %[^\n]", &slot, input);
			else if (command[0] == 'w')
				parse = sscanf(command, "w %d %[^\n]", &slot, input);

			//if the formatting is incorrect, throw an error, but don't quit
			if (parse == 0)
				puts("ERROR");
			else if (slot < 1 || slot > 4) {
				printf("ERROR: Invalid slot number\n");
			}
			else {
				/**
				 * Here we will store the command into its appropriate slot in 'history'
				 * If this is the first time we are storing data, just store it directly
				 * If not, we will traverse the linked list and then store it at the end.
				 */
				if (!strcmp(history[slot-1]->data, "")) { //empty
					history[slot-1]->data = input;
					history[slot-1]->status = -1;
					history[slot-1]->next = NULL;
					history[slot-1]->timeRun = getTime();
				}
				else {								   //not empty
					//create a temporary link
					linked_list * curr = history[slot-1];

					//traverse to the end
					while (curr->next != NULL)
						curr = curr->next;

					/**
					 * first we check if there is already a process
					 * in this slot. If so, we kill it
					 */
					if (curr->status == -1 || WIFSTOPPED(curr->status)) {
						if (command[0] == 'r') {
							//print process information to correct logfile
							fprintf(logFile[slot-1], "trash~$ %s\n", curr->data);
							fprintf(logFile[slot-1], "\tRun:    %s", curr->timeRun);
							fprintf(logFile[slot-1], "\tUser:   %dms\n\tSystem: %dms\n", curr->userTime, curr->sysTime);
							fprintf(logFile[slot-1], "\tStatus: KILLED\n\n");
							fclose(logFile[slot-1]);
							reopen(slot-1);

							printf("Killing running process in slot %d\n", slot);
							kill(processes[slot-1], SIGKILL);

						}
						else {
							//printf("Waiting for running process in slot %d\n", slot);
				
							if (queue != NULL) {
								
							}
							if (curr->status == -1) {
								int wait_status;
								pid_t wait;
								wait = waitpid(processes[slot-1], &wait_status, 0);
							}
						}
						
					}
					//Create the new link and then place it in the list
					linked_list * link = (linked_list *) malloc(sizeof(linked_list));
					link->data = input;
					link->next = NULL;
					link->status = -1;
					link->timeRun = getTime();
					curr->next = link;
				}
				
				//declare the signal handler for the child process
				signal(SIGCHLD, handle);

				processes[slot-1] = fork();
				if (processes[slot-1] == -1) {
					return 1;
				}
				else if (processes[slot-1] == 0) { //child
					strcat(input, " > temp.out");
					execlp("bash", "bash", "-c", input, (char *) NULL);
					printf("EXEC ERROR\n");
				}
				else { //parent
					/* do nothing! */
				}
			}
		}
		else if (command[0] == 'c' && command[1] == 'd') {
			//parse which directory we want to go to
			char * directory = (char *) malloc(strlen(command) - 3);
			int parse = sscanf(command, "cd %[^\n]", directory);

			//check formatting
			if (parse == 0)
				puts("ERROR PARSING COMMAND");
			else {
				//use chdir to change directory
				int change = chdir(directory);

				//check if directory exists, if not throw error
				if (change == -1)
					printf("-trash: cd \'%s\' - No such file or directory\n", directory);
			}
		}
		else if (!strcmp(command, "h") || !strcmp(command, "H")) {
			puts("==================================");
			puts("=====trash, version 0.2 alpha=====");
			puts("==================================");
			puts("\nHere is a list of available commands:");
			puts("  i             get overview of all process slots");
			puts("  o <N>         get output of most recent process in slot <N>");
			puts("  l <N>         display log of commands in slot <N>");
			puts("  r <N> <...>   run <...> in slot <N> - will kill process if slot is occupied");
			puts("  w <N> <...>   run <...> in slot <N> - will wait for process if slot is occupied");
			puts("  cd <...>      change to <...> directory");
			puts("  h             display this message");
			puts("  ls            view current directory's contents");
			puts("  z <N>         pause the process in slot <N> if it is running");
			puts("  g <N>         resume the process in slot <N> if it is paused");
			puts("  <N> <OP> <N>  perform basic calculator operations like /, *, +, -");
			puts("  rpg           TO BE IMPLEMENTED -- will be a fun command ;)");
			puts("  q             quit trash\n");
		}
		//LOG COMMAND
		else if (command[0] == 'l' || command[0] == 'L') {
			//parse the command to see which slot we want to view
			int slot;
			int parse = sscanf(command, "l %d\n", &slot);

			//check for correct formatting
			if (parse == 0)
				puts("ERROR PARSING COMMAND");
			else if (slot < 1 || slot > 4) 
				printf("ERROR: Invalid slot number\n");
			else {
				pid_t child = fork();
				if (child == -1) {
					return 1;
				}
				else if (child == 0) { //child
					//cat the log file to the shell
					char * temp = (char *) malloc(15);
					strcat(temp, "cat logs/slot_");

					if (slot == 1)
						strcat(temp, "1.log");
					else if (slot == 2)
						strcat(temp, "2.log");
					else if (slot == 3)
						strcat(temp, "3.log");
					else
						strcat(temp, "4.log");

					execlp("bash", "bash", "-c", temp, (char *) NULL);
					printf("EXEC ERROR\n");
				}
				else { //parent
					int log_status;
					pid_t log_child = waitpid(child, &log_status, 0);
				}
			}
		}
		//INFO COMMAND
		else if (!strcmp(command, "i")) {
			//info header
			printf("\nSLOT\tCOMMAND \tSTATUS\n");

			//print a formatted output message
			for (int i = 0; i < 4; i++) {
				//line number
				printf("   %d\t", i+1);

				//traverse to end of linked list
				linked_list * curr = history[i];
				while (curr->next != NULL)
					curr = curr->next;

				//temporary string so we can truncate if its too long
				char * temp = (char *) malloc(14);
				strcpy(temp, curr->data);
				if (strcmp(curr->data, "")) {
					if (strlen(temp) > 14) {
						temp[12] = 0;
						strcat(temp, "...");
						printf("%s ", temp);
					}
					else if (strlen(temp) < 8) {
						printf("%s\t\t", temp);
					}
					else {
						printf("%s\t", temp);
					}
				}

				//print usage time data
				if (!strcmp(curr->data, "")) {
					printf("n/a\t\tn/a\n");
					printf("\tUser: n/a\tSystem: n/a\n");
				}
				else if (curr->status == -1) {
					printf("running\n");
					printf("\tUser: %dms\tSystem: %dms\n", 1, 1);
				}
				else if (WIFSIGNALED(curr->status)) {
					printf("ended with signal: %d\n", WTERMSIG(curr->status));
					printf("\tUser: %dms\tSystem: %dms\n", curr->userTime, curr->sysTime);
				}
				else if (WIFEXITED(curr->status)) {
					printf("exited normally\n");
					printf("\tUser: %dms\tSystem: %dms\n", curr->userTime, curr->sysTime);
				}
				else if (WIFSTOPPED(curr->status)) {
					printf("stopped\n");
					printf("\tUser: %dms\tSystem: %dms\n", curr->userTime, curr->sysTime);
				}
			}

			//print newline to make it cleaner
			printf("\n");
		}
		//PAUSE/CONTINUE COMMANDS
		else if (command[0] == 'z' || command[0] == 'Z' || command[0] == 'g' || command[0] == 'G') {
			//parse the command for the desired slot number
			int slot;
			int parse;

			if (command[0] == 'z' || command[0] == 'Z')
				parse = sscanf(command, "z %d\n", &slot);
			else
				parse = sscanf(command, "g %d\n", &slot);

			//check if it was formatted correctly
			if (parse == 0) 
				puts("Command not recognized");
			else if (slot < 0 || slot > 4) 
				puts("Invalid slot number. Choose 1 - 4");
			else {
				//create a temporary link for the current process slot
				linked_list * curr = history[slot-1];

				//traverse to the end to get the most recent process
				while (curr->next != NULL)
					curr = curr->next;

				/**
				 * Based on if we're calling 'w' or 'g' we perform actions
				 * If 'z' we check if there is a process running
				 *     if so, we pause it
				 * If 'g' we check if there is a paused process
				 * If so, we continue it, otherwise nothing
				 */
				if (command[0] == 'z') {
					//if the current process is still running
					if (curr->status == -1) {
						//we set it to -2, for "paused"
						curr->status = -2;
						
						//then we pause it
						kill(processes[slot-1], SIGSTOP);
					}
					else {
						//Otherwise, just throw an error
						printf("No running process in slot %d!\n", slot);
					}
				}
				else { //Otherwise if 'g' was called
					//we check if the current process has been paused
					if (WIFSTOPPED(curr->status)) {
						//if so, we set it back to -1, for running
						curr->status = -1;

						//and then send a SIGCONT signal to start it back up
						kill(processes[slot - 1], SIGCONT);
					}
					else {
						//Otherwise, print an error and do nothign!
						printf("No stopped process in slot %d!\n", slot);
					}
				}
			}
		}
		//O COMMAND
		else if (command[0] == 'o' || command[0] == 'O') {
			//parse the command for the slot number
			int slot;
			int parse = sscanf(command, "o %d\n", &slot);

			if (parse == 0)
				puts("Command not recognized");
			else if (slot < 0 || slot > 4) 
				puts("Invalid slot number, choose 1-4");
			else {
				//create a temporary linked list pointer
				linked_list * curr = history[slot - 1];

				//traverse to the end for the most current process
				while (curr->next != NULL)
					curr = curr->next;

				printf("Displaying output of most recent process in slot %d...\n\n", slot);

				printf("%s\n", curr->output);
			}
		}
		//CALCULATOR
		else if (command[0] == '0' || command[0] == '1' || command[0] == '2' ||
				 command[0] == '3' || command[0] == '4' || command[0] == '5' ||
				 command[0] == '6' || command[0] == '7' || command[0] == '8' ||
				 command[0] == '9') {

			//parse for operations
			int parse;
			char * operation = (char *) malloc(1);
			double operand1, operand2;

			parse = sscanf(command, "%lf %s %lf\n", &operand1, operation, &operand2);

			if (parse != 3)
				puts("Error: Invalid expression");
			else {
				//division
				if (!strcmp(operation, "/"))
					printf("    %lf\n", operand1 / operand2);
				//multiplication
				else if (!strcmp(operation, "*"))
					printf("    %lf\n", operand1 * operand2);
				//addition
				else if (!strcmp(operation, "+"))
					printf("    %lf\n", operand1 + operand2);
				//subtraction
				else if (!strcmp(operation, "-"))
					printf("    %lf\n", operand1 - operand2);
				//modulo
				else if (!strcmp(operation, "%")) {
					int n = operand1 / operand2;
					for (int i = 0; i < n; i++) 
						operand1 -= operand2;

					printf("    %lf\n", operand1);
				}
				//exponents
				else if (!strcmp(operation, "**") || !strcmp(operation, "^"))
					printf("    %lf\n", pow(operand1, operand2));
				else 
					puts("Error: Invalid operator");
			}
		}
		//RPG COMMAND
		else if (!strcmp(command, "rpg")) {
			printf("Starting RPG!\n");

			int game_running = 1;

			//game input
			char * action = NULL;
			size_t action_size = 0;

			while (game_running) {
				//get next command
				//the current command
				getline(&action, &action_size, stdin);

				//remove the newline character at end of string if it exists
				if (action[strlen(action) - 1] == '\n')
					action[strlen(action) - 1] = 0;

				if (!strcmp(action, "q")) {
					game_running = 0;
				}
			}
		}
	}

	return 0;
}










