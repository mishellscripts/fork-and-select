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
#define READ_END 0
#define WRITE_END 1

struct timeval tv1;
struct timeval tv2;


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

	srand(time(NULL));
	int messageNum = 1;

	pid_t pids[5];
	int fds[5][2];
	int i;
	int n = 5;
	int sleepDuration = rand() % 3;

	/* Start children. */
	for (i = 0; i < n; ++i)
	{

		sleepDuration = rand() % 3;
		if (pipe(fds[i]) == -1)
		{
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		char message[128];
		if (i == 4)
		{
			int msg_size = 0;
			while (time(NULL) - startTime < 30)
			{

				FD_ZERO(&read_set);
				FD_SET(fds[i][READ_END], &read_set);

				printf("\nEnter message: \n");
				scanf("%s", &message);
				puts("");

				gettimeofday(&tv2, NULL);

				unsigned long microsec = (1000000 * tv2.tv_sec + tv2.tv_usec) - (1000000 * tv1.tv_sec + tv1.tv_usec);

				time_t milsec = microsec / 1000;
				time_t sec = milsec / 1000;
				time_t min = sec / 60;
				time_t remainingsec = sec % 60;
				time_t remainingmsec = milsec % 1000;

				char *timestamp = malloc(256);

				sprintf(timestamp, "%d:%.2d.%.3d: Child %d message: %s", min,
						remainingsec, remainingmsec, i + 1, message);

				write(fds[i][WRITE_END], timestamp, strlen(timestamp) + 1);
				//fprintf(fPtr, "Child %d writes %s\n\n", i + 1, timestamp);
				//fprintf(fPtr, "%s\n\n", i + 1, timestamp);

				// Select
				rc = select(FD_SETSIZE, &read_set, NULL, NULL, &timeout);
				if (rc > 0)
				{
					puts("sad read");
					msg_size = read(fds[i][READ_END], read_msg, BUFFER_SIZE);
					read_msg[msg_size] = 0;
					//fprintf(fPtr, "Parent: Read '%s' from the pipe.\n", read_msg);
					fprintf(fPtr, "%s from the terminal\n", read_msg);
				}
				else if (rc == 0)
				{
					printf("@");
					fflush(stdout);
				}
				else
				{
					perror("select");
					exit(1);
				}
			}
			break;
		}

		if ((pids[i] = fork()) < 0)
		{
			perror("fork");
			abort();
		}
		else if (pids[i] == 0)
		{
			int size = 0;
			//Work for child process
			while (time(NULL) - startTime < 30)
			{
				FD_ZERO(&read_set);
				FD_SET(fds[i][READ_END], &read_set);
				sleepDuration = rand() % 3;
				gettimeofday(&tv2, NULL);

				unsigned long microsec = (1000000 * tv2.tv_sec + tv2.tv_usec) - (1000000 * tv1.tv_sec + tv1.tv_usec);

				time_t milsec = microsec / 1000;
				time_t sec = milsec / 1000;
				time_t min = sec / 60;
				time_t remainingsec = sec % 60;
				time_t remainingmsec = milsec % 1000;

				char *timestamp = malloc(500);

				if (i != 4)
				{
					sprintf(timestamp, "%d:%.2d.%.3d: Child %d message %d", min,
							remainingsec, remainingmsec, i + 1, messageNum);
					messageNum++;

					write(fds[i][WRITE_END], timestamp, strlen(timestamp) + 1);
					//fprintf(fPtr, "Child %d writes %s\n", i + 1, timestamp);
					//fprintf(fPtr, "%s\n\n", i + 1, timestamp);

					//fprintf(fPtr, "Child %d sleeps for %d sec\n\n", i + 1, sleepDuration);
					//sleep(sleepDuration);

					// Select
					rc = select(FD_SETSIZE, &read_set, NULL, NULL, &timeout);
					if (rc > 0)
					{
						size = read(fds[i][READ_END], read_msg, BUFFER_SIZE);
						read_msg[size] = 0;
						fprintf(fPtr, "%s from the pipe.\n", read_msg);
						printf("%s from the pipe.\n", read_msg);
					}
					else if (rc == 0)
					{
						printf("@");
						fflush(stdout);
					}
					else
					{
						perror("select");
						exit(1);
					}
				}
				sleep(sleepDuration);
			}
			break;
		}
	}
	fclose(fPtr);
	return 0;
}
