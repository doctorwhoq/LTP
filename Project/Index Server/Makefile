# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings

LIBS ?= -lpthread
# the list of header files
#INCL   = server.h
#the list of source files
SRC = server.c
OBJ = $(SRC:.c=.o)

# the build target executable:
TARGET = server

$(TARGET): $(OBJ)
	$(CC) $(CCFLAGS) $(LOCFLAGS) $< $(LIBS) -o $@

$(OBJ): $(INCL)

clean:
	$(RM) $(TARGET) $(OBJ)
