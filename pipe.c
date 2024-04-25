#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	if(argc < 2){		//no programs
		perror("no programs \n");
		exit(22);
	}

	int n = argc - 1;	//num of programs
	int fds[n - 1][2];   // 1 less pipe then process
	int pids[n];

	//make all pipes
	for(int i = 0; i < n - 1; i++) {
		if(pipe(fds[i]) == -1){//error
			perror("error piping \n");
			exit(EXIT_FAILURE);
		}	
	}

	//run each command
	//each command takes input from pipe[i-1] & puts output into pipe[i]
	//special case: i = 0 -> don't redirect input
	//special case: i = n-1 -> don't redirect output
	for(int i = 0; i < n; i++) {
		pids[i] = fork();
		if(pids[i] == 0){ //child process
			for (int j = 0; j < n - 1; j++) {
                if (j != i && j != (i - 1)) {
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
			if (i != 0){  //i is anything except 0
				close(fds[i - 1][1]);
				dup2(fds[i - 1][0], 0);
				close(fds[i - 1][0]);
			}
			execlp(argv[i + 1], argv[i + 1], NULL);
			perror("invalid program \n");
			exit(EXIT_FAILURE);
		}
		else if (pids[i] < 0){
			perror("error forking \n");
			exit(EXIT_FAILURE);
		}
	}

	//close all pipes
	for (int i = 0; i < n - 1; i++) {
        close(fds[i][0]);
        close(fds[i][1]);
    }

    //wait
    for (int i = 0; i < n; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) == -1) {
            perror("error waiting");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(status)) {
            perror("Child process %d exited with status %d\n", (i + 1), WEXITSTATUS(status));
        } else {
            perror("Child process %d exited abnormally\n", (i + 1));
			exit(EXIT_FAILURE);
        }
    }

	return 0;
}
