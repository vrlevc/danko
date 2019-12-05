### Configuration. ####################################################

# Shell for scripts.
SHELL				= /bin/bash
# Default C compiler options.
CFLAGS 				= -Wall -g
# C source files for the resver.
SOURCES				= server.c module.c common.c main.c
# Corresponding object files.
OBJECTS				= $(SOURCES:.c=.o)
# Server module shared library files.
MODULES				= diskfree.so issue.so processes.so time.so
# Tests for server modules
MODULES_TESTS		= $(MODULES:.so=_test)

### Rules. ############################################################

# Phony targets don't correspond to files that are built;
# they are names for conceptual built targets.
.PHONY:				all clean

# Default target: build everything.
all: 				server $(MODULES) $(MODULES_TESTS)

# Clean up build products.
clean:
		rm -f $(OBJECTS) $(MODULES) server $(MODULES_TESTS)

# The mian server program. Link with -Wl,-export-dynamic so dynamicaly
# loaded modules can bind symbols in the program.
# Link in libdl, which contains calls for dynamic loading.
server:				$(OBJECTS)
		$(CC) $(CFLAGS) -Wl,-export-dynamic -o $@ $^ -ldl

# All object files in the server depends on server.h. But use the 
# default rule for building object files from source files.
$(OBJECTS):			server.h

# Rule for building module shared libraries from the corresponding source
# files. Compile -fPIC and generate a shared object file.
$(MODULES):	\
%.so:				%.c server.h
		$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

# Rules for building test for modules using correspoinding test sources
# which include module source files.
$(MODULES_TESTS): \
%:					%.c common.c server.h
		$(CC) $(CFLAGS) -o $@ $^
		./$@

### Notes. ############################################################

# server executable is linked with the -Wl,-export-dynamic compiler option.
# With this option, GCC passes the -export-dynamic option to the linker, 
# which creates an executable file that also exports its external symbols 
# as a shared library.This allows modules, which are dynamically loaded as 
# shared libraries, to reference functions from common.c that are linked 
# statically into the server executable.