#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
	
	puts("Cleaning up logfiles...");

	for (int i = 1; i <= 4; i++) {
		//clean up the log files
		char * filename = (char *) malloc(100);

		pid_t child = fork();

		if (child == -1)
			return 1;
		else if (child > 0) {

		}
		else  { //child
			if (i == 1)
				strcat(filename, "echo \"\n=======================\nPROCESS SLOT 1 LOGFILE:\n=======================\" > logs/slot_1.log");
			else if (i == 2)
				strcat(filename, "echo \"\n=======================\nPROCESS SLOT 2 LOGFILE:\n=======================\" > logs/slot_2.log");
			else if (i == 3)
				strcat(filename, "echo \"\n=======================\nPROCESS SLOT 3 LOGFILE:\n=======================\" > logs/slot_3.log");
			else
				strcat(filename, "echo \"\n=======================\nPROCESS SLOT 4 LOGFILE:\n=======================\" > logs/slot_4.log");

			execlp("bash", "bash", "-c", filename, (char *) NULL);
			printf("Shouldnt reach here\n");
		}

	}

	puts("Finished tidying up!");

	return 0;
}