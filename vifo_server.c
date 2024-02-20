// rchaney@pdx.edu
	
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>

#include "vifo.h"

#define BUFFER_SIZE 200

static int is_verbose = 0;
static int vifo_fd = -1;
static char vifo_name[PATH_MAX] = {'\0'};
static sig_atomic_t num_child = 0;

extern void server(void);

// Handlers
void exit_handler(void);
void sigint_handler(int);
void sigchld_handler(int);

int 
main(int argc, char *argv[])
{
    {
        int opt;

        while ((opt = getopt(argc, argv, OPTIONS_SERVER)) != -1) {
            switch (opt) {
            case 'v':
                is_verbose++;
                fprintf(stderr, "Server: verbose enabled\n");
                break;
            default:
                fprintf(stderr
                        , "*** opt:%c ptarg: <%s> optind: %d opterr: %d optopt: %d ***\n"
                        , opt, optarg, optind, opterr, optopt);
                break;
            }
        }
    }

    if (is_verbose) {
        fprintf(stderr, "Server starting\n");
    }
    
    // Handler setup
    atexit(exit_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    server();

    return EXIT_SUCCESS;
}

// Handlers would look good in here
void exit_handler(void)
{
	fprintf(stderr, "Exit handler called!\n");
    close(vifo_fd);
	unlink(vifo_name);
}

void sigint_handler(int sig)
{
	fprintf(stderr, "Sigint handler called!\n");
	exit(EXIT_SUCCESS);
}

void sigchld_handler(int sig)
{
	fprintf(stderr, "Sigchld handler called!\n");
	
	int status;
	pid_t cpid;

	while((cpid = waitpid(-1, &status, 0)) > 0)
	{
		--num_child;
		if(is_verbose > 0) 
		{
			fprintf(stderr, "\nParent signal handler: Found child exit %d: pid: %d exit value: %d\n", sig, cpid, WEXITSTATUS(status));
			if(num_child == 0)
			{
				printf("all child processes reaped\n");
			}
		}
	}

}

void
server(void)
{
//    ssize_t result = 0;
    char buffer[BUFFER_SIZE] = {0};
    pid_t new_server = -1;

    // Create the name of the FIFO using the macro.
    sprintf(vifo_name, "%s/%s", getenv("HOME"), VIFO_SERVER);
    if (is_verbose > 0) {
        fprintf(stderr, "vifo name:\tserver FIFO: %s\n"
                , vifo_name);
    }

    // make a vifo around here
	fprintf(stderr, "%s\n", vifo_name);
	if(mkfifo(vifo_name, VIFO_PERMISSIONS) == 0 && is_verbose > 0)
	{
			perror("Making vifo failed!");
	}

	// Open the FIFO. This call to open() will block until the
    // read side is opened.
	
    for ( ; ; ) {
		vifo_fd = open(vifo_name, O_RDONLY); // Blocks until opened by a client
		if(vifo_fd < 0)
		{
			perror("Opening vifo failed!");
			exit(EXIT_SUCCESS);
		}

		fprintf(stdout, "%s %s", "Server", PROMPT);

        memset(buffer, 0, BUFFER_SIZE);

        // read PID from the server vifo
		read(vifo_fd, buffer, BUFFER_SIZE);	

		fprintf(stdout, "Server: connection from client pid %s\n", buffer);

        // fork here
		new_server = fork();	

        if (new_server < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (0 == new_server) {
            // exec in here
            // be sure to check for a failure from exec
            // if exec fails, call perror and exit
            // I used execlp()
			execlp("./vifo_client_server", "vifo_client_server", "-p", buffer, "-v", (char *) NULL);
			perror("exec failed");
			exit(EXIT_FAILURE);
        }
        else {
			// The parent process
			++num_child;
        }
    }
}
