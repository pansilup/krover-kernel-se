
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
        //declareSymbolicObject(tmp, 8, 1, 1, 0x0, "who_rsi");
        tmp += 0x8;  //adr of rdi
        declareSymbolicObject(tmp, 8, 1, 1, 1, "which_rdi");
        return true;
        //break;
    case SCALL_SETPRIORITY:
        printf("case: %d\n", (int)scall_idx);
        tmp += 0x60;  //adr of rdx
        printf ("nice value: %d. \n", *((unsigned long*)tmp));
        declareSymbolicObject(tmp, 8, 1, 1, 19, "prio_rdx");
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
        //declareSymbolicObject(tmp, 8, 1, 10, "count_rdx");
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
        //declareSymbolicObject(tmp, 8, 1, 2, "domain_rdi");    
        return true;
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
    unsigned long tmp = m_regs->regs.rdi; //base address of pt_regs object passed to syscall handler
    //unsigned long tmp = m_regs->regs.rsi; //base address of pt_regs object passed to do_syscall_64
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
