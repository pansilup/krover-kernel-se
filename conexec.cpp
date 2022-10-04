#include <linux/types.h>
#include <sys/mman.h>
#include <signal.h>
#include <ucontext.h>

#include <iostream>

#include "conexec.h"
#include "VMState.h"
#include "interface.h"
#include "symexec.h"
#include "thinctrl.h"
#include "EFlagsManager.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;

extern "C" void InsnExecNonRIP (struct pt_regs* regs);
void InsnExecNonRIP (struct pt_regs* regs);
asm (" .text");
asm (" .type    InsnExecNonRIP, @function");
// asm (" .align 1024");
asm (" .align 4096");
asm ("InsnExecNonRIP: \n");
/* save Ana context on stack */
asm ("pushq %rax \n"); 
asm ("pushq %rbx \n");
asm ("pushq %rcx \n");
asm ("pushq %rdx \n");
asm ("pushq %rdi \n");//0x58
asm ("pushq %rsi \n");//0x50
asm ("pushq %rbp \n");//0x48
asm ("pushq %r8 \n");//0x40
asm ("pushq %r9 \n");//0x38
asm ("pushq %r10 \n");//0x30
asm ("pushq %r11 \n");//0x28
asm ("pushq %r12 \n");//0x20
asm ("pushq %r13 \n");//0x18
asm ("pushq %r14 \n");//0x10
asm ("pushq %r15 \n");//0x8
asm ("pushf \n");//0x0
asm ("movq %rsp, 0xcf(%rip) \n");//Save A-rsp
/* load target context */
asm ("movq (%rdi), %r15 \n");//addr of pt_regs 
asm ("movq 0x8(%rdi), %r14 \n");
asm ("movq 0x10(%rdi), %r13 \n");
asm ("movq 0x18(%rdi), %r12 \n");
asm ("movq 0x20(%rdi), %rbp \n");
asm ("movq 0x28(%rdi), %rbx \n");
asm ("movq 0x30(%rdi), %r11 \n");
asm ("movq 0x38(%rdi), %r10 \n");
asm ("movq 0x40(%rdi), %r9 \n");
asm ("movq 0x48(%rdi), %r8 \n");
asm ("movq 0x50(%rdi), %rax \n");
asm ("movq 0x58(%rdi), %rcx \n");
asm ("movq 0x60(%rdi), %rdx \n");
asm ("movq 0x68(%rdi), %rsi \n");
asm ("push 0x90(%rdi) \n");
asm ("popf \n");
asm ("movq 0x98(%rdi), %rsp \n");
asm ("movq 0x70(%rdi), %rdi \n");
/* 15-byte nop for T instruction */
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
// asm ("movq $0xabcdabcd, %rax \n");
// asm ("vmcall \n");
/* save T context */
// asm ("movq %rsp, 0x7f(%rip) \n");//Save T-rsp
// asm ("movq 0x70(%rip), %rsp \n");//Load A-rsp
asm ("xchg 0x70(%rip), %rsp \n");//Load A-rsp
asm ("push %rdi \n");
asm ("movq 0x60(%rsp), %rdi \n");//addr of pt_regs 
asm ("movq %r15, (%rdi) \n");
asm ("movq %r14, 0x8(%rdi) \n");
asm ("movq %r13, 0x10(%rdi) \n");
asm ("movq %r12, 0x18(%rdi) \n");
asm ("movq %rbp, 0x20(%rdi) \n");
asm ("movq %rbx, 0x28(%rdi) \n");
asm ("movq %r11, 0x30(%rdi) \n");
asm ("movq %r10, 0x38(%rdi) \n");
asm ("movq %r9 , 0x40(%rdi) \n");
asm ("movq %r8 , 0x48(%rdi) \n");
asm ("movq %rax, 0x50(%rdi) \n");
asm ("movq %rcx, 0x58(%rdi) \n");
asm ("movq %rdx, 0x60(%rdi) \n");
asm ("movq %rsi, 0x68(%rdi) \n");
asm ("pop %rsi \n");
asm ("movq %rsi, 0x70(%rdi) \n");//save T-rdi
asm ("pushf \n");
asm ("pop 0x90(%rdi) \n");
asm ("movq 0x20(%rip), %rsi \n");//saved T-rsp
asm ("movq %rsi, 0x98(%rdi) \n");
/* Restore Ana context */
asm ("popf \n");
asm ("popq %r15 \n");
asm ("popq %r14 \n");
asm ("popq %r13 \n");
asm ("popq %r12 \n");
asm ("popq %r11 \n");
asm ("popq %r10 \n");
asm ("popq %r9 \n");
asm ("popq %r8 \n");
asm ("popq %rbp \n");
asm ("popq %rsi \n");
asm ("popq %rdi \n");
asm ("popq %rdx \n");
asm ("popq %rcx \n");
asm ("popq %rbx \n");
asm ("popq %rax \n");
asm ("retq \n");
asm ("nop \n");//saved Ana-RSP/T-RSP 8-byte 
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");

