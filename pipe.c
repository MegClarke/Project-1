#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	if(argc < 2){		//no programs
		exit(22);
	}

	int n = argc - 1;	//num of programs
	int fds[n - 1][2];   // 1 less pipe then process


	//make all pipes
	for(int i = 0; i < n - 1; i++) {
		if(pipe(fds[i]) == -1){//error
			exit(0); //which error?
		}	
	}

	//run each command
	//each command takes input from pipe[i-1] & puts output into pipe[i]
	//special case: i = 0 -> don't redirect input
	//special case: i = n-1 -> don't redirect output
	for(int i = 0; i < n; i++) {
		int pid;
		pid = fork();
		if(pid == 0){
		//child process

			for (int j = 0; j < n - 1; j++) {
                if (j != i || j != (i - 1)) {
                    close(fds[j][0]);
                    close(fds[j][1]);
                }
            }

			//redirect output to pipe[i]
			if (i < n - 1){   //i is 0 to n - 1
				close(fds[i][0]);
				dup2(fds[i][1], 1);
				close(fds[i][1]);
			}
			//redirect input to output of pipe[i - 1]
			else if (i != 0){  //i is anything except 0
				close(fds[i - 1][1]);
				dup2(fds[i - 1][0], 0);
				close(fds[i - 1][0]);
			}
			execlp(argv[i + 1], argv[i + 1], NULL);

			exit(0);
		}

		//parent process
		else if (pid > 0){
			int status;
			waitpid(pid, &status, 0);
			if (WIFEXITED(status)) {	// Child process terminated normally
            	printf("Child process exited with status: %d\n", WEXITSTATUS(status));
        	} else {	// Child process terminated abnormally
            	printf("Child process terminated abnormally\n");
				//error?
        	}

			exit(0);
		}

		else{
			exit(22); //error (which exit?)
		}
	}
	return 0;
}
