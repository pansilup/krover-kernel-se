
#include "centralhub.h"

#include <asm/ptrace.h>
#include <linux/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#include <map>
#include <vector>

// #include "BPatch.h"
// #include "BPatch_binaryEdit.h"
// #include "BPatch_flowGraph.h"
// #include "BPatch_function.h"


#include "centralhub.h"
#include "VMState.h"
#include "defines.h"
#include "fatctrl.h"
// #include "interface.h"
#include "thinctrl.h"

using namespace std;
// using namespace Dyninst;

/****************************** ExecState **************************/
ExecState::ExecState(ulong adds, ulong adde)
{
    m_VM.reset(new VMState());
    m_emeta.reset(new EveMeta);
    // exit(0);
    // // return;
    auto F = new CFattCtrl(m_VM.get(), m_emeta.get()); 
    auto T = new CThinCtrl(m_VM.get(), adds, adde);
    F->m_Thin = T;
    m_FattCtrl.reset(F);
    m_ThinCtrl.reset(T);

}

ExecState::~ExecState() {}

// bool ExecState::declareSymbolicObject(ulong addr, ulong size, const char *name) {
//pp-s
//bool ExecState::declareSymbolicObject(ulong addr, ulong size, bool isSigned, long conVal, const char *name) {
//    return m_VM->createSYMemObject(addr, size, isSigned, conVal, name);
//}
bool ExecState::declareSymbolicObject(ulong addr, ulong size, bool isSigned, bool hasSeed, long conVal, const char *name) {
    return m_VM->createSYMemObject(addr, size, isSigned, hasSeed, conVal, name);
}
//pp-e

// bool ExecState::declareSymbolicRegister(uint index, uint size, const char *name) {
//pp-s
//bool ExecState::declareSymbolicRegister(uint index, uint size, bool isSigned, long conVal, const char *name) {
//    return m_VM->createSYRegObject(index, size, isSigned, conVal, name);
//}
bool ExecState::declareSymbolicRegister(uint index, uint size, bool isSigned, bool hasSeed, long conVal, const char *name) {
    return m_VM->createSYRegObject(index, size, isSigned, hasSeed, conVal, name);
}
//pp-e

// bool ExecState::SynRegsFromNative(struct pt_regs* regs)
bool ExecState::SynRegsFromNative(struct MacReg* regs)
{
    VMState::SetCPUState(m_VM.get(), regs);
    return true;
}

// bool ExecState::SynRegsToNative(struct pt_regs* regs)
bool ExecState::SynRegsToNative(struct MacReg* regs)
{
    VMState::ReadCPUState(m_VM.get(), regs);
    return true;
}

