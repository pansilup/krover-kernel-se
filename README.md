# OASISKernelSE
To test user space arbitrary instructions, 
   in loader/main.cpp
    init_t_ctx(); 
    execState->SynRegsFromNative(&machRegs);
    
    // int index= x86_64::rdx;
    // std::string name = "rdx";
    // execState->declareSymbolicRegister(index, 8, &name[0]);
    execState->processAt(machRegs.regs.rip);
  in centralhub.cpp, processFunction()
   comment the lines which declare symbols.

To test a syscall, install int3 probe at the respective syscall handler, in
loader/main.cpp 
    execState->MoniStartOfSE(0xffffffff810b9440);//addr of indirect call in do_syscall_64 
    to_native();
  then, in centralhub.cpp, processFunction()
    declare memory symbols on kernel stack
