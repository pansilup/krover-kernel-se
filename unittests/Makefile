DYNINST_PATH	:= /home/yph/Projects/kernelSE/dyninst/dyninst_install
DYNINST_INC_PATH		:= ${DYNINST_PATH}/include
DYNINST_LIB_PATH		:= ${DYNINST_PATH}/lib


CXX_FLAGS	:= -O0 -g -ggdb3 -std=c++11


OperandParser: test_OperandParser.cpp
	g++ ${CXX_FLAGS} -I.. -I$(DYNINST_INC_PATH) $^ -o $@  -L..  -Wl,-rpath,'$$ORIGIN/..' -loasis -L$(DYNINST_LIB_PATH) -Wl,-rpath,'${DYNINST_PATH}/lib' -ldyninstAPI -linstructionAPI -lcommon