bool ExecState::defineSymbolsForScalls(unsigned long scall_idx, unsigned long tmp/*pt_regs_base_adr*/)
{
    /*
    struct pt_regs {
	r15; r14; r13; r12; bp;	
    bx;	 r11; r10; r9;	r8;	
    ax;	 cx;  dx;  si;  di; 
    orig_ax;  ip;  cs;  flags; sp; ss; }
    */
    
    switch (scall_idx)
    {
    case SCALL_GETPRIORITY:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        declareSymbolicObject(tmp, 8, 1, 1, 0x0, "who_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 8, 1, 1, 1, "which_rdi");
        return true;
        //break;
    case SCALL_SETPRIORITY:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x60;  //adr of rdx
        //printf ("nice value: %u. \n", *((unsigned long*)tmp));
        //declareSymbolicObject(tmp, 8, 1, 1, 19, "prio_rdx");
        tmp += 0x8;  //adr of rsi
        declareSymbolicObject(tmp, 8, 1, 1, 0, "who_rsi"); 
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 8, 1, 1, 0x0, "which_rdi");    
        return true;
        //break;
    case SCALL_WRITE:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x60;  //adr of rdx
        //printf ("nice value: %d. \n", *((unsigned long*)tmp));
        //declareSymbolicObject(tmp, 8, 1, 1, 10, "count_rdx");
        tmp += 0x8;  //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 0x0, "buf_rsi"); 
        tmp += 0x8;  //adr of rdi
        //declareSymbolicObject(tmp, 8, 1, 0x0, "fd_rdi");    
        return true;
        //break;
    case SCALL_LSEEK:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x60;  //adr of rdx
        //printf ("nice value: %d. \n", *((unsigned long*)tmp));
        //declareSymbolicObject(tmp, 8, 1, 10, "whence_rdx");
        tmp += 0x8;  //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 0x0, "offset_rsi"); 
        tmp += 0x8;  //adr of rdi
        //declareSymbolicObject(tmp, 8, 1, 0x0, "fd_rdi");    
        return true;
    case SCALL_SOCKET:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x60;  //adr of rdx
        //printf ("nice value: %d. \n", *((unsigned long*)tmp));
        //declareSymbolicObject(tmp, 8, 1, 17, "protocol_rdx");
        tmp += 0x8;  //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 2, "type_rsi"); 
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 8, 1, 1, 2, "domain_rdi");    
        return true;
    case SCALL_ACCESS:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        //declareSymbolicObject(tmp, 8, 1, 1, 1, "filename_rdi");
        return true;
        //break;
    case SCALL_PIPE:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        tmp += 0x8;  //adr of rdi
        unsigned long fd0_adr = *(unsigned long*)tmp;
        unsigned long fd1_adr = fd0_adr + 4;
        printf("tmp : %lx fd2 %d, fd2 %d\n", *(unsigned long*)tmp, *(int *)fd0_adr, *(int *)fd1_adr);
        declareSymbolicObject( fd0_adr, 4, 1, 1, 0x2, "fd1");
        declareSymbolicObject( fd1_adr, 4, 1, 1, 0x1, "fd2");
        return true;
        //break;
    }
    case SCALL_DUP:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 4, 1, 1, 1, "fd_rdi");
        return true;
        //break;
    case SCALL_DUP2:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 4, 1, 1, 1, "fd_rdi");
        return true;
        //break;
    case SCALL_ALARM:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 4, 1, 1, 1, "seconds_rdi");
        return true;
        //break;   
    case SCALL_FCNTL:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 4, 1, 1, 1, "cmd_rdi");
        return true;
        //break;
    case SCALL_STAT:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long filename_start = *(char*)tmp;
        //declareSymbolicObject(filename_start, 4, 0, 1, 0x74657374, "filename_rdi");
        return true;
        //break;
    }
    case SCALL_TRUNCATE:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        declareSymbolicObject(tmp, 8, 1, 1, 0x5, "len_rsi");
        tmp += 0x8;  //adr of rdi
        //declareSymbolicObject(tmp, 4, 1, 1, 1, "cmd_rdi");
        return true;
        //break;
    case SCALL_GETCWD:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        declareSymbolicObject(tmp, 8, 1, 1, 0x128, "len_rsi");
        tmp += 0x8;  //adr of rdi
        //declareSymbolicObject(tmp, 4, 1, 1, 1, "cmd_rdi");
        return true;
        //break;
    case SCALL_CHDIR:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x128, "len_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long directory_name_adr = *(unsigned long*)tmp;
        printf("dir nm :%c%c%c\n", *(char*)directory_name_adr, *(char*)(directory_name_adr+1), *(char*)(directory_name_adr+2) );
        declareSymbolicObject(directory_name_adr    , 1, 0, 1, 0x64, "dirname_rdi_1"); //d
        declareSymbolicObject(directory_name_adr + 1, 1, 0, 1, 0x69, "dirname_rdi_2"); //i
        declareSymbolicObject(directory_name_adr + 2, 1, 0, 1, 0x72, "dirname_rdi_3"); //r
        declareSymbolicObject(directory_name_adr + 3, 1, 0, 1, 0x00, "dirname_rdi_4"); //\0
        return true;
        //break;
    }
    case SCALL_RENAME:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x128, "len_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long old_filename_adr = *(unsigned long*)tmp;
        printf("dir nm :%c%c%c%c\n", *(char*)old_filename_adr, *(char*)(old_filename_adr+1), *(char*)(old_filename_adr+2), *(char*)(old_filename_adr+3) );
        declareSymbolicObject(old_filename_adr    , 1, 0, 1, 0x6f, "fname_rdi_1"); //o
        declareSymbolicObject(old_filename_adr + 1, 1, 0, 1, 0x6c, "fname_rdi_2"); //l
        declareSymbolicObject(old_filename_adr + 2, 1, 0, 1, 0x64, "fname_rdi_3"); //d
        //declareSymbolicObject(old_filename_adr + 3, 1, 0, 1, 0x00, "fname_rdi_4"); //\0
        return true;
        //break;
    }
    case SCALL_MKDIR:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x128, "len_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long directory_name_adr = *(unsigned long*)tmp;
        printf("dir nm :%c%c%c\n", *(char*)directory_name_adr, *(char*)(directory_name_adr+1), *(char*)(directory_name_adr+2) );
        declareSymbolicObject(directory_name_adr    , 1, 0, 1, 0x64, "dirname_rdi_1"); //d
        declareSymbolicObject(directory_name_adr + 1, 1, 0, 1, 0x69, "dirname_rdi_2"); //i
        declareSymbolicObject(directory_name_adr + 2, 1, 0, 1, 0x72, "dirname_rdi_3"); //r
        //declareSymbolicObject(directory_name_adr + 2, 1, 0, 1, 0x00, "dirname_rdi_4"); //\0
        return true;
        //break;
    }
    case SCALL_RMDIR:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x128, "len_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long directory_name_adr = *(unsigned long*)tmp;
        printf("dir nm :%c%c%c\n", *(char*)directory_name_adr, *(char*)(directory_name_adr+1), *(char*)(directory_name_adr+2) );
        declareSymbolicObject(directory_name_adr    , 1, 0, 1, 0x64, "dirname_rdi_1"); //d
        declareSymbolicObject(directory_name_adr + 1, 1, 0, 1, 0x69, "dirname_rdi_2"); //i
        declareSymbolicObject(directory_name_adr + 2, 1, 0, 1, 0x72, "dirname_rdi_3"); //r
        //declareSymbolicObject(directory_name_adr + 2, 1, 0, 1, 0x00, "dirname_rdi_4"); //\0
        return true;
        //break;
    }
    case SCALL_CREAT:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x128, "len_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long file_name_adr = *(unsigned long*)tmp;
        printf("file nm :%c%c%c\n", *(char*)file_name_adr, *(char*)(file_name_adr+1), *(char*)(file_name_adr+2) );
        declareSymbolicObject(file_name_adr    , 1, 0, 1, 0x6f, "fname_rdi_1"); //o
        declareSymbolicObject(file_name_adr + 1, 1, 0, 1, 0x6c, "fname_rdi_2"); //l
        declareSymbolicObject(file_name_adr + 2, 1, 0, 1, 0x64, "fname_rdi_3"); //d
        //declareSymbolicObject(directory_name_adr + 2, 1, 0, 1, 0x00, "dirname_rdi_4"); //\0
        return true;
        //break;
    }
    case SCALL_UMASK:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 8, 1, 1, 0x0, "fd_rdi");
        return true;
        //break;
    case SCALL_GETRLIMIT:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 8, 1, 1, 0x7, "resource_rdi"); //seed val 7 : RLIMIT_NOFILE
        return true;
        //break;
    case SCALL_SETRLIMIT:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 8, 1, 1, 0x7, "resource_rdi"); //seed val 7 : RLIMIT_NOFILE
        return true;
        //break;
    case SCALL_LINK:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x128, "len_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long old_filename_adr = *(unsigned long*)tmp;
        printf("file nm :%c%c%c%c\n", *(char*)old_filename_adr, *(char*)(old_filename_adr+1), *(char*)(old_filename_adr+2), *(char*)(old_filename_adr+3) );
        declareSymbolicObject(old_filename_adr    , 1, 0, 1, 0x6f, "fname_rdi_1"); //o
        declareSymbolicObject(old_filename_adr + 1, 1, 0, 1, 0x6c, "fname_rdi_2"); //l
        declareSymbolicObject(old_filename_adr + 2, 1, 0, 1, 0x64, "fname_rdi_3"); //d
        //declareSymbolicObject(old_filename_adr + 3, 1, 0, 1, 0x00, "fname_rdi_4"); //\0
        return true;
        //break;
    }
    case SCALL_UNLINK:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x128, "len_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long old_filename_adr = *(unsigned long*)tmp;
        printf("file nm :%c%c%c%c\n", *(char*)old_filename_adr, *(char*)(old_filename_adr+1), *(char*)(old_filename_adr+2), *(char*)(old_filename_adr+3) );
        declareSymbolicObject(old_filename_adr    , 1, 0, 1, 0x6f, "fname_rdi_1"); //o
        declareSymbolicObject(old_filename_adr + 1, 1, 0, 1, 0x6c, "fname_rdi_2"); //l
        declareSymbolicObject(old_filename_adr + 2, 1, 0, 1, 0x64, "fname_rdi_3"); //d
        //declareSymbolicObject(old_filename_adr + 3, 1, 0, 1, 0x00, "fname_rdi_4"); //\0
        return true;
        //break;
    }
    case SCALL_SYMLINK:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x128, "len_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long old_filename_adr = *(unsigned long*)tmp;
        printf("file nm :%c%c%c%c\n", *(char*)old_filename_adr, *(char*)(old_filename_adr+1), *(char*)(old_filename_adr+2), *(char*)(old_filename_adr+3) );
        declareSymbolicObject(old_filename_adr    , 1, 0, 1, 0x6f, "fname_rdi_1"); //o
        declareSymbolicObject(old_filename_adr + 1, 1, 0, 1, 0x6c, "fname_rdi_2"); //l
        declareSymbolicObject(old_filename_adr + 2, 1, 0, 1, 0x64, "fname_rdi_3"); //d
        //declareSymbolicObject(old_filename_adr + 3, 1, 0, 1, 0x00, "fname_rdi_4"); //\0
        return true;
        //break;
    }
    case SCALL_CHMOD:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x128, "len_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long old_filename_adr = *(unsigned long*)tmp;
        printf("file nm :%c%c%c%c\n", *(char*)old_filename_adr, *(char*)(old_filename_adr+1), *(char*)(old_filename_adr+2), *(char*)(old_filename_adr+3) );
        declareSymbolicObject(old_filename_adr    , 1, 0, 1, 0x6f, "fname_rdi_1"); //o
        declareSymbolicObject(old_filename_adr + 1, 1, 0, 1, 0x6c, "fname_rdi_2"); //l
        declareSymbolicObject(old_filename_adr + 2, 1, 0, 1, 0x64, "fname_rdi_3"); //d
        //declareSymbolicObject(old_filename_adr + 3, 1, 0, 1, 0x00, "fname_rdi_4"); //\0
        return true;
        //break;
    }
    case SCALL_MKNOD:
    {
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x60; //adr of rdx
        declareSymbolicObject(tmp, 8, 1, 1, 0100000, "mode_rdx"); // type= S_IFREG :0100000
        tmp += 0x8; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 777, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        unsigned long old_filename_adr = *(unsigned long*)tmp;
        printf("node nm :%c%c%c%c\n", *(char*)old_filename_adr, *(char*)(old_filename_adr+1), *(char*)(old_filename_adr+2), *(char*)(old_filename_adr+3) );
        //declareSymbolicObject(old_filename_adr    , 1, 0, 1, 0x6f, "nodename_rdi_1"); //o
        //declareSymbolicObject(old_filename_adr + 1, 1, 0, 1, 0x6c, "nodename_rdi_2"); //l
        //declareSymbolicObject(old_filename_adr + 2, 1, 0, 1, 0x64, "nodename_rdi_3"); //d
        //declareSymbolicObject(old_filename_adr + 3, 1, 0, 1, 0x00, "fname_rdi_4"); //\0
        return true;
        //break;
    }
    case SCALL_SYSFS:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 4, 1, 1, 3, "option_rdi");
        return true;
        //break;
    case SCALL_SCH_GET_PRIO_MAX:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x68; //adr of rsi
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "mode_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 4, 1, 1, 1, "policy_rdi"); //policy = SCHED_FIFO 1
        return true;
        //break;
    default:
        break;
    }

}

