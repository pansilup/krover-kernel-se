# CFLAGS = -g -fpic -pie -O3 -std=c++0x
CFLAGS = -g -fpic -pie -std=c++0x -fno-stack-protector 
# INC = -I/usr/local/include/
DYNINST_PATH	:= /home/neo/smu/kernel-se/dyninst/dyninst_install
DYNINST_INC_PATH		:= ${DYNINST_PATH}/include
DYNINST_LIB_PATH		:= ${DYNINST_PATH}/lib

Z3_PATH	:= /home/neo/smu/kernel-se/z3
Z3_INC_PATH		:= ${Z3_PATH}/include
Z3_LIB_PATH		:= ${Z3_PATH}/libz3.so

# KSE_PATH 		:= /home/jqhong/Documents/OASISKernelSE
KSE_PATH 		:= /home/neo/smu/kernel-se/KRover

LOADER_PATH		:= /home/neo/smu/kernel-se/u-loader/install
STDLIB_PATH 	:= ${LOADER_PATH}/lib
STDINC_PATH 	:= ${LOADER_PATH}/include

INC = -I${KSE_PATH}
LIB = -L${KSE_PATH}

INC += -I${STDINC_PATH}
INC += -I${DYNINST_INC_PATH}
INC += -I${Z3_INC_PATH}

LIB += -L${STDLIB_PATH}
LIB += -L${DYNINST_LIB_PATH}
LIB += -L${Z3_LIB_PATH}

LDFLAGS= '-Wl,--rpath=${DYNINST_LIB_PATH}:${Z3_PATH}:${KSE_PATH}:/usr/lib/x86_64-linux-gnu:${STDLIB_PATH}:/lib/x86_64-linux-gnu' '-Wl,--dynamic-linker=/home/neo/smu/kernel-se/u-loader/build-glibc/elf/ld.so'

# DEP = -loasis -lparseAPI -linstructionAPI -lsymtabAPI -ldynDwarf -lelf -ldynElf -lcommon -lsymLite 
DEP = -loasis 
testtest: main.cpp
	g++ -o testtest main.cpp $(CFLAGS) $(LDFLAGS) $(INC) $(LIB) $(DEP)
clean:
	rm testtest
