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

extern void server(void);

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
    // this is a good place to put an exit handler
    // this is a good place to have a signal handler for SIGINT
    // this is a good place to have a signal handler for SIGCHLD

    server();

    return EXIT_SUCCESS;
}

// Handlers would look good in here

void
server(void)
{
    ssize_t result = 0;
    char buffer[BUFFER_SIZE] = {0};
    pid_t new_server = -1;

    // Create the name of the FIFO using the macro.
    sprintf(vifo_name, "%s/%s", getenv("HOME"), VIFO_SERVER);
    if (is_verbose > 0) {
        fprintf(stderr, "vifo name:\tserver FIFO: %s\n"
                , vifo_name);
    }

    // make a vifo around here

    // Open the FIFO. This call to open() will block until the
    // read side is opened.

    for ( ; ; ) {
        fprintf(stdout, "%s %s", "Server", PROMPT);

        memset(buffer, 0, BUFFER_SIZE);

        // read from the server vifo

        fprintf(stdout, "Server: connection from client pid %s\n", buffer);

        // fork here
        if (new_server < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (0 == new_server) {
            // exec in here
            // be sure to check for a failure from exec
            // if exec fails, call perror and exit
            // I used execlp()
        }
        else {
            // the parent process
        }
    }
}
