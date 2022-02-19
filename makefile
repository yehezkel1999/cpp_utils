#
#	Author: Tal Ben Yehezkel
#
# makefile cheat sheet:
#
# VERY IMPORTANT: makefiles are whitespace sensitive, meaning that 
# unaligned text and/or random whitespaces can cause it not to work,
# so if everything looks proper and it still doesn't work it's probably
# that.
# 
# target: dependencies
# 	commands
#
# when the dependencies change the target gets generated by executing
# the given commands.
# 
# $@: name of the target being generated.
# $<: first dependency.
# $^: all of the dependencies.
# $(VARIABLE): use a variable'
#
# *: search file system for matching file names
# %: either matches one or more characters in a string, replaces the 
# 	 matched string with a string, or used in rule definitions in some 
# 	 functions.
#
# wildcard: generates a space seperated list of names of the existing
# 			files that match one of the given file name patterns.
# patsubst: takes a string to replace, what to replace it with, and a 
# 			list of files, and replaces the strings accourdingly.
# 
# compiler flags:
# -c: create an object file rather than an executable.
# -o: create an executable with a name.
# -g: debug flag
# -Wall: enables compiler warnings

# compiler:
CC:=g++
# compiler flags: 
CFLAGS=-g -Wall
# always on flags:
ALLFLAGS:=-fconcepts -pthread
# linker flags, -L<path>: look in the path for libraries, -lsocket: link with libsocket:
LDFLAGS:=
# remove command (predefined as rm -f):
RM:=rm -f
# zip command:
ZIP:=zip

# .cpp and .h files directory:
SRC:=src
# object files directory:
OBJ:=obj

# all of the source files: 
SRCS:=$(shell find $(SRC)/ -name *.cpp)
# $(info $$SRCS is [${SRCS}])

# all of the object files, from the list of cpp files, swap out .cpp with .o,
# and the location of the file, from src/ to obj/:
OBJS:=$(shell find src/ -name *.cpp | sed s/.cpp/.o/g | sed 's/src/obj/')
#$(info $$OBJS is [${OBJS}])

# all of the header files:
HDRS:=$(wildcard $(SRC)/*.h)

# binary name:
BIN:=utils

# the target to be executed when the makefile is executed (command: make)
all: $(BIN)

# release build: cleans the build and rebuilds it with optimazation flags 
# rather than debug flags (command: make release)
release: CFLAGS=-O2 -DNDEBUG
release: clean
release: $(BIN)

# make the binary with the given compiler, flags and all obj files.
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(ALLFLAGS) -o $@ $(LDFLAGS)

# make every obj file with the corresponding cpp file.
$(OBJ)/%.o: $(SRC)/%.cpp
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(ALLFLAGS) -c $< -o $@

# deletes the binary, all the files in the obj files directory, and 
# debug symbols (command: make clean)
clean:
	$(RM) -r $(OBJ)/
	$(RM) $(BIN)
	$(RM) *.o
