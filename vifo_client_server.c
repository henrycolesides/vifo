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
static int vifo_cmd_fd = -1;

extern void client_server(const char *);

void exit_handler(void);

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
	atexit(exit_handler);
    client_server(client_pid_str);

    return EXIT_SUCCESS;
}

void
client_server(const char *client_pid_str)
{
    char vifo_name_cmd[PATH_MAX] = { '\0' };
    char vifo_name_data[PATH_MAX] = { '\0' };
    char buffer[BUFFER_SIZE] = {'\0'};
	char cwd[PATH_MAX] = {'\0'};
   // char cmd[10] = {'\0'};
	char * cmd;
	char * dir;
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
	//vifo_cmd_fd = open(vifo_name_cmd, O_RDONLY);

	vifo_cmd_fd = open(vifo_name_cmd, O_RDONLY);
    for(;;) {
        ssize_t br = 0;

        // read commands from the client
		memset(buffer, 0, BUFFER_SIZE);
		read(vifo_cmd_fd, buffer, BUFFER_SIZE);	
		cmd = strtok(buffer, " \n");
		dir = strtok(NULL, " \n");

        // get the command sent from the client for checking
        if (strcmp(cmd, COMMAND_DIR) == 0) {
            // popen
            FILE *dir = NULL;
			ssize_t br = 0;
            // this will look a lot like the ldir command in
            // the client

			dir = popen(DIR_LISTING, "r");
			vifo_data_fd = open(vifo_name_data, O_WRONLY);
			if (dir != NULL)
			{
				//while((br = read(dir, buffer, BUFFER_SIZE)) > 0)
				while(fgets(buffer, BUFFER_SIZE, dir) != NULL)
				{
					write(vifo_data_fd, buffer, strlen(buffer));	
				}
				pclose(dir);
			}
			close(vifo_data_fd);
			vifo_data_fd = -1;
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
			
			vifo_data_fd = open(vifo_name_data, O_WRONLY);
			dir = &(buffer[3]);
			if(chdir(dir) != 0)
			{
				perror("client cd failed");
				fprintf(stderr, "failed cd \"%s\"\n", dir);
				memset(cwd, 0, PATH_MAX);
				sprintf(cwd, "bad directory \"%s\"", dir);		
				write(vifo_data_fd, cwd, strlen(cwd));
			}
			else
			{
				getcwd(cwd, PATH_MAX);
				write(vifo_data_fd, cwd, strlen(cwd));
			}
			close(vifo_data_fd);
			vifo_data_fd = -1;
            // change the directory for the client-server
        }
        else if (strcmp(cmd, COMMAND_GET) == 0) {
            int filefd = -1;
            char filename[BUFFER_SIZE] = {'\0'};
			vifo_data_fd = open(vifo_name_data, O_WRONLY);
			filefd = open(&buffer[4], O_RDONLY);	
			if(filefd < 0) // Failed to open
			{
				fprintf(stderr, "Client-server: ");
				perror("get failed");
				fprintf(stderr, "Client-server: ");
				fprintf(stderr, "failed get \"%s\"\n", &buffer[4]);
			}
			else
			{
				memset(buffer, 0, BUFFER_SIZE);
				while((br = read(filefd, buffer, BUFFER_SIZE)) != 0)
				{
					write(vifo_data_fd, buffer, br);
				}
				close(filefd);
			}
			close(vifo_data_fd);
			vifo_data_fd = -1;
            // send a file to the client
        }
        else if (strcmp(cmd, COMMAND_PUT) == 0) {
            int filefd = -1;
            //char filename[BUFFER_SIZE] = {'\0'};
            mode_t old_mask = 0;
			//file = dir;

			filefd = open(dir, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
			vifo_data_fd = open(vifo_name_data, O_RDONLY);

			memset(buffer, 0, BUFFER_SIZE);
			while((br = read(vifo_data_fd, buffer, BUFFER_SIZE)) != 0)
			{
				write(filefd, buffer, br);
			}
	
			if(filefd >= 0) close(filefd);
			close(vifo_data_fd);
			vifo_data_fd = -1;
            // receive a file from the client
        }
		else if(strcmp(cmd, COMMAND_QUIT) == 0)
		{
			exit(EXIT_SUCCESS);
		}
        else {
            fprintf(stderr, "Client-server: unknown command \"%s\"\n", cmd);
            // unknown command
        }
    }
}

void exit_handler(void)
{
	close(vifo_cmd_fd);
}