//assume RIP Relative instruction only read rip, do not write rip
//so, load r15 with rip; after execution, no need to update r15 and rip
extern "C" void InsnExecRIP (struct pt_regs* regs);
void InsnExecRIP (struct pt_regs* regs);
asm (" .text");
asm (" .type    InsnExecRIP, @function");
asm ("InsnExecRIP: \n");
/* save Ana context on stack */
asm ("pushq %rax \n"); 
asm ("pushq %rbx \n");
asm ("pushq %rcx \n");
asm ("pushq %rdx \n");
asm ("pushq %rdi \n");//0x58
asm ("pushq %rsi \n");//0x50
asm ("pushq %rbp \n");//0x48
asm ("pushq %r8 \n");//0x40
asm ("pushq %r9 \n");//0x38
asm ("pushq %r10 \n");//0x30
asm ("pushq %r11 \n");//0x28
asm ("pushq %r12 \n");//0x20
asm ("pushq %r13 \n");//0x18
asm ("pushq %r14 \n");//0x10
asm ("pushq %r15 \n");//0x8
asm ("pushf \n");//0x0
asm ("movq %rsp, 0xd0(%rip) \n");//Save A-rsp
/* load target context */
asm ("movq 0x80(%rdi), %r15 \n");//rdi stores addr of pt_regs, load r15 with rip 
// asm ("vmcall \n");
asm ("movq 0x8(%rdi), %r14 \n");
asm ("movq 0x10(%rdi), %r13 \n");
asm ("movq 0x18(%rdi), %r12 \n");
asm ("movq 0x20(%rdi), %rbp \n");
asm ("movq 0x28(%rdi), %rbx \n");
asm ("movq 0x30(%rdi), %r11 \n");
asm ("movq 0x38(%rdi), %r10 \n");
asm ("movq 0x40(%rdi), %r9 \n");
asm ("movq 0x48(%rdi), %r8 \n");
asm ("movq 0x50(%rdi), %rax \n");
asm ("movq 0x58(%rdi), %rcx \n");
asm ("movq 0x60(%rdi), %rdx \n");
asm ("movq 0x68(%rdi), %rsi \n");
asm ("push 0x90(%rdi) \n");
asm ("popf \n");
asm ("movq 0x98(%rdi), %rsp \n");
asm ("movq 0x70(%rdi), %rdi \n");
/* 15-byte nop for T instruction */
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
// asm ("movq $0xabcdabcd, %rax \n");
// asm ("vmcall \n");
/* save T context */
// asm ("movq %rsp, 0x7f(%rip) \n");//Save T-rsp
// asm ("movq 0x70(%rip), %rsp \n");//Load A-rsp
asm ("xchg %rsp, 0x6d(%rip) \n");//restore A-rsp, save T-rsp
asm ("push %rdi \n");
asm ("movq 0x60(%rsp), %rdi \n");//addr of pt_regs 
// asm ("movq %r15, (%rdi) \n");//no need to update r15
asm ("movq %r14, 0x8(%rdi) \n");
asm ("movq %r13, 0x10(%rdi) \n");
asm ("movq %r12, 0x18(%rdi) \n");
asm ("movq %rbp, 0x20(%rdi) \n");
asm ("movq %rbx, 0x28(%rdi) \n");
asm ("movq %r11, 0x30(%rdi) \n");
asm ("movq %r10, 0x38(%rdi) \n");
asm ("movq %r9 , 0x40(%rdi) \n");
asm ("movq %r8 , 0x48(%rdi) \n");
asm ("movq %rax, 0x50(%rdi) \n");
asm ("movq %rcx, 0x58(%rdi) \n");
asm ("movq %rdx, 0x60(%rdi) \n");
asm ("movq %rsi, 0x68(%rdi) \n");
asm ("pop %rsi \n");
asm ("movq %rsi, 0x70(%rdi) \n");//save T-rdi
asm ("pushf \n");
asm ("pop 0x90(%rdi) \n");
asm ("movq 0x20(%rip), %rsi \n");//saved T-rsp
asm ("movq %rsi, 0x98(%rdi) \n");
/* Restore Ana context */
asm ("popf \n");
asm ("popq %r15 \n");
asm ("popq %r14 \n");
asm ("popq %r13 \n");
asm ("popq %r12 \n");
asm ("popq %r11 \n");
asm ("popq %r10 \n");
asm ("popq %r9 \n");
asm ("popq %r8 \n");
asm ("popq %rbp \n");
asm ("popq %rsi \n");
asm ("popq %rdi \n");
asm ("popq %rdx \n");
asm ("popq %rcx \n");
asm ("popq %rbx \n");
asm ("popq %rax \n");
asm ("retq \n");
asm ("nop \n");//saved Ana-RSP/T-RSP 8-byte 
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");