// bool ExecState::processAt(ulong addr, struct pt_regs *regs) {
bool ExecState::processAt(ulong addr) {
    
    //pp-s
    unsigned long scall;
    //pp-e
    
    struct MacReg* m_regs = (struct MacReg*) m_VM->getPTRegs();
    printf ("rax: %lx, rdi:%lx, rsi: %lx, rdx: %lx. \n", m_regs->regs.rax, m_regs->regs.rdi, m_regs->regs.rsi, m_regs->regs.rdx);
    
    //pp-s
    //unsigned long tmp = m_regs->regs.rdi; //base address of pt_regs object passed to syscall handler
    unsigned long tmp = m_regs->regs.rsi; //base address of pt_regs object passed to do_syscall_64
    //pp-e

    printf ("fs_base: %lx, gs_base:%lx . \n", m_regs->fs_base, m_regs->gs_base);
    
    //pp-s
    scall = *((unsigned long*)(tmp+0x8*15)); //16th element in pt_regs is syscall no
    printf("syscall idx : %lu\n", scall);
    ExecState::defineSymbolsForScalls(scall, tmp);
    //pp-e

    return m_FattCtrl->processFunc(addr);
}

bool ExecState::MoniStartOfSE(ulong addr) {
    return m_FattCtrl->MoniStartOfSE(addr);
}

