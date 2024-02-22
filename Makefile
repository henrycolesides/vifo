# vifo Makefile, Henry Sides, Lab3, CS333
CC = gcc -g
CFLAGS = -Wextra -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement -Wno-return-local-addr -Wunsafe-loop-optimizations -Wuninitialized -Werror
S = vifo_server
C = vifo_client
CS = vifo_client_server

all: $(S) $(C) $(CS)

$(S): $(S).o 
	$(CC) $(CFLAGS) -o $@ $<

$(S).o: $(S).c vifo.h
	$(CC) $(CFLAGS) -c $<

$(C): $(C).o 
	$(CC) $(CFLAGS) -o $@ $<

$(C).o: $(C).c vifo.h
	$(CC) $(CFLAGS) -c $<

$(CS): $(CS).o 
	$(CC) $(CFLAGS) -o $@ $<

$(CS).o: $(CS).c vifo.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(S) $(C) $(CS) \#*

cfg config: all
	if [ ! -d Testing-server ] ; then mkdir Testing-server; fi ; cd Testing-server ; rm -f * ; ln -fs ../$(S) . ; ln -fs ../$(CS) . ; cp ../?-s.txt .
	for D in 1 2 3 ; do if [ ! -d Testing-client$${D} ] ; then mkdir Testing-client$${D} ; fi ; cd Testing-client$${D} ; rm -f * ; ln -fs ../$(C) . ; cp ../*.bin . ; cd .. ; done
