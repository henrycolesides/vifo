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

#define BUFFER_SIZE 1024

// Don't change these
#define COWSAY "cowsay -W60"
#define CRESET "\033[0m"
#define CLEAR  "\033[2J\033[H"

#define FRBW   "\033[31;47m"
#define FGBW   "\033[32;47m"
#define FYBW   "\033[33;47m"
#define FBBW   "\033[34;47m"
#define FPBW   "\033[35;47m"
#define FCBW   "\033[36;47m"


static int  is_verbose = 0;
static char vifo_name_cmd[PATH_MAX] = { '\0' };
static char vifo_name_data[PATH_MAX] = { '\0' };
static int  vifo_cmd_fd = -1;
static int  vifo_data_fd = -1;

// Exit handlers:
void exit_handler(void);
void sigint_handler(int);


// Don't change this
static const char *colors[] = {
    FRBW
    , FGBW
    //, FYBW
    , FBBW
    , FPBW
    //, FCBW
};

// Don't change these
static const char *cheers[] = {
    "You are AMAZING!!!"
    , "You can do this!"
    , "You are good at this stuff!"
    , "Yoda says, Amazing are you!"
    , "You are a star!"
    , "You are an awesome student!"
    , "You are doing great on this assignment!"
    , "Fist bump...  BOOM!!!"
    , "The Force is with you."
    , "This is the Way."
    , "I know you can do this."
    , "Keep up the incredible work!"
    , "You are unstoppable!"
    , NULL
};

// Helpful text is good. This is good enough
static const char *help_string =
    "local commands:\n"
    "\thelp:  display this help\n"
    "\tquit:  exit the client (can also be accomplished with a Control-d)\n"
    "\tclear: clear the screen\n"
    "\tlcd:   change the local directory, the directory for the client\n"
    "\tldir:  display the contents of the present working directory for the client\n"
    "\tlpwd:  display the present working directory for the client\n"
    "\tcheer: shhhh!!!...  It's a secret...\n"
    "\nremote commands:\n"
    "\tcd:    change the remote directory, the directory for the client-server\n"
    "\tdir:   display the contents of the present working directory for the client-server\n"
    "\tpwd:   display the present working directory for the client-server\n"
    "\ntransfer commands:\n"
    "\tget:   copy the given file from the client-server to the client\n"
    "\tput:   copy the given file from the client to the client-server\n"
    ;

// you should have a signal handler for SIGINT
// You should have an exit handler
extern void client(void);

int 
main(int argc, char *argv[])
{
    {
        int opt;

        while ((opt = getopt(argc, argv, OPTIONS_CLIENT)) != -1) {
            switch (opt) {
            case 'v':
                is_verbose++;
                fprintf(stderr, "Client: verbose enabled\n");
                break;
            default:
                fprintf(stderr
                        , "*** opt:%c ptarg: <%s> optind: %d opterr: %d optopt: %d ***\n"
                        , opt, optarg, optind, opterr, optopt);
                break;
            }
        }
    }

    // don't chaneg the call to srand()
    srand(time(NULL));

    atexit(exit_handler);
    signal(SIGINT, sigint_handler);

    if (is_verbose) {
        fprintf(stderr, "Client: Exit handler established."
                "\nClient: SIGINT handler established.\n");
    }
    client();

    return EXIT_SUCCESS;
}

