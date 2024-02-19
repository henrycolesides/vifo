// rchaney@pdx.edu

#pragma once

#ifndef _VIFO_H
# define _VIFO_H 1

// base named for vifos (aka fifos) created for IPC
# define VIFO_SERVER "vifo__server"
# define VIFO_CLIENT_CMD "vifo__command_"
# define VIFO_CLIENT_DATA "vifo__data_"
# define VIFO_PERMISSIONS  (S_IRUSR | S_IWUSR)

# define PROMPT    ">>> "

// local commands: completed within the client
# define COMMAND_LCD   "lcd"   // change the pwd for the client
# define COMMAND_LPWD  "lpwd"  // display the pwd for the client
# define COMMAND_LDIR  "ldir"  // show the contents of client pwd
# define COMMAND_QUIT  "quit"  // exit the client
# define COMMAND_HELP  "help"  // show this AMAZING help text
# define COMMAND_CLEAR "clear" // clear the screen
# define COMMAND_CHEER "cheer" // some fun

// remote commands: sent from client to client-server
// data are returned back to the client
# define COMMAND_DIR  "dir"   // show the contents of client-server pwd
# define COMMAND_PWD  "pwd"   // display the pwd for the client-server
# define COMMAND_CD   "cd"    // change the pwd for the client-server

// transfer commands: transfer a file from client to client-server (put)
//    or from client-server to client. (get)
# define COMMAND_GET  "get"   // fetch a file from client-server to client
# define COMMAND_PUT  "put"   // push a file from client to client-server

// not a lot here
# define OPTIONS_SERVER "v"
# define OPTIONS_CLIENT "v"
// The -p <pid> is a required option to the client-server. It indicates
// pid of the client process with which the client-server will
// communicate.
# define OPTIONS_CLIENT_SERVER "vp:"

// used with popen(), in both client and client-server
# define DIR_LISTING "ls -lFh"

#endif // _VIFO_H
