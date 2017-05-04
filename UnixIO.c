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

	//time_t curTime;
	//gettimeofday(&tv, NULL);
	//printf(tv.tv_sec);
	//curTime = tv.tv_sec;
	//printf("%d\n", curTime);

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
		if(pipe(fds[i]) == -1){
			perror("pipe");
			exit(EXIT_FAILURE);
		}
		if ((pids[i] = fork()) < 0) {
			perror("fork");
			abort();
		}
		else if (pids[i] == 0) {
			//if (pipe(fds[i][0] == -1)) {
			//	fprintf(stderr,"pipe() failed");
			//	return 1;
			//}
			//pipe(fds[i]);

			//Work for child process
			printf("%d\n", i);

			//for(int x = 0; x < 5; x++) {
			while(time(NULL) - startTime < 5) {
				int sleepDuration = rand() % 3;
				//alarm(2);

				//time_t currentTime = time(NULL);
				//time_t elapsedTime = currentTime - startTime;
				//printf("%d\n", elapsedTime);

				gettimeofday(&tv2, NULL);

				unsigned long microsec = (1000000 * tv2.tv_sec + tv2.tv_usec)
																		- (1000000 * tv1.tv_sec + tv1.tv_usec);

				time_t milsec = microsec/1000;
				time_t sec = milsec/1000;
				time_t min = sec/60;
				time_t remainingsec = sec % 60;
				time_t remainingmsec = milsec % 1000;

				char *timestamp = malloc(100);
				sprintf(timestamp, "%d:%.2d.%.3d: Child %d message %d\n", min,
						remainingsec, remainingmsec, i, messageNum);
				//min + ":" + remainingsec + "." + remainingmsec + "\n";

				write(fds[i][WRITE_END], timestamp, strlen(timestamp)+1);
				//printf("%d:%.2d.%.3d\n", min, remainingsec, remainingmsec);
				puts(timestamp);

				// Close the unused WRITE end of the pipe.
				close(fds[i][WRITE_END]);

				// Read from the READ end of the pipe.
				//read(fd[READ_END], read_msg, BUFFER_SIZE);
				//printf("Child %d: Read '%s' from the pipe.\n", i, read_msg);
				// Close the READ end of the pipe.
				close(fds[i][READ_END]);

				printf("Child %d sleeps for %d sec\n\n", i, sleepDuration);
				sleep(sleepDuration);
				messageNum++;
				break;
			}
		}
		else{
			continue;
		}	//}

	}

		//exit(0);
	//}

	//puts("done");

	/* Wait for children to exit. */
	int status;
	pid_t pid = 1;





	while (n > 0) {
		pid = wait(&status);
		//printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);

		printf("done\n");

		--n;  // TODO(pts): Remove pid from the pids array.
	}

	return 0;
}