//r15 will be reload with a IMM before the rewritten rip rel insn
extern "C" void BlockExecRIP (struct pt_regs* regs);
void BlockExecRIP (struct pt_regs* regs);
asm (" .text");
asm (" .type    BlockExecRIP, @function");
asm ("BlockExecRIP: \n");
/* save Ana context on stack */
asm ("pushq %rax \n"); 
asm ("pushq %rbx \n");
asm ("pushq %rcx \n");
asm ("pushq %rdx \n");
asm ("pushq %rdi \n");//0x58
asm ("pushq %rsi \n");//0x50
asm ("pushq %rbp \n");//0x48
asm ("pushq %r8 \n");//0x40
asm ("pushq %r9 \n");//0x38
asm ("pushq %r10 \n");//0x30
asm ("pushq %r11 \n");//0x28
asm ("pushq %r12 \n");//0x20
asm ("pushq %r13 \n");//0x18
asm ("pushq %r14 \n");//0x10
asm ("pushq %r15 \n");//0x8
asm ("pushf \n");//0x0
asm ("movq %rsp, 0xdc(%rip) \n");//Save A-rsp into saved_rsp 
/* load target context */
/* r15 will be reload with a IMM in T_page */
// asm ("vmcall \n");
asm ("movq 0x8(%rdi), %r14 \n"); //rdi stores addr of pt_regs
asm ("movq 0x10(%rdi), %r13 \n");
asm ("movq 0x18(%rdi), %r12 \n");
asm ("movq 0x20(%rdi), %rbp \n");
asm ("movq 0x28(%rdi), %rbx \n");
asm ("movq 0x30(%rdi), %r11 \n");
asm ("movq 0x38(%rdi), %r10 \n");
asm ("movq 0x40(%rdi), %r9 \n");
asm ("movq 0x48(%rdi), %r8 \n");
asm ("movq 0x50(%rdi), %rax \n");
asm ("movq 0x58(%rdi), %rcx \n");
asm ("movq 0x60(%rdi), %rdx \n");
asm ("movq 0x68(%rdi), %rsi \n");
asm ("push 0x90(%rdi) \n");
asm ("popf \n");
asm ("movq 0x98(%rdi), %rsp \n");
asm ("movq 0x70(%rdi), %rdi \n");
/* jmp to T_page. 8-nop to store the addr of T_page */
asm ("jmpq *0x88(%rip) \n");//Jump_to_T_Addr
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
/* callback to check if mem involves symbol */
asm ("Call_Back: \n");//actually, it is a jmp insn on T page, so we need to save T rip by ourself
asm ("movq %r15, 0x89(%rip) \n");//Saved_T_RIP
/* BlkExec_RET: save T context except r15 */
asm ("End_T: \n");
asm ("xchg %rsp, 0x7a(%rip) \n");//restore A-rsp, save T-rsp
asm ("push %rdi \n");
asm ("movq 0x60(%rsp), %rdi \n");//addr of pt_regs 
asm ("movq %r14, 0x8(%rdi) \n");
asm ("movq %r13, 0x10(%rdi) \n");
asm ("movq %r12, 0x18(%rdi) \n");
asm ("movq %rbp, 0x20(%rdi) \n");
asm ("movq %rbx, 0x28(%rdi) \n");
asm ("movq %r11, 0x30(%rdi) \n");
asm ("movq %r10, 0x38(%rdi) \n");
asm ("movq %r9 , 0x40(%rdi) \n");
asm ("movq %r8 , 0x48(%rdi) \n");
asm ("movq %rax, 0x50(%rdi) \n");
asm ("movq %rcx, 0x58(%rdi) \n");
asm ("movq %rdx, 0x60(%rdi) \n");
asm ("movq %rsi, 0x68(%rdi) \n");
asm ("pop %rsi \n");
asm ("movq %rsi, 0x70(%rdi) \n");//save T-rdi into pt_regs
asm ("pushf \n");
asm ("pop 0x90(%rdi) \n");
asm ("movq 0x2d(%rip), %rsi \n");//Saved_RSP
asm ("movq %rsi, 0x98(%rdi) \n");//save T-rsp into pt_regs 
/* Restore Ana context */
asm ("popf \n");
asm ("popq %r15 \n");
asm ("popq %r14 \n");
asm ("popq %r13 \n");
asm ("popq %r12 \n");
asm ("popq %r11 \n");
asm ("popq %r10 \n");
asm ("popq %r9 \n");
asm ("popq %r8 \n");
asm ("popq %rbp \n");
asm ("popq %rsi \n");
asm ("popq %rdi \n");
asm ("popq %rdx \n");
asm ("popq %rcx \n");
asm ("popq %rbx \n");
asm ("popq %rax \n");
asm ("retq \n");
asm ("  .align 32 \n");
asm ("Jmp_to_T_Addr: \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("Saved_RSP: \n");
asm ("nop \n");//saved Ana-RSP/T-RSP 8-byte 
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("Saved_T_RIP: \n");
asm ("nop \n");//save next T rip before calling callback 
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");
asm ("nop \n");


// bool ConExecutor::RewRIPInsn(void* orig_insn_addr, Instruction* instr)
// return value indicates how many bytes are copied
int ConExecutor::RewRIPInsn(void* T_insn, void* orig_insn_addr, Instruction* instr)
{
    char orig_insn[12];
    // void* T_insn_no_r15 = (void*)((char*)InsnExecRIP + 0x6e);
    // memcpy (orig_insn, orig_insn_addr, 0x7);
    // memcpy (orig_insn, orig_insn_addr, 0x8);
    // void* T_insn_no_r15 = (void*)((char*)InsnExecRIP + 0x6c);
    
    entryID opcode = instr->getOperation().getID();

    // printf ("T_insn_no_r15 at: %lx. \n", T_insn_no_r15);
        
    static Expression::Ptr x86_gs(new RegisterAST(x86_64::gs));
  
    if (opcode == e_mov || opcode == e_cmp || opcode == e_lea || opcode == e_add || opcode == e_sub)
    {
        memcpy (orig_insn, orig_insn_addr, 0x8);
    
        if (instr->isRead(x86_gs))
        {
            // printf ("gs base rip rel insn. \n");
            if (opcode == e_mov || opcode == e_add)
            {
                if (instr->size() == 7)
                {
                    memcpy (orig_insn+2, (void *)((unsigned long)orig_insn_addr+0x1), 6);
                    orig_insn[1] = 0x41;
                }
                else if(instr->size() == 8)
                {
                    orig_insn[1] |= 0x1;
                }
                else
                {
                    printf ("non recognised gs base insn. \n");
                    asm volatile ("vmcall; \n\t");
                }
                //handling orig_insn[3]
                if (orig_insn[4] == 0 && orig_insn[5] == 0 && orig_insn[6] == 0  && orig_insn[7] == 0)
                {
                    orig_insn[3] |= 0x02;
                    // memcpy(T_insn_no_r15, orig_insn, 4);
                    memcpy(T_insn, orig_insn, 4);
                    return 4;
                }
                else if (orig_insn[4] != 0 && orig_insn[5] == 0 && orig_insn[6] == 0  && orig_insn[7] == 0) 
                {
                    orig_insn[3] |= 0x42;
                    // memcpy(T_insn_no_r15, orig_insn, 5);
                    memcpy(T_insn, orig_insn, 5);
                    return 5;
                }
                else
                {
                    orig_insn[3] |= 0x82;
                    // memcpy(T_insn_no_r15, orig_insn, 8);
                    memcpy(T_insn, orig_insn, 8);
                    return 8;
                }
                // unsigned long* tmp = (unsigned long*)T_insn_no_r15;
                // printf ("new rip-relative insn with gs base: %lx. \n", *tmp);
            }
            else
            {
                asm volatile ("vmcall; \n\t");
            }

        }
        else
        {
            if (instr->size() == 7)//both operand size are 8-byte
            {
                orig_insn[0] |= 0x1;
                if (orig_insn[3] == 0 && orig_insn[4] == 0 && orig_insn[5] == 0  && orig_insn[6] == 0)
                {
                    orig_insn[2] |= 0x02;
                    // memcpy(T_insn_no_r15, orig_insn, 3);
                    memcpy(T_insn, orig_insn, 3);
                    return 3;
                }
                else if (orig_insn[3] != 0 && orig_insn[4] == 0 && orig_insn[5] == 0  && orig_insn[6] == 0) 
                {
                    orig_insn[2] |= 0x42;
                    // memcpy(T_insn_no_r15, orig_insn, 4);
                    memcpy(T_insn, orig_insn, 4);
                    return 4;
                }
                else
                {
                    orig_insn[2] |= 0x82;
                    // memcpy(T_insn_no_r15, orig_insn, 7);
                    memcpy(T_insn, orig_insn, 7);
                    return 7;
                }
            }
            else if (instr->size() == 6)//the non-rip rel operand is 4-byte
            {
                memcpy (orig_insn+1, orig_insn_addr, 6);
                orig_insn[0] = 0x41;
                if (orig_insn[3] == 0 && orig_insn[4] == 0 && orig_insn[5] == 0  && orig_insn[6] == 0)
                {
                    orig_insn[2] |= 0x02;
                    // memcpy(T_insn_no_r15, orig_insn, 3);
                    memcpy(T_insn, orig_insn, 3);
                    return 3;
                }
                else if (orig_insn[3] != 0 && orig_insn[4] == 0 && orig_insn[5] == 0  && orig_insn[6] == 0) 
                {
                    orig_insn[2] |= 0x42;
                    // memcpy(T_insn_no_r15, orig_insn, 4);
                    memcpy(T_insn, orig_insn, 4);
                    return 4;
                }
                else
                {
                    orig_insn[2] |= 0x82;
                    // memcpy(T_insn_no_r15, orig_insn, 7);
                    memcpy(T_insn, orig_insn, 7);
                    return 7;
                }

            }
            else
            {
                asm volatile ("vmcall; \n\t");
            }
        }
    }
    else if (opcode == e_and && instr->isRead(x86_gs))
    { 
        if (instr->size() == 11)
        {
            memcpy (orig_insn, orig_insn_addr, 0x1);
            memcpy (orig_insn+2, (void *)((unsigned long)orig_insn_addr+0x1), 10);
            orig_insn[1] = 0x41;
            // currently, only handles when the displacement is 32-bit
            if (orig_insn[4] != 0 && orig_insn[5] != 0 && orig_insn[6] != 0)
            {
                orig_insn[3] |= 0x82;
                // memcpy(T_insn_no_r15, orig_insn, 12);
                memcpy(T_insn, orig_insn, 12);
                return 12;
            }
            else
            {
                assert(0);//TO FIX: For rip relative operand, the displacement is alway 4-byte long even if the displacement(offset) is 0. But if use r15 as base, the size for displacement is based on the displacement value, can be 0-bit, 8-bit, 16-bit or 32-bit. So, when change rip-->r15, be careful about the displacement size.

            }
            // unsigned long* tmp = (unsigned long*)T_insn_no_r15;
            // printf ("new rip-relative and insn with gs base: %lx. \n", *tmp);
            // printf ("new rip-relative and insn with gs base: %lx. \n", *(tmp+1));
        }
        else
        {
            assert(0);
        }
    }
    else if (opcode == e_xadd)
    {
        if (instr->size() == 8)
        {
            memcpy (orig_insn, orig_insn_addr, 0x1);
            memcpy (orig_insn+2, (void *)((unsigned long)orig_insn_addr+1), 0x7);
            orig_insn[1] = 0x41;
            if (orig_insn[5] != 0 && orig_insn[6] != 0 && orig_insn[7] != 0)
            {
                orig_insn[4] |= 0x82;
                // memcpy(T_insn_no_r15, orig_insn, 9);
                memcpy(T_insn, orig_insn, 9);
                return 9;
            }
            else
            {
                assert(0);
            }
        }
        else
        {
            assert(0);
        }
    }
    else if (opcode == e_test)
    {
        memcpy ((void*)&orig_insn[1], orig_insn_addr, 0x7);
           
        orig_insn[0] = 0x41;
        if (orig_insn[4] == 0 && orig_insn[5] == 0 && orig_insn[6] == 0  && orig_insn[7] == 0)
        {
            orig_insn[2] |= 0x02;
            // memcpy(T_insn_no_r15, orig_insn, 4);
            memcpy(T_insn, orig_insn, 4);
            return 4;
        }
        else if (orig_insn[4] != 0 && orig_insn[5] == 0 && orig_insn[6] == 0  && orig_insn[7] == 0) 
        {
            orig_insn[2] |= 0x42;
            // memcpy(T_insn_no_r15, orig_insn, 5);
            memcpy(T_insn, orig_insn, 5);
            return 5;
        }
        else
        {
            orig_insn[2] |= 0x82;
            // memcpy(T_insn_no_r15, orig_insn, 8);
            memcpy(T_insn, orig_insn, 8);
            return 8;
        }
        // unsigned long* tmp = (unsigned long*)T_insn_no_r15;
        // printf ("new rip-relative test insn: %lx. \n", *tmp);

    }
    else
    {
        printf ("rip-relative instruction, type not handled. \n");
        asm volatile ("movq $0x999999, %%rax; \n\t"
                "vmcall; \n\t"
                :::"%rax");
    }
    // return true;
    return 0;
}
    
bool ConExecutor::ClearTinsn(void* T_addr, int size)
{
    // unsigned long* tmp = (unsigned long*)&NopBytes[0];
    // printf ("nop bytes: %lx. \n", *tmp);
    
    memcpy(T_addr, (void*)&NopBytes[0], size);
    return true;
}

/* InsnDispatch: execute one Insn per time, update T_RIP. Different return value
 * is to facilate Testing */
bool ConExecutor::InsnDispatch(Instruction* instr, struct pt_regs* regs)
{
    int InsnSize = instr->size();

#ifdef _DEBUG_OUTPUT
    std::cout << instr->format() << std::endl;
    // printf ("crtAddr: %lx, insn cate: %d. size: %d. \n", crtAddr, cate, InsnSize);
#endif

    // struct pt_regs* regs = m_regs;
    // asm volatile ("movq $0x999999, %%rax; \n\t"
    //         "vmcall; \n\t"
    //         :::"%rax");
    ulong crtAddr = regs->rip - InsnSize; 
    /* For RIP relative instruction: if r15 is not used in instruction, replace rip with r15 */
    Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(Arch_x86_64)));
    if (instr->isRead(thePC))
    {
        Expression::Ptr theR15(new RegisterAST(MachRegister(x86_64::r15)));
        if (instr->isRead(theR15) || instr->isWritten(theR15))
        {
            printf ("r15 is used, use another reg . \n");
            asm volatile ("movq $0x999999, %%rax; \n\t"
                    "vmcall; \n\t"
                    :::"%rax");
        }
        else
        {
            /* TODO: currently only handle cmp/mov/lea/sub/add which are
             * 7-bytes */
            // printf ("rip relative instruction, addr: %lx. \n", crtAddr);
            
            void* T_insn_no_r15 = (void*)((char*)InsnExecRIP + 0x6c);
            RewRIPInsn(T_insn_no_r15, (void*)crtAddr, instr);
            InsnExecRIP(regs);
            // void* T_insn_no_r15 = (void*)((char*)InsnExecRIP + 0x6c);
            // ClearTinsn(T_insn_no_r15, 8);
            ClearTinsn(T_insn_no_r15, 12);
            
            // std::cout << "rdi: " << std::hex << regs->rdi << std::endl;
        }
    }
    /* For non-rip-relative instruction, */
    else 
    {
        // printf ("non rip relative instruction, addr: %lx. \n", crtAddr);
        // std::cout << "rdi: " << regs->rdi << std::endl;
        // std::cout << "rax: " << regs->rax << std::endl;
        void* T_insn = (void*)((char*)InsnExecNonRIP + 0x68);
        // std::cout << "T_insn at: " << T_insn << std::endl;
        unsigned long* tmp = (unsigned long*) T_insn;
        
        memcpy(T_insn, (void*)crtAddr, InsnSize);
        
#ifdef _DEBUG_OUTPUT            
        printf ("copy T insn: %lx. \n", *tmp);
#endif

        InsnExecNonRIP(regs);
        ClearTinsn(T_insn, InsnSize);

#ifdef _DEBUG_OUTPUT            
        printf ("restore nop: %lx. \n", *tmp);
#endif
       
        // std::cout << "rdi: " << regs->rdi << std::endl;
    }
    
    return 0;
}


