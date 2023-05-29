CC = gcc
CFLAGS = -Wall -g

PROG = student-record
OBJECTS = $(PROG).o

all : $(PROG) 

$(PROG) : $(OBJECTS)
	gcc -g -o $(PROG) $(OBJECTS)
	
%.o : %.c 
	gcc -c $(CFLAGS) $< -o $@

clean :
	-rm -f $(OBJECTS) $(PROG) 
