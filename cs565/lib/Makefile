# Makefile for hello pass

# Path to top level of LLVM hierarchy
LEVEL = ../..

# Name of the library to build
LIBRARYNAME = cs565opt

# Make the shared library become a loadable module so the tools can
# dlopen/dlsym on the resulting library.
LOADABLE_MODULE = 1

# Might need to change this to .so in LINUX
LIB_EXT = .dylib

TEST_PRINT = $(LEVEL)/../bin/opt -load $(LEVEL)/../lib/$(LIBRARYNAME)$(LIB_EXT) -proj1 <
TEST_PRINT2 = $(LEVEL)/../bin/opt -load $(LEVEL)/../lib/$(LIBRARYNAME)$(LIB_EXT) -proj1b < 
# Include the makefile implementation stuff
include $(LEVEL)/Makefile.common

success:
	$(TEST_PRINT) ../tests/test3.bc > /dev/null 
warning:
	$(TEST_PRINT) ../tests/warning.bc > /dev/null
type_mismatch:
	$(TEST_PRINT) ../tests/type_mismatch.bc > /dev/null
callgraph:
	$(TEST_PRINT2) ../tests/test3.bc > /dev/null


testPrint:
	$(TEST_PRINT) ../tests/test1.bc > /dev/null
	$(TEST_PRINT2) ../tests/test1.bc > /dev/null