// extern "C" void BlockExecRIP (void);
// void BlockExecRIP (void);
// asm (" .text");
// asm (" .type    BlockExecRIP, @function");
// asm ("BlockExecRIP: \n");
// /* save Ana context on stack */
// asm ("pushq %rax \n"); 
// asm ("pushq %rbx \n");
// asm ("pushq %rcx \n");
// asm ("pushq %rdx \n");
// asm ("pushq %rdi \n");
// asm ("pushq %rsi \n");
// asm ("pushq %rbp \n");
// asm ("pushq %r8 \n");
// asm ("pushq %r9 \n");
// asm ("pushq %r10 \n");
// asm ("pushq %r11 \n");
// asm ("pushq %r12 \n");
// asm ("pushq %r13 \n");
// asm ("pushq %r14 \n");
// // asm ("pushq %r15 \n");
// asm ("pushf \n");
// asm ("movq %rsp, 0xdd(%rip) \n");//save ana rsp
// /* load target context */
// asm ("movq $0x7f7fffffec48, %rax \n");//addr of target_ctx
// asm ("movq (%rax), %rbx \n");
// asm ("pushq %rbx \n");
// asm ("popf \n");
// // asm ("movq 0x8(%rax), %r15 \n");
// asm ("movq 0x10(%rax), %r14 \n");
// asm ("movq 0x18(%rax), %r13 \n");
// asm ("movq 0x20(%rax), %r12 \n");
// asm ("movq 0x28(%rax), %rbp \n");
// asm ("movq 0x30(%rax), %rbx \n");
// asm ("movq 0x38(%rax), %r11 \n");
// asm ("movq 0x40(%rax), %r9 \n");
// asm ("movq 0x48(%rax), %r8 \n");
// asm ("movq 0x50(%rax), %r10 \n");
// asm ("movq 0x58(%rax), %rdx \n");
// asm ("movq 0x60(%rax), %rsi \n");
// asm ("movq 0x68(%rax), %rdi \n");
// asm ("movq 0x70(%rax), %rsp \n");
// asm ("movq 0x80(%rax), %rax \n");
// asm ("movq 0x88(%rax), %rcx \n");
// /* jmp to T_page. 8-nop to store the addr of T_page */
// asm ("jmpq *(%rip) \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// /* BlkExec_RET: save T context */
// asm ("movq %rax, 0x7f(%rip) \n");
// asm ("movq $0x7f7fffffec48, %rax \n");//addr of target_ctx
// // asm ("movq %r15, 0x8(%rax) \n");
// asm ("pushf \n");
// asm ("popq %r15 \n");
// asm ("movq %r15, (%rax) \n");//save eflags
// asm ("movq %r14, 0x10(%rax) \n");
// asm ("movq %r13, 0x18(%rax) \n");
// asm ("movq %r12, 0x20(%rax) \n");
// asm ("movq %rbp, 0x28(%rax) \n");
// asm ("movq %rbx, 0x30(%rax) \n");
// asm ("movq %r11, 0x38(%rax) \n");
// asm ("movq %r9, 0x40(%rax) \n");
// asm ("movq %r8, 0x48(%rax) \n");
// asm ("movq %r10, 0x50(%rax) \n");
// asm ("movq %rdx, 0x58(%rax) \n");
// asm ("movq %rsi, 0x60(%rax) \n");
// asm ("movq %rdi, 0x68(%rax) \n");
// asm ("movq %rsp, 0x70(%rax) \n");
// asm ("movq %rcx, 0x88(%rax) \n");
// asm ("movq 0x2d(%rip), %rcx \n");//save T-RAX
// asm ("movq %rcx, 0x80(%rax) \n");
// /* Restore Ana stack & Load Ana context */
// asm ("movq 0x17(%rip), %rsp\n");//restore ana rsp
// asm ("popf \n");
// // asm ("popq %r15 \n");
// asm ("popq %r14 \n");
// asm ("popq %r13 \n");
// asm ("popq %r12 \n");
// asm ("popq %r11 \n");
// asm ("popq %r10 \n");
// asm ("popq %r9 \n");
// asm ("popq %r8 \n");
// asm ("popq %rbp \n");
// asm ("popq %rsi \n");
// asm ("popq %rdi \n");
// asm ("popq %rdx \n");
// asm ("popq %rcx \n");
// asm ("popq %rbx \n");
// asm ("popq %rax \n");
// asm ("retq \n");
// asm ("nop \n");//Ana-RSP 8-byte 
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");//T-RSP 8-byte
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");
// asm ("nop \n");



