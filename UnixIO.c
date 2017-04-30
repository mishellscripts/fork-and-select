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
	char write_msg[BUFFER_SIZE] = "You're my child process!";
	char read_msg[BUFFER_SIZE];

	//puts("Test printdfdsfd");

	srand(NULL);
	int messageNum = 1;

	//for (int i = 0; i < 30; i++)
	for (int i = 0; i < 5; i++)
	{

	//if (i % 6 == 0)
	//{
	//int numChild = i / 6;
	pid_t pid;  // child process id
	int fd[2];  // file descriptors for the pipe
	int sleepDuration = rand() % 3;

	signal(SIGALRM,(void (*)(int)) killProcess);

	//puts("Test print-");

	// Create the pipe.
	if (pipe(fd) == -1) {
		fprintf(stderr,"pipe() failed");
		return 1;
	}

	// Fork a child process.

	pid = fork();

	//puts("Test print0");

	if (pid > 0) {
		// PARENT PROCESS.
		//puts("Test print1");

		// Close the unused READ end of the pipe.
		close(fd[READ_END]);

		// Write to the WRITE end of the pipe.
		write(fd[WRITE_END], write_msg, strlen(write_msg)+1);
		printf("Parent: Wrote '%s' to the pipe.\n", write_msg);

		// Close the WRITE end of the pipe.
		close(fd[WRITE_END]);
	}
	else if (pid == 0) {
		// CHILD PROCESS.

		alarm(30);

		for(;;){

		gettimeofday(&tv2, NULL);
		time_t milsec = (tv2.tv_usec/1000) - (tv1.tv_usec/1000);
		time_t sec = (tv2.tv_usec/1000000) - (tv1.tv_usec/1000000);
		time_t min = sec/60;
		time_t remainingsec = sec % 60;
		time_t remainingmsec = milsec % 1000;

		char *timestamp = malloc(100);
		sprintf(timestamp, "%d:%.2d.%.3d: Child %d message %d\n", min,
				remainingsec, remainingmsec, i, messageNum);
		//min + ":" + remainingsec + "." + remainingmsec + "\n";

		write(fd[WRITE_END], timestamp, strlen(timestamp)+1);
		//printf("%d:%.2d.%.3d\n", min, remainingsec, remainingmsec);
		puts(timestamp);

		// Close the unused WRITE end of the pipe.
		close(fd[WRITE_END]);

		// Read from the READ end of the pipe.
		read(fd[READ_END], read_msg, BUFFER_SIZE);
		printf("Child %d: Read '%s' from the pipe.\n", i, read_msg);

		//char tmbuf[64], buf[64];
		///time_t nowtime = tv.tv_sec;
		//struct tm *nowtm = localtime(&nowtime);
		//strftme(tmbuf, sizeof tmbuf, "%M:%S", nowtm);

		//printf("%d\n", localtime(tv.tv_sec));

		// Close the READ end of the pipe.
		close(fd[READ_END]);

		sleep(sleepDuration);
		messageNum++;
	}
	}
	else {
		fprintf(stderr, "fork() failed");
		return 1;
	}
	}
	//}
	return 0;
}

