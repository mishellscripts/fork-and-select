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

	gettimeofday(&tv1, NULL);
	time_t startTime = time(NULL);

	char write_msg[BUFFER_SIZE] = "You're my child process!";
	char read_msg[BUFFER_SIZE];

	srand(NULL);
	int messageNum = 1;

	pid_t pids[5];
	int fds[5][2];
	int i;
	int n = 5;

	/* Start children. */
	for (i = 0; i < n; ++i) {
		if ((pids[i] = fork()) < 0) {
			perror("fork");
			abort();
		}
		else if (pids[i] == 0) {
			//if (pipe(fds[i][0] == -1)) {
			//	fprintf(stderr,"pipe() failed");
			//	return 1;
			//}
			pipe(fds[i]);

			//Work for child process
			while(time(NULL) - startTime < 5) {
				int sleepDuration = rand() % 3;

				gettimeofday(&tv2, NULL);

				unsigned long microsec = (1000000 * tv2.tv_sec + tv2.tv_usec)
																										- (1000000 * tv1.tv_sec + tv1.tv_usec);

				time_t milsec = microsec/1000;
				time_t sec = milsec/1000;
				time_t min = sec/60;
				time_t remainingsec = sec % 60;
				time_t remainingmsec = milsec % 1000;

				char *timestamp = malloc(100);
				sprintf(timestamp, "%d:%.2d.%.3d: Child %d message %d", min,
						remainingsec, remainingmsec, i, messageNum);

				write(fds[i][WRITE_END], timestamp, strlen(timestamp)+1);
				printf("Child %d writes %s\n", i, timestamp);

				printf("Child %d sleeps for %d sec\n\n", i, sleepDuration);
				sleep(sleepDuration);
				messageNum++;
			}
		}
		//exit(0);
	}

	struct timeval timeout;
	int rc;

	// 2.5 seconds time limit.
	timeout.tv_sec = 2;
	timeout.tv_usec = 500000;

	fd_set read_set;

	FD_ZERO(&read_set);
	for (i = 0; i < n; ++i) {
		FD_SET(fds[i][READ_END], &read_set);
	}

	rc = select(sizeof(read_set)*8, &read_set,
			NULL, NULL, &timeout);

	if (rc > 0) {
		//close(fds[i][WRITE_END]);
		//			    	read(fds[i][READ_END], read_msg, BUFFER_SIZE);
		read(&read_set, read_msg, BUFFER_SIZE);
		printf("Parent: Read '%s' from the pipe.\n", read_msg);
		close(READ_END);
	}
	else {
		puts("None read");
	}

	return 0;

}