bool ConExecutor::checkIfMemUseWriteReg(Instruction* in, std::set<int> writeRegIDs)
{
    std::vector<Operand> oprands;
    in->getOperands(oprands);
    for (auto O : oprands) {

#ifdef _DEBUG_OUTPUT            
        std::cout << O.format(Arch_x86_64, 0) << std::endl;
#endif

        if (O.readsMemory())
        {
            // std::set<Expression::Ptr> memrd;
            // O.addEffectiveReadAddresses(memrd);
                
            std::vector<Expression::Ptr> memrd;
            auto V = O.getValue();
            V->getChildren(memrd);
            assert(memrd.size() == 1);  // memory dereference: [xxx] -> xxx
            
#ifdef _DEBUG_OUTPUT            
            std::cout << "++++++++++++memrd size " << memrd.size() << std::endl;
#endif

            auto it = *memrd.begin();

            // assert(memrd.size() == 1);
            // auto it = *memrd.begin();
               
            std::vector<Expression::Ptr> exps;
            it->getChildren(exps);

            for (auto E : exps)
            {
                RegisterAST* reg_ptr = dynamic_cast<RegisterAST*>(E.get());
                if (reg_ptr != nullptr)
                {
                    if (writeRegIDs.find(reg_ptr->getID()) != writeRegIDs.end())
                    {
                        return true;
                    }
                }
            }
        }
        if (O.writesMemory()) 
        {
            // std::set<Expression::Ptr> memwr;
            // O.addEffectiveReadAddresses(memwr);
            
            std::vector<Expression::Ptr> memwr;
            auto V = O.getValue();
            V->getChildren(memwr);
            assert(memwr.size() == 1);  // memory dereference: [xxx] -> xxx
            
#ifdef _DEBUG_OUTPUT            
            std::cout << "+++++++++++memwr size " << memwr.size() << std::endl;
#endif
     
            // assert(memwr.size() == 1);
            auto it = *memwr.begin();
               
            std::vector<Expression::Ptr> exps;
            it->getChildren(exps);

            for (auto E : exps)
            {
                RegisterAST* reg_ptr = dynamic_cast<RegisterAST*>(E.get());
                if (reg_ptr != nullptr)
                {
                    if (writeRegIDs.find(reg_ptr->getID()) != writeRegIDs.end())
                    {
                        return true;
                    }
                }
            }

        }
    }
    return false;
}

