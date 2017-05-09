#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>

#define BUFFER_SIZE 32
#define READ_END     0
#define WRITE_END    1

struct timeval tv1;
struct timeval tv2;

void killProcess(int pid, int sig) {
	kill(pid, SIGKILL);
}

int main()
{

	// Create parent with 5 children process (fork?) **
	// Ea child lifeline of 30 secs **
	// has a pipe **
	// Child 1-4: Sleeps for random (0-2) seconds **
	//	and print message of current time when they wake up **
	//	using (gettimeofday(&tv, NULL)), sent back to parent thru pipe
	// Child 5: Prompts in terminal, write string to parent thru pipe
	// Parent uses select() to choose which message to write to output.txt

	FILE *fPtr;
	fPtr = fopen("output.txt", "w");

	gettimeofday(&tv1, NULL);
	time_t startTime = time(NULL);

	char write_msg[BUFFER_SIZE] = "You're my child process!";
	char read_msg[BUFFER_SIZE];

	fd_set read_set;

	struct timeval timeout;
	int rc;

	// 2.5 seconds time limit.
	timeout.tv_sec = 2;
	timeout.tv_usec = 500000;

	srand(NULL);
	int messageNum = 1;

	pid_t pids[5];
	int fds[5][2];
	int i;
	int n = 5;
	int sleepDuration = rand() % 3;

	/* Start children. */
	for (i = 0; i < n; ++i) {


		char message[300];
		if (i == 4) {
			while(time(NULL) - startTime < 5) {
				printf("Enter message: \n");
				scanf("%s", &message);
				puts("");
			}
		}


		sleepDuration = rand() % 3;
		if (pipe(fds[i]) == -1) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
		if ((pids[i] = fork()) < 0) {
			perror("fork");
			abort();
		}
		else if (pids[i] == 0) {
			//Work for child process
			while(time(NULL) - startTime < 5) {

				FD_ZERO(&read_set);
				FD_SET(fds[i][READ_END], &read_set);
				sleepDuration = rand() % 3;
				gettimeofday(&tv2, NULL);

				unsigned long microsec = (1000000 * tv2.tv_sec + tv2.tv_usec)-(1000000 * tv1.tv_sec + tv1.tv_usec);

				time_t milsec = microsec/1000;
				time_t sec = milsec/1000;
				time_t min = sec/60;
				time_t remainingsec = sec % 60;
				time_t remainingmsec = milsec % 1000;

				char *timestamp = malloc(500);



				if(i == 4) {

					/*char message[300];
					int nread;
					if(FD_ISSET(fds[i][READ_END], &read_set)) {
						//puts("duitty");
						//printf(">>>>>");

						//scanf("%c\n", &message);
						ioctl(fds[i][READ_END], FIONREAD, &nread);
						puts("");
						if (nread == 0) {
							//puts("sad");
							//exit(0);
						}
						else {
						nread = read(fds[i][READ_END], message, nread);
						message[nread] = 0;
						printf("Read %d characters from the keyboard: %s\n", nread, message);
						}
					}
					printf("Enter message: \n");

					scanf("%s", &message);
					puts("");*/
					//fprintf(stdout, "[child] Enter message: ");
					//fgets(message, 80, stdin);
					//printf("[child] line = %s\n", message);
					sprintf(timestamp, "5th >>>> %d:%.2d.%.3d: Child %d message %s", min,
							remainingsec, remainingmsec, i+1, message);
				}
				else { //for remaining 4 children
					sprintf(timestamp, "%d:%.2d.%.3d: Child %d message %d", min,
							remainingsec, remainingmsec, i+1, messageNum);
					messageNum++;
				}



				write(fds[i][WRITE_END], timestamp, strlen(timestamp)+1);
				//printf("Child %d writes %s\n", i, timestamp);
				fprintf(fPtr, "Child %d writes %s\n", i+1, timestamp);

				if(i != 4) {
					//printf("Child %d sleeps for %d sec\n\n", i, sleepDuration);
					fprintf(fPtr, "Child %d sleeps for %d sec\n\n", i+1, sleepDuration);
					sleep(sleepDuration);
				}

				//select
				//FD_SET(fds[i][READ_END], &read_set);

				rc = select(FD_SETSIZE, &read_set, NULL, NULL, &timeout);
				//if (FD_ISSET(fds[i][READ_END], &read_set)) {
				if (rc > 0) {
					//close(fds[i][WRITE_END]);
					read(fds[i][READ_END], read_msg, BUFFER_SIZE);
					//read(&read_set, read_msg, BUFFER_SIZE);
					//printf("Parent: Read '%s' from the pipe.\n", read_msg);
					fprintf(fPtr, "Parent: Read '%s' from the pipe.\n", read_msg);
					//close(READ_END);
				}
				else {
					//puts("None read");
				}



			}
			break;
			//exit(0);
			// FD_ZERO(&fds);    	// initialize inputs
			//FD_SET(0, &inputs);  	// set file descriptor 0 (stdin)
		}
	}
	//puts("done");

	fclose(fPtr);
	return 0;
}