void
client(void)
{
    // put these in the data segment
    static char path[PATH_MAX] = {'\0'};
    static char cwd[PATH_MAX] = { '\0' };
    static char buffer[BUFFER_SIZE] = {'\0'};
    const int num_colors = sizeof(colors) / sizeof(colors[0]);
    int num_cheers = 0;
    char cmd[10] = {'\0'};
    ssize_t br = 0;

    for (; cheers[num_cheers] != NULL; ++num_cheers)
        ;
 
    sprintf(vifo_name_cmd, "%s/%s%d"
            , getenv("HOME"), VIFO_CLIENT_CMD, getpid());
    sprintf(vifo_name_data, "%s/%s%d"
            , getenv("HOME"), VIFO_CLIENT_DATA, getpid());
    if (is_verbose) {
        fprintf(stdout, "Client: cmd vifo >%s<\n"
                , vifo_name_cmd);
        fprintf(stdout, "Client: data vifo >%s<\n"
                , vifo_name_data);
    }
    {
		if(mkfifo(vifo_name_cmd, VIFO_PERMISSIONS) == 0 && is_verbose > 0)
	 	{
			perror("Making vifo failed!");
		}
   		
		if(mkfifo(vifo_name_data, VIFO_PERMISSIONS) == 0 && is_verbose > 0)
	 	{
			perror("Making vifo failed!");
		}
    }
    {
        char vifo_name_server[PATH_MAX] = { '\0' };
        int server_fd = 0;
        
        sprintf(vifo_name_server, "%s/%s", getenv("HOME"), VIFO_SERVER);
        if (is_verbose) {
            fprintf(stderr, "Client: vifo name:\tserver FIFO: %s\n"
                    , vifo_name_server);
        }


        // open the server vifo and write client pid into it (as a string);
		if((server_fd = open(vifo_name_server, O_WRONLY)) > 0)
		{
			int bytes_written = sprintf(buffer, "%d", getpid());
			write(server_fd, buffer, bytes_written);
		}
		else if(is_verbose > 0)
		{
			perror("Opening server vifo failed!");
		}
			
        // close the server vifo
		close(server_fd);
    }

    // open the client command vifo as read only
	vifo_cmd_fd = open(vifo_name_cmd, O_WRONLY);

    for(;;) {
        char *rd_res = NULL;
        int result = 0;

        fprintf(stdout, "%s: %s", "Client", PROMPT);
        memset(buffer, 0, BUFFER_SIZE);
        rd_res = fgets(buffer, BUFFER_SIZE, stdin);
        if (rd_res == NULL) {
            // need message
            exit(EXIT_SUCCESS);
        }
        
        memset(cmd, 0, sizeof(cmd));
        result = sscanf(buffer, "%s", cmd);
        if (result < 1) 
		{
            continue;
        }
        if (strcmp(cmd, COMMAND_DIR) == 0) 
		{
            // send a request to the client-server for the contents of the remote pwd
			write(vifo_cmd_fd, cmd, strlen(cmd));
            fprintf(stdout, "remote dir:\n");
        	
			// Read data in from data vifo
			vifo_data_fd = open(vifo_name_data, O_RDONLY);
			while((br = read(vifo_data_fd, buffer, BUFFER_SIZE)) > 0)
			{
					write(STDOUT_FILENO, buffer, br);
			}
			close(vifo_data_fd);
			vifo_data_fd = -1;
        }
        else if (strcmp(cmd, COMMAND_PWD) == 0) // Test, not sure 
		{
            // This looks pretty easy.
            // send the command to the client-server
            write(vifo_cmd_fd, cmd, strlen(cmd));
            fprintf(stdout, "remote pwd:\n");

            // open the data vifo
            vifo_data_fd = open(vifo_name_data, O_RDONLY);
            while ((br = read(vifo_data_fd, path, PATH_MAX)) > 0) {
                // read data from the client-server
                write(STDOUT_FILENO, path, br);
            }
            // close the data vifo
            close(vifo_data_fd);
            vifo_data_fd = -1;
            printf("\n");
        }
        else if (strcmp(cmd, COMMAND_CD) == 0) // <--- do this
		{
            // send a request to the client-server to change the remote pwd
            // the client-server returns the pwd

        }
        else if (strcmp(cmd, COMMAND_GET) == 0) // <--- do this
		{
            // fetch a file from the client-server
            int filefd = -1;
            char *file = NULL;

        }
        else if (strcmp(cmd, COMMAND_PUT) == 0)  // <--- do this
		{
            // push a file from the client to the client-server
            int filefd = -1;
            char *file = NULL;

        }
        else if (strcmp(cmd, COMMAND_LPWD) == 0) // DONE
		{
            // this is just toooooo easy
            getcwd(cwd, PATH_MAX);
            printf("local pwd:\n%s\n", cwd);
        }
        else if (strcmp(cmd, COMMAND_LCD) == 0)  // <--- do this
		{
            // think about using getcwd()
            // This command has 2 forms. The first form requires
            //   a directory follow the lcd string. In the second
            //   form, no directory follows lcd string, in this
            //   form the current directory is changed to your HOME
            //   directory. Consider the chdir() function.
            char *dir = NULL;

        }
        else if (strcmp(cmd, COMMAND_LDIR) == 0) // DONE
		{
            FILE *dir = NULL;

            fprintf(stdout, "local dir:\n");
            dir = popen(DIR_LISTING, "r");
            if (dir != NULL) 
			{
                while(fgets(buffer, BUFFER_SIZE, dir) != NULL) 
				{
                    printf("%s", buffer);
                    //fputs(buffer, stdout);
                }
                pclose(dir);
            }
        }
        else if (strcmp(cmd, COMMAND_HELP) == 0)  // DONE
		{
            fputs(help_string, stdout);
        }
        else if (strcmp(cmd, COMMAND_CHEER) == 0) // DONE
		{
            // no need to change anything in here
            static char cowsay[PATH_MAX] = { '\0' };
            const char *color = colors[rand() % num_colors];
            const char *cheer = cheers[rand() % num_cheers];

            sprintf(cowsay, COWSAY " \"%s %s %s\"", color, cheer, CRESET);
            system(cowsay);
        }
        else if (strcmp(cmd, COMMAND_CLEAR) == 0)  // DONE
		{
            fputs(CLEAR, stdout);
        }
        else if (strcmp(cmd, COMMAND_QUIT) == 0)  // DONE
		{
            break;
        }
        else 
		{
            // unknown command
            printf("Unrecognized command...\n");
        }
    }
}

void exit_handler(void)
{
	// In the future, this will handle cleanup 
	// to avoid mem leaks
	close(vifo_cmd_fd);
	fprintf(stderr, "Exit handler called!\n");
	unlink(vifo_name_cmd);
	unlink(vifo_name_data);	
}

void sigint_handler(int sig)
{
	printf("Signal for %d caught\n", sig);
	exit(EXIT_SUCCESS);
}