bool ConExecutor::checkIfImplicitMemUseWriteReg(Instruction* in, std::set<int> writeRegIDs)
{
    std::set<Expression::Ptr> memrd = in->getOperation().getImplicitMemReads();
    if(memrd.size() != 0)
    {
        for (auto it : memrd)
        {
            std::vector<Expression::Ptr> exps;
            it->getChildren(exps);

            for (auto E : exps)
            {
                RegisterAST* reg_ptr = dynamic_cast<RegisterAST*>(E.get());
                if (reg_ptr != nullptr)
                {
                    if (writeRegIDs.find(reg_ptr->getID()) != writeRegIDs.end())
                    {
                        return true;
                    }
                }
            }
        }
    }
    
    std::set<Expression::Ptr> memwr = in->getOperation().getImplicitMemWrites();
    if (memwr.size() != 0)
    {
        for (auto it : memwr)
        {
            std::vector<Expression::Ptr> exps;
            it->getChildren(exps);

            for (auto E : exps)
            {
                RegisterAST* reg_ptr = dynamic_cast<RegisterAST*>(E.get());
                if (reg_ptr != nullptr)
                {
                    if (writeRegIDs.find(reg_ptr->getID()) != writeRegIDs.end())
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool ConExecutor::InsertInsnUpdateR15(void* nt_t_page, ulong newR15)
{
    char Insn[10];
    Insn[0] = 0x49;
    Insn[1] = 0xbf;
    int i = 0;
    for (i = 2; i < 10; i ++)
    {
        Insn[i] = ((unsigned char)(newR15>>((i-2)*8))) & 0xff;
    }
    memcpy(nt_t_page, Insn, 10);
    return true;
}

int cb_count;
ulong t0, t1, t;
/* S_Addr: starting addr of T insn that has not been copied;
 * crtAddr: current addr of T insn to be copied;
 * T_Insn is a tmp container to store the reassembled insn if it's a rip rel insn; 
 * nt_t_page: next addr on T_page where the following T_Insn are copied to; */
// bool ConExecutor::BlockDispatch(Address S_Addr, Address E_Addr)
// bool ConExecutor::BlockDispatch(Address S_Addr)
bool ConExecutor::BlockDispatch(Address S_Addr, struct pt_regs* m_regs)
{
    void* nt_t_page = T_page;
    Address crtAddr = S_Addr;
    Instruction* in;
    Instruction I;
    InsnCategory cate;

    printf("block dispatch...\n");
#ifdef _DEBUG_OUTPUT            
    printf ("T page: %lx, S_Addr: %lx. \n", T_page, S_Addr);
#endif

    unsigned long BlockExec_Ret = (unsigned long)((unsigned long)BlockExecRIP + 0x7a);//End_T
    unsigned long BlockExec_CallBack = (unsigned long)((unsigned long)BlockExecRIP + 0x73);
    unsigned long* BlkExec_T_page = (unsigned long*)((unsigned long)BlockExecRIP + 0xf3);
    unsigned long* saved_rip_ptr = (unsigned long*)((unsigned long)BlockExecRIP + 0x103);
    
    std::map<ulong, ulong> T_insn_addr_map;//key is Target Insn addr in T_page, value is corresponding original addr 

    bool isR15Used = false;
    bool shouldSymExe = false;
    bool EndPatch = false;
    bool isFlagChangeInsn = false;

    ulong nop_bytes = 0x9090909090909090;
    /* clear previous round T insn on T page */
    memset(nt_t_page, 0x0, 0x1000);
    
    // std::set<RegisterAST::Ptr> writeRegs;
    std::set<int> writeRegIDs;
    std::set<RegisterAST::Ptr> WREGs;
        
    Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(Arch_x86_64)));
    Expression::Ptr theR15(new RegisterAST(MachRegister(x86_64::r15)));

    printf ("S_Addr: %lx. \n", S_Addr);
    printf ("cb_count : %d. \n", cb_count);

loop:
    while (true)
    {
        isR15Used = false;
        shouldSymExe = false;
        EndPatch = false;
       
#ifdef _DEBUG_OUTPUT            
        // printf ("crtAddr: %lx. m_regs->rip : %lx. \n", crtAddr, m_regs->rip);
        printf ("crtAddr: %lx. nt_t_page: %lx. \n", crtAddr, nt_t_page);
        // printf ("nt_t_page: %lx. \n", nt_t_page);
#endif

        /* disassemble insn */
        int idx = crtAddr & 0xFFFFFFF;
        if (m_ThinCtrl->m_InsnCache[idx] == nullptr)
        {
            I = m_ThinCtrl->decoder->decode((unsigned char *)m_ThinCtrl->m_cr->getPtrToInstruction(crtAddr));
            in = new Instruction(I);
            m_ThinCtrl->m_InsnCache[idx] = in;
            // printf("idx: %x, crtAddr: %lx. \n", idx, crtAddr);
            printf ("insn not found in cache :%lx. \n", crtAddr);
            // PreParseOperand(in);
        }
        else
        {
            in = m_ThinCtrl->m_InsnCache[idx];
        }
        cate = in->getCategory();

#ifdef _DEBUG_OUTPUT            
        std::cout << in->format() << std::endl; 
#endif

        /* as long as there is a flag changing insn, clear symbolic flag */
        // if (m_ThinCtrl->m_EFlagsMgr.get()->isFlagChangingInstr(in->getOperation().getID()))
        if (m_ThinCtrl->m_EFlagsMgr->isFlagChangingInstr(in->getOperation().getID()))
            isFlagChangeInsn = true;
       
        if (in->isRead(theR15) || in->isWritten(theR15))
        {
#ifdef _DEBUG_OUTPUT            
            printf ("find a r15 insn. break directly \n");
#endif
            isR15Used = true;
            EndPatch = true;
            break;
        }

        /* if control flow insn, stop copy */
        /* else if: where to dispatch a mem access insn cannot be determined if it uses one of the write reg, insert a call back; Otherwise, determine by hasSymOperand() */
        /* else: non mem access insn, can be determined by hasSymOperand()
         * directly */
        if (cate == c_ReturnInsn || cate == c_CallInsn || cate == c_BranchInsn)
        {
            EndPatch = true;
        }
        else if (!writeRegIDs.empty() && (in->readsMemory() || in->writesMemory()))
        {
            if (checkIfMemUseWriteReg(in, writeRegIDs) || checkIfImplicitMemUseWriteReg(in, writeRegIDs))
            {
                memcpy(nt_t_page, Lea_RIP_Insn, 7);//copy the lea insn to save T rip
                nt_t_page = (void*)((unsigned long)nt_t_page + 7);
                
                T_insn_addr_map[(ulong)nt_t_page] = crtAddr;
                
                memcpy(nt_t_page, Jmp_RIP_Insn, 6);//copy the jmp insn to jump tp callback 
                nt_t_page = (void*)((unsigned long)nt_t_page + 6);
                *((unsigned long*)nt_t_page) = BlockExec_CallBack;
                nt_t_page = (void*)((unsigned long)nt_t_page + 8);

                memcpy(nt_t_page, (void*)crtAddr, in->size());
                // crtAddr += in->size();
                nt_t_page = (void*)((unsigned long)nt_t_page + in->size());

                cb_count ++;

#ifdef _DEBUG_OUTPUT
                printf ("insert a call back at: %lx, nt_t_page: %lx. \n", crtAddr, nt_t_page);
#endif 
                // break;
            }
            else
            {
                shouldSymExe = m_ThinCtrl->hasSymOperand(in);
                if (shouldSymExe)
                    EndPatch = true;
                else
                {
                    if (in->isRead(thePC))
                    {
                        ulong newR15 = crtAddr + in->size();
                        InsertInsnUpdateR15(nt_t_page, newR15);
                        nt_t_page = (void*)((unsigned long)nt_t_page + 10);
                        int s = RewRIPInsn(nt_t_page, (void*)crtAddr, in);
                        nt_t_page = (void*)((unsigned long)nt_t_page + s);

#ifdef _DEBUG_OUTPUT
                        printf ("for rip rel insn, update R15 as: %lx. \n", newR15);
#endif

                    }
                    else
                    {
                        memcpy(nt_t_page, (void*)crtAddr, in->size());
                        // crtAddr += in->size();
                        nt_t_page = (void*)((unsigned long)nt_t_page + in->size());
                    }
                }
            }
        }
        else 
        {
            shouldSymExe = m_ThinCtrl->hasSymOperand(in);
            if (shouldSymExe)
                EndPatch = true;
            else
            {
                if (in->isRead(thePC))
                {
                    ulong newR15 = crtAddr + in->size();
                    InsertInsnUpdateR15(nt_t_page, newR15);
                    nt_t_page =(void*)((unsigned long)nt_t_page + 10);
                    int s = RewRIPInsn(nt_t_page, (void*)crtAddr, in);
                    nt_t_page = (void*)((unsigned long)nt_t_page + s);

#ifdef _DEBUG_OUTPUT
                    printf ("for rip rel insn, update R15 as: %lx. \n", newR15);
#endif
                }
                else
                {
                    memcpy(nt_t_page, (void*)crtAddr, in->size());
                    // crtAddr += in->size();
                    nt_t_page = (void*)((unsigned long)nt_t_page + in->size());
                }
            }
        }

        if (EndPatch)
            break;
        
        /* update write reg set */
        WREGs.clear();
        in->getWriteSet(WREGs);
        for (auto R : WREGs)
            writeRegIDs.insert(R->getID());

        /* update crtAddr */
        crtAddr += in->size();
    }

#ifdef _DEBUG_OUTPUT
    printf ("---------------end of patch, crtAddr: %lx, nt_t_page: %lx. \n", crtAddr, nt_t_page);
#endif

    // /* assume all insn on T_page can be done by CIE, update rip. It can be
    //  * adjusted later if crtAddr != m_regs->rip */
    // m_regs->rip = crtAddr;

   
    /* Copy the jmp insn at the end of T insn block, it fetched the dst from (%rip) */
    memcpy(nt_t_page, Jmp_RIP_Insn, 6);//copy the jmp insn at the end
    nt_t_page = (void*)((unsigned long)nt_t_page + 6);
    *((unsigned long*)nt_t_page) = BlockExec_Ret;
    
    /* Init Jump target of BlockExecRIP as T_page */
    *BlkExec_T_page = (unsigned long)T_page;

#ifdef _DEBUG_OUTPUT
    printf ("BlockExec_T_page: %p, %lx. \n", BlkExec_T_page, *BlkExec_T_page);
    // printf ("crtAddr: %lx. \n", crtAddr);
#endif

    // ulong t0, t1, t;
    
    /* finish all insn on T_page unless a symbolic operand invovled */
    while (true)
    {
#ifdef _DEBUG_OUTPUT
        // printf ("+++++m_regs->rip before BlockExecRIP: %lx. \n", m_regs->rip);
#endif

        *saved_rip_ptr = nop_bytes;

        BlockExecRIP(m_regs);

        printf ("+++++m_regs->rip after BlockExecRIP: %lx. \n", m_regs->rip);
        t0 = rdtsc();

        ulong saved_rip = *saved_rip_ptr;

#ifdef _DEBUG_OUTPUT
        printf ("saved_rip: %lx. \n", saved_rip);
#endif
        // printf ("saved_rip: %lx. \n", saved_rip);

        if (saved_rip != nop_bytes)
        {
            ulong T_insn_addr = T_insn_addr_map[saved_rip];
            assert(T_insn_addr);
            m_regs->rip = T_insn_addr;
            int idx = T_insn_addr & 0xFFFFFFF;
            in = m_ThinCtrl->m_InsnCache[idx];
            assert(in);
            shouldSymExe = m_ThinCtrl->hasSymOperand(in);
            
            t1 = rdtsc();
            t += t1 - t0;
            printf ("=========== \n");
            printf ("t0: %lx, t1: %lx, t: %lx. \n", t0, t1, t);
            
            if (shouldSymExe)
            {
                crtAddr = T_insn_addr;
                break;
                // return false;
            }
            else
            {
                // *BlkExec_T_page = (unsigned long)(saved_rip + 21);
                *BlkExec_T_page = (unsigned long)(saved_rip + 14);
                // *saved_rip_ptr = nop_bytes;
                continue;
            }

            // t1 = rdtsc();
            // t = t1 - t0;
            // printf ("=========== \n");
            // printf ("t0: %lx, t1: %lx, t: %lx. \n", t0, t1, t);

        }
        else
        {
            /* update rip */
            m_regs->rip = crtAddr;
            break;
        }
        // return true;
    }
#ifdef _DEBUG_OUTPUT        
    printf ("//////m_regs->rip after all BlockExecRIPs : %lx. \n", m_regs->rip);
#endif

    if (isFlagChangeInsn)
        m_ThinCtrl->m_VM->clearAllSymFlag();

    /* if insn patching stopped due to R15 involved insn, invoke InsnDiaptchm,
     * then continue the patch */
    if (isR15Used)
    {
        int idx = crtAddr & 0xFFFFFFF;
        in = m_ThinCtrl->m_InsnCache[idx];
        assert(in);
        m_regs->rip += in->size();
#ifdef _DEBUG_OUTPUT
        printf ("invoke InsnDispatch for: %lx. \n", crtAddr);
#endif
        InsnDispatch(in, m_regs);
        m_regs->rip += in->size();

        /* continue to patch insn to T page */
        nt_t_page = T_page;
        writeRegIDs.clear();
        crtAddr += in->size();
    
        /* clear previous round T insn on T page */
        memset(nt_t_page, 0x0, 0x1000);
        
        goto loop;
    }

    if (shouldSymExe) //return to thinCtrl due to symbolic operand
    {
        return true;
    }
    else //return to thinCtrl due to control flow insn
    {
        return false;
    }
}

// ConExecutor::ConExecutor(CThinCtrl* thinCtrl_ptr) 
ConExecutor::ConExecutor() 
{
    /* Make the page where three asm function located writable */
    int ret;
    void* execPage = (void*)(((unsigned long)InsnExecNonRIP) & ~0xFFF);
    ret = mprotect(execPage, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
    // execPage = (void*)(((unsigned long)InsnExecRIP) & ~0xFFF);
    // ret = mprotect(execPage, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
    // execPage = (void*)(((unsigned long)BlockExecRIP) & ~0xFFF);
    // ret = mprotect(execPage, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
    // printf ("ret of mprotect: %d. \n", ret);
    
    /* Init a RIP-relative Jmp Insn which is the last Insn of Block T-Insn Executor */
    auto init = std::initializer_list<unsigned char>({0xff, 0x25, 0x00, 0x00, 0x00, 0x00});
    std::copy(init.begin(), init.end(), Jmp_RIP_Insn);
    T_page = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE | MAP_POPULATE, -1, 0);
    if (T_page == MAP_FAILED)
    {
        printf ("Init T_page failed: %lx. \n", (unsigned long)T_page);
        assert(0);
    }

    /* Init lea (%rip), %r15 */
    init = std::initializer_list<unsigned char>({0x4c, 0x8d, 0x3d, 0x00, 0x00, 0x00, 0x00});
    std::copy(init.begin(), init.end(), Lea_RIP_Insn);


    // /* Init the addr of T_page on BlockExecRIP */
    // unsigned long* BlkExec_T_page = (unsigned long*)(BlockExecRIP + 0x6d);
    // *BlkExec_T_page = (unsigned long)T_page;
    /* / */
}

