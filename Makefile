
DYNINST_PATH	:= /home/neo/smu/kernel-se/dyninst/dyninst_install
DYNINST_INC_PATH		:= ${DYNINST_PATH}/include
DYNINST_LIB_PATH		:= ${DYNINST_PATH}/lib

Z3_PATH	:= /home/neo/smu/kernel-se/z3
Z3_INC_PATH		:= ${Z3_PATH}/include
# Z3_LIB_PATH		:= ${Z3_PATH}/libz3.so
Z3_LIB_PATH		:= ${Z3_PATH}

INC_PATH := -I${DYNINST_INC_PATH}
INC_PATH += -I${Z3_INC_PATH}

LIB_PATH := -L${DYNINST_LIB_PATH}
LIB_PATH += -L${Z3_LIB_PATH}

CC		:= g++
CPPFLAGS	:=-fPIC -c -std=c++11 
# CPPFLAGS	:=-std=c++11 
# CPPFLAGS	:=-std=c++11 -fstack-protector-strong

# LDFLAGS = '-Wl,--rpath=${DYNINST_LIB_PATH}:${Z3_LIB_PATH}'
LDFLAGS= '-Wl,--rpath=${DYNINST_LIB_PATH}:${Z3_PATH}:/usr/lib/x86_64-linux-gnu:${STDLIB_PATH}:/lib/x86_64-linux-gnu' '-Wl,--dynamic-linker=/home/neo/smu/kernel-se/u-loader/build-glibc/elf/ld.so'

# DEP = -ldyninstAPI -lz3
DEP = -lz3 -lparseAPI -linstructionAPI -lsymtabAPI -ldynDwarf -lelf -ldynElf -lcommon -lsymLite 

ifeq (${DEBUG}, 1)
	CPPFLAGS	+= -O0 -g -ggdb3  -DDEBUG
else
	CPPFLAGS	+= -O0
# CPPFLAGS	+= -O0
endif

CPPS	:= centralhub.cpp VMState.cpp CPUState.cpp MemState.cpp fatctrl.cpp thinctrl.cpp symexec.cpp conexec.cpp oprand.cpp Expr.cpp EFlagsManager.cpp Z3Handler.cpp SymList.cpp
OBJS	:= $(patsubst %.cpp,%.o,${CPPS})


all: liboasis.so


$(OBJS):%.o:%.cpp
	$(CC) $(CPPFLAGS) $(INC_PATH) $^ -o $@


liboasis.so: ${OBJS}	
	$(CC) --shared $^ -o $@ $(LIB_PATH) $(LDFLAGS) $(DEP) -std=c++11


clean:
	rm -rf ${OBJS}
