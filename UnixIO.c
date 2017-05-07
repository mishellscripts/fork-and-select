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

int main() {

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
		printf("%d\n", i);
		if (pipe(fds[i]) == -1) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
		if ((pids[i] = fork()) < 0) {
			perror("fork");
			abort();
		} else if (pids[i] == 0) {
			//if (pipe(fds[i][0] == -1)) {
			//	fprintf(stderr,"pipe() failed");
			//	return 1;
			//}

			//pipe(fds[i]);

			//Work for child process

			while (time(NULL) - startTime < 5) {
				int sleepDuration = rand() % 3;

				gettimeofday(&tv2, NULL);

				unsigned long microsec = (1000000 * tv2.tv_sec + tv2.tv_usec)
								- (1000000 * tv1.tv_sec + tv1.tv_usec);

				time_t milsec = microsec / 1000;
				time_t sec = milsec / 1000;
				time_t min = sec / 60;
				time_t remainingsec = sec % 60;
				time_t remainingmsec = milsec % 1000;

				char *timestamp = malloc(100);
				sprintf(timestamp, "%d:%.2d.%.3d: Child %d message %d", min,
						remainingsec, remainingmsec, i, messageNum);

				write(fds[i][WRITE_END], timestamp, strlen(timestamp) + 1);
				//printf("Child writes %s\n", timestamp);

				//printf("Child %d sleeps for %d sec\n\n", i, sleepDuration);
				sleep(sleepDuration);
				messageNum++;

				//close(fds[i][WRITE_END]);
				//read(fds[i][READ_END], read_msg, BUFFER_SIZE);
				//printf("Parent: Read '%s' from the pipe.\n", read_msg);
			}
			close(fds[i][READ_END]);
			break;
		}
		//exit(0);
	}

	if (pids[0] > 0) {
		char buffer[128];
		int result, nread;

		fd_set inputs, inputfds; // sets of file descriptors
		struct timeval timeout;

		FD_ZERO(&inputs);    	// initialize inputs to the empty set
		FD_SET(0, &inputs);  	// set file descriptor 0 (stdin)

		//  Wait for input on stdin for a maximum of 2.5 seconds.
		for (;;) {
			inputfds = inputs;

			// 2.5 seconds.
			timeout.tv_sec = 2;
			timeout.tv_usec = 500000;

			// Get select() results.
			result = select(FD_SETSIZE, &inputfds, (fd_set *) 0, (fd_set *) 0,
					&timeout);

			// Check the results.
			//   No input:  the program loops again.
			//   Got input: print what was typed, then terminate.
			//   Error:     terminate.
			switch (result) {
			case 0: {				// timeout w/o input
				printf("@");
				fflush(stdout);
				break;
			}

			case -1: {				// error
				perror("select");
				exit(1);
			}

			// If, during the wait, we have some action on the file descriptor,
			// we read the input on stdin and echo it whenever an
			// <end of line> character is received, until that input is Ctrl-D.
			default: {				// Got input
				if (FD_ISSET(0, &inputfds)) {
					ioctl(0, FIONREAD, &nread);

					if (nread == 0) {
						printf("Keyboard input done.\n");
						exit(0);
					}

					nread = read(0, buffer, nread);
					buffer[nread] = 0;
					printf("Read %d characters from the keyboard: %s", nread,
							buffer);
				}
				break;
			}
			}
		}
	}
	return 0;
}

