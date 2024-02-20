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

extern void client_server(const char *);

int 
main(int argc, char *argv[])
{
    char *client_pid_str = NULL;
    {
        int opt;

        while ((opt = getopt(argc, argv, OPTIONS_CLIENT_SERVER)) != -1) {
            switch (opt) {
            case 'v':
                is_verbose++;
                fprintf(stderr, "Client-server: verbose enabled\n");
                break;
            case 'p':
				fprintf(stderr, "%s\n", optarg);
                client_pid_str = optarg;
                break;
            default:
                fprintf(stderr
                        , "*** opt:%c ptarg: <%s> optind: %d opterr: %d optopt: %d ***\n"
                        , opt, optarg, optind, opterr, optopt);
                break;
            }
        }
    }
    if (client_pid_str == NULL) {
        fprintf(stderr, "Client-server: failed to provide pid string!\n");
        exit(EXIT_FAILURE);
    }

    umask(0);
    fprintf(stderr, "Client-server starting\n");
    client_server(client_pid_str);

    return EXIT_SUCCESS;
}

void
client_server(const char *client_pid_str)
{
    char vifo_name_cmd[PATH_MAX] = { '\0' };
    char vifo_name_data[PATH_MAX] = { '\0' };
    char buffer[BUFFER_SIZE] = {'\0'};
    char cmd[10] = {'\0'};
    int vifo_cmd_fd = -1;
    int vifo_data_fd = -1;
    int result = 0;
    int client_pid = 0;

    result = sscanf(client_pid_str, "%d", &client_pid);
    if (result != 1) {
        perror("Client-server: cannot parse client pid");
        fprintf(stderr, "Client-server: bad pid passed to server \"%s\""
                , client_pid_str);
        exit(2);
    }
    sprintf(vifo_name_cmd, "%s/%s%d"
            , getenv("HOME"), VIFO_CLIENT_CMD, client_pid);
    sprintf(vifo_name_data, "%s/%s%d"
            , getenv("HOME"), VIFO_CLIENT_DATA, client_pid);
    if (is_verbose) {
        fprintf(stdout, "Client-server: cmd vifo \"%s\"\n"
                , vifo_name_cmd);
        fprintf(stdout, "Client-server: data vifo \"%s\"\n"
                , vifo_name_data);
    }
    // open the command vifo for read-only
	vifo_cmd_fd = open(vifo_name_cmd, O_RDONLY);

    for(;;) {
        ssize_t br = 0;

        // read commands from the client
		read(vifo_cmd_fd, cmd, 10);	

        // get the command sent from the client for checking
        if (strcmp(cmd, COMMAND_DIR) == 0) {
            // popen
            FILE *dir = NULL;

            // this will look a lot like the ldir command in
            // the client
        }
        else if (strcmp(cmd, COMMAND_PWD) == 0) {
            // look at the client code to see the other side of this command
            char path[PATH_MAX] = {'\0'};

            getcwd(path, PATH_MAX);
            vifo_data_fd = open(vifo_name_data, O_WRONLY);
            write(vifo_data_fd, path, strlen(path));
            close(vifo_data_fd);
            vifo_data_fd = -1;
        }
        else if (strcmp(cmd, COMMAND_CD) == 0) {
            // change the directory for the client-server
        }
        else if (strcmp(cmd, COMMAND_GET) == 0) {
            int filefd = -1;
            char filename[BUFFER_SIZE] = {'\0'};

            // send a file to the client
        }
        else if (strcmp(cmd, COMMAND_PUT) == 0) {
            int filefd = -1;
            char filename[BUFFER_SIZE] = {'\0'};
            mode_t old_mask = 0;

            // receive a file from the client
        }
        else {
            fprintf(stderr, "Client-server: unknown command \"%s\"\n", cmd);
            // unknown command
        }
    }
}