void ExecState::InitRediPagePool() {
    return m_FattCtrl->InitRediPagePool();
}

void ExecState::DBHandler() {
    return m_FattCtrl->DBHandler();
}
// /******************************exported for external**************************/
// // CFacade *gHub = nullptr;
// ExecState *es = nullptr;
// 
// EXPORT_ME bool oasis_lib_init(ulong adds, ulong adde) {
//     if (es != nullptr)
//         delete es;
//     es = new ExecState(adds, adde);
// 
//     return true;
// }
// 
// EXPORT_ME void oasis_lib_fini(void) {
//     if (es != nullptr)
//         delete es;
// }
// 
// EXPORT_ME bool StartExecutionAt(ulong addr, struct pt_regs *regs) {
//     // if (gHub == nullptr) {
//     if (es == nullptr) {
//         cout << "invoke oasis_lib_init first to do system initialization\n";
//         exit(EXIT_FAILURE);
//     }
//     return es->processAt(addr, regs);
// }
// 
// EXPORT_ME bool DeclareSymbolicObject(ulong addr, ulong size) {
//     if (es == nullptr) {
//         cout << "invoke oasis_lib_init first to do system initialization\n";
//         exit(EXIT_FAILURE);
//     }
//     return es->declareSymbolicObject(addr, size);
// }

/* Jiaqi */
/* /Jiaqi */

// Module initialization and finalization
__attribute__((constructor)) void module_init(void) {
    // cout << __PRETTY_FUNCTION__ << "\n";
}

__attribute__((destructor)) void module_fini(void) {
    // cout << __PRETTY_FUNCTION__ << "\n";
}
