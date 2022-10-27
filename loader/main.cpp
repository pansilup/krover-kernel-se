#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <link.h>
#include <linux/types.h>

#include <asm/ptrace.h>

#include "centralhub.h"
#include "CPUState.h"
// #include "defines.h"
// #include "oprand.h"
// #include "Register.h"
#include "dyn_regs.h"

using namespace Dyninst;

// #define DBG(fmt, ...) \

// #define DBG(fmt, ...) \
//     do {printf ("%s(): " fmt, __func__, ##__VA_ARGS__); } while (0)

/* definition from Target VM */
#define __START_KERNEL_MAP 0xffffffff80000000
#define PAGE_OFFSET 0xffff888000000000


struct shar_arg
{
    volatile unsigned long flag;//1: ready to receive analysis request; 2: a new analysis request issued by guest hyp; 3: analysis request handling done. 
    unsigned long rdi;
    unsigned long rsi;
    unsigned long rdx;
    unsigned long rcx;
    unsigned long r8;
    unsigned long r9;
    unsigned long r11;
    unsigned long r10;
    unsigned long rax;
    unsigned long eflags;
    unsigned long rip;
    unsigned long rsp;
    unsigned long rbx;
    unsigned long rbp;
    unsigned long r12;
    unsigned long r13;
    unsigned long r14;
    unsigned long r15;
    // unsigned long long xmm0;
    // unsigned long long xmm1;
    // unsigned long long xmm2;
    // unsigned long long xmm3;
    // unsigned long long xmm4;
    // unsigned long long xmm5;
    // unsigned long long xmm6;
    // unsigned long long xmm7;
    unsigned long fs_base;
    unsigned long gs_base;
    unsigned long msr_kernel_gs_base;
    unsigned long gdt;
    unsigned long idt;
    unsigned long tss_base;
    unsigned long tss_pg_off;
    unsigned long g_syscall_entry;
    unsigned long pf_entry;
    unsigned long int3_entry;
    unsigned long cr0;
    unsigned long cr2;
    unsigned long cr3;
    unsigned long cr4;
    unsigned long efer;
    unsigned long apic_base_addr;
    unsigned long apic_access_addr;
    unsigned long io_bitmap_a_addr;
    unsigned long io_bitmap_b_addr;
    unsigned long msr_bitmap_addr;
    unsigned long tsc_offset;
    unsigned long exit_reason;
    unsigned long exit_qualification;
    unsigned long inst_len;
    unsigned long event_flag;
    unsigned long entry_intr_info;
    unsigned long user_flag;
    volatile unsigned long guest_timeout_flag;
    volatile unsigned long exit_wrong_flag;
    volatile unsigned long cross_page_flag;
};
struct shar_arg* ei_shar_args;

/* the following two structs is set for load and store convinience */
struct target_context {
    unsigned long eflags;//eflags::rsp are saved on fixed ana stack
    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long rbp;
    unsigned long rbx;
    unsigned long r11;
    unsigned long r9;
    unsigned long r8;
    unsigned long r10;
    unsigned long rdx;
    unsigned long rsi;
    unsigned long rdi;
    unsigned long rsp;
    unsigned long rip;//The rip::rcx need to be copied from board_ctx
    unsigned long rax;
    unsigned long rcx;
};
volatile struct target_context* target_ctx;

//the region to temporaly store target's rax & rcx
struct board_context {
    unsigned long t_db_handler;                             //f98
    unsigned long t_ve_handler;                             //fa0
    unsigned long t_int3_handler;                            //fa8
    unsigned long entry_gate;                               //fb0
    unsigned long pf_handler;                               //fb8
    unsigned long syscall_handler;                          //fc0
    // unsigned long ret_handler;//normal ret from malloc/free //fc8
    unsigned long reserved1;
    unsigned long syscall_exit_handler;//i.e., sysret handler  //fd0
    // unsigned long reserved2;
    // unsigned long rdi;//since malloc&free only have one arg
    unsigned long rcx;                                      //fd8
    unsigned long rax;                                      //fe0
    // unsigned long rsp;
    unsigned long rip;                                      //fe8
};
volatile struct board_context* board_ctx;
    
unsigned long exit_gate_va;
unsigned long idt_va;
unsigned long gdt_va;
unsigned long tss_va;
unsigned long data_page;
unsigned long root_pt_va;
// unsigned long hyp_shar_mem;
// unsigned long klee_shar_mem;
unsigned long ana_t_tss;
unsigned long ana_t_gdt;
unsigned long ana_t_idt;
unsigned long* virt_exce_area;
unsigned long ana_stack;
/* Target #PF uses its original stack as in the guest VM, while #VE, #INT3, #DB
 * use oaais_lib's data page as stack since these event should be transparent to
 * the guest VM */
// unsigned long t_pf_stack;
unsigned long t_int3_stack;
// unsigned long t_ve_stack;
// unsigned long t_db_stack;
unsigned long entry_gate;
unsigned long exit_gate;
unsigned long syscall_exit_gate;
unsigned long t_fsbase;
unsigned long nme_fsbase;
unsigned long* gdt_base;
unsigned long uk_offset;

ExecState* execState;

// struct MacReg {
//     struct pt_regs regs;
//     ulong fs_base;
//     ulong gs_base;
// };

struct MacReg machRegs;

void native_to_SE_ctx_switch();
void SE_to_native_ctx_switch();

// #define max_redirect_idx 8
// // #define max_int3 30
// // struct hook_info probe_orig_inst[max_int3];
// char per_hook[0x1];
// int crt_int3_idx;
// int crt_max_redir_idx;//the current max number of redirected pages
// int crt_redir_idx; //indicate the idx of the current in use redirected page
// unsigned long redirected_pages[max_redirect_idx];
// unsigned long new_pages[max_redirect_idx];
// unsigned long offsets[max_redirect_idx];
// 
// struct pt_regs regs;
// 
// /* initialize page_pool */
// typedef struct pool
// {
//     void* init;
//     void* next;
//     void* end;
// } POOL;
// 
// POOL* page_pool;
// 
// void pool_create (size_t size)
// {
//     void* temp = valloc(size);
//     page_pool->init = temp;
//     page_pool->next = temp;
//     page_pool->end = temp + size;
//     
//     memset (temp, 0x0, size);
//     
//     printf ("redirected page start from :%p. ends : %p. \n", temp, page_pool->end);
//     return;
// }
// 
// void pool_destroy (POOL *p)
// {
//     free(p);
// }
// 
// size_t pool_available (POOL* p)
// {
//     return (unsigned long)p->end - (unsigned long)p->next;
// }
// 
// void* pool_alloc (POOL* p, size_t size)
// {
//     if (pool_available(p) < size)
//     {
//         return NULL;
//     }
//     void* mem = (void*) p->next;
//     p->next += size;
//     return mem;
// }
// /* / */

// void usage(const char *self) {
//     printf("%s oasis_lib hello_lib\n", self);
// }

/*========================================================*/
static __attribute__ ((noinline)) unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    // asm volatile ("int $3;\n\t");
    return ((unsigned long long) lo | ((unsigned long long) hi << 32));
}

void write_fs (unsigned long base)
{
    asm volatile ("movq %0, %%rax; \n\t"
            "wrfsbase %%rax; \n\t"
            ::"m"(base):"%rax");
    return;
}

unsigned long read_fs (void)
{
    unsigned long base;
    asm volatile (
            "rdfsbase %%rax; \n\t"
            "movq %%rax, %0; \n\t"
            ::"m"(base):"%rax");
    return base;
}

void write_gs (unsigned long base)
{
    asm volatile ("movq %0, %%rax; \n\t"
            "wrgsbase %%rax; \n\t"
            ::"m"(base):"%rax");
    return;
}

unsigned long read_gs (void)
{
    unsigned long base;
    asm volatile (
            "rdgsbase %%rax; \n\t"
            "movq %%rax, %0; \n\t"
            ::"m"(base):"%rax");
    return base;
}
unsigned long rdmsr (unsigned long idx)
{
    unsigned long value;
    unsigned long high, low;
    asm volatile ("mov %2, %%ecx; \n\t"
            "rdmsr; \n\t"
            "mov %%edx, %0; \n\t"
            "mov %%eax, %1; \n\t"
            :"=m"(high), "=m"(low):"m"(idx):"%eax","%edx","%ecx");
    value = ((high << 32) & 0xffffffff00000000) | (low & 0xffffffff);
    return value;
}
void wrmsr (unsigned long idx, unsigned long value)
{
    unsigned long high, low;
    high = (value >> 32) & 0xffffffff;
    low = value & 0xffffffff;
    asm volatile ("mov %2, %%ecx; \n\t"
            "mov %0, %%edx; \n\t"
            "mov %1, %%eax; \n\t"
            "wrmsr; \n\t"
            ::"m"(high), "m"(low), "m"(idx):"%eax","%edx","%ecx");
    return;
}
unsigned long rd_cr0 (void)
{
    unsigned long cr0;
    asm volatile ("mov %%cr0, %%rax; \n\t"
            "mov %%rax, %0; \n\t"
            :"=m"(cr0)::"%rax");
    return cr0;
}
unsigned long rd_cr2 (void)
{
    unsigned long cr2;
    asm volatile ("mov %%cr2, %%rax; \n\t"
            "mov %%rax, %0; \n\t"
            :"=m"(cr2)::"%rax");
    return cr2;
}
unsigned long rd_cr4 (void)
{
    unsigned long cr4;
    asm volatile ("mov %%cr4, %%rax; \n\t"
            "mov %%rax, %0; \n\t"
            :"=m"(cr4)::"%rax");
    return cr4;
}
void wr_cr0 (unsigned long cr0)
{
    asm volatile (
            "mov %0, %%rax; \n\t"
            "mov %%rax, %%cr0; \n\t"
            ::"m"(cr0):"%rax");
    return;
}
void wr_cr2 (unsigned long cr2)
{
    asm volatile (
            "mov %0, %%rax; \n\t"
            "mov %%rax, %%cr2; \n\t"
            ::"m"(cr2):"%rax");
    return;
}
void wr_cr4 (unsigned long cr4)
{
    asm volatile (
            "mov %0, %%rax; \n\t"
            "mov %%rax, %%cr4; \n\t"
            ::"m"(cr4):"%rax");
    return;
}

void switch_to_ring0 (void)
{
    void* mem = malloc (10);
    asm volatile ("movq %%rsp, %%rdx; \n\t"
            "movq %0, %%rdi; \n\t"
            "movq $0xffff, %%rsi; \n\t"
            "movq %%rsi, (%%rdi); \n\t"
            "movq $0x63, 0x8(%%rdi); \n\t"
            "REX.W lcall *(%%rdi); \n\t"
            "movq %%rdx, %%rsp; \n\t"
            ::"m"(mem):"%rdi","%rsi", "%rdx");
    return;
}

void restore_user_privilege (void)
{
    asm volatile (
            "movq %%rsp, %%rdi; \n\t"
            "pushq $0x2b; \n\t"
            "pushq %%rdi; \n\t"
            "pushfq; \n\t"
            "lea 0x5(%%rip), %%rdi; \n\t"
            "pushq $0x33; \n\t"
            "pushq %%rdi; \n\t"
            "iretq; \n\t"
            :::"%rdi");
    return;
}


void func(void)
{
    asm volatile ("" : );
    return;
}

/* This call cate is used by analyser to escalate privilege from user to kernel */
void init_call_gate ()
{
    unsigned long* temp_gdt;
    unsigned long call_gate_entry;
    unsigned long call_gate_addr;

    call_gate_addr = (unsigned long) func;
    // temp_gdt = (unsigned long*) shar_args->gdtr;
    temp_gdt = gdt_base;
    call_gate_entry = (call_gate_addr & 0xffff) | (0x10 << 16) | ((unsigned long) (0xec00) << 32) | (((call_gate_addr >> 16) & 0xffff) << 48);
    temp_gdt[12] = call_gate_entry;
    call_gate_entry = (call_gate_addr >> 32) & 0xffffffff;
    temp_gdt[13] = call_gate_entry;
    
    asm volatile ("clflush (%0)" :: "r"(&(temp_gdt[12])));
    
    return;
}
/* /========================================================*/

/*========================================================*/
extern "C" void nme_pf_handler (unsigned long, unsigned long*);
void nme_pf_handler (unsigned long cr2, unsigned long* pf_stack)
{
    int err_code = *pf_stack;
    printf ("cr2: %lx, err_code: %lx. \n", cr2, (unsigned long)err_code);

    // asm volatile ("mov $0xabcdabcd, %rax; \n\t"
    //         "vmcall; \n\t");

    // if (err_code & 0x10)
    // {
    //     printf ("unhandled target #PF, error_code: %x, cr2: %lx. \n", err_code, cr2);
    //     asm volatile ("mov $0xabcdabcd, %rax; \n\t"
    //             "vmcall; \n\t");
    // }
    // else
    // {
    //     nt_ctx->err_code = err_code;
    //     nt_ctx->cr2 = cr2;
    //     nt_ctx->flag = 2;
    //     
    //     do {
    //     } while (nt_ctx->flag != 0);

    //     board_ctx->rip = *(pf_stack + 1);
    //     
    // }
   
    // asm volatile ("movq %0, %%rax; \n\t"
    //         "movq (%%rax), %%rax; \n\t"
    //         ::"m"(cr2):"%rax");

    // asm volatile ("movq $0x9843211, %%rax; \n\t"
    //         "vmcall; \n\t"
    //         :::"%rax");
    return;
}

extern "C" void pf_store_context (void);
void pf_store_context (void);
asm (" .text");
asm (" .type    pf_store_context, @function");
asm ("pf_store_context: \n");
asm ("movq $0xabcdabcd, %rax \n");
asm ("vmcall \n");
asm ("movq %rsp, %rax \n");
asm ("movq $0xfffffeffffffecc0, %rsp \n");//switch to analyser's secure stack
asm ("pushq %rax \n"); // save pf handler rsp in nme stack
asm ("pushq %rdi \n");// 6 syscall args
asm ("pushq %rsi \n");
asm ("pushq %rdx \n");
asm ("pushq %r10 \n");
asm ("pushq %r8 \n");
asm ("pushq %r9 \n");
//asm ("pushq %r10 \n");
asm ("pushq %r11 \n");
asm ("pushq %rbx \n");//the rest of user context
asm ("pushq %rbp \n");
asm ("pushq %r12 \n");
asm ("pushq %r13 \n");
asm ("pushq %r14 \n");
asm ("pushq %r15 \n");
asm ("movq %cr2, %rdi \n");//pass cr2 as 1st arg 
asm ("movq %rax, %rsi \n");// pass pf rsp as 2rd arg

asm ("movsd %xmm0, -0x10(%rsp) \n");
asm ("movsd %xmm1, -0x20(%rsp) \n");
asm ("movsd %xmm2, -0x30(%rsp) \n");
asm ("movsd %xmm3, -0x40(%rsp) \n");
asm ("movsd %xmm4, -0x50(%rsp) \n");
asm ("movsd %xmm5, -0x60(%rsp) \n");
asm ("movsd %xmm6, -0x70(%rsp) \n");
asm ("movsd %xmm7, -0x80(%rsp) \n");
asm ("sub $0x90, %rsp \n");
// asm ("vmcall \n");

asm ("callq nme_pf_handler \n");

asm ("add $0x90, %rsp \n");
asm ("movsd -0x10(%rsp), %xmm0 \n");
asm ("movsd -0x20(%rsp), %xmm1 \n");
asm ("movsd -0x30(%rsp), %xmm2 \n");
asm ("movsd -0x40(%rsp), %xmm3 \n");
asm ("movsd -0x50(%rsp), %xmm4 \n");
asm ("movsd -0x60(%rsp), %xmm5 \n");
asm ("movsd -0x70(%rsp), %xmm6 \n");
asm ("movsd -0x80(%rsp), %xmm7 \n");

// asm ("vmcall \n");
asm ("popq %r15 \n");
asm ("popq %r14 \n");
asm ("popq %r13 \n");
asm ("popq %r12 \n");
asm ("popq %rbp \n");
asm ("popq %rbx \n");
asm ("popq %r11 \n");
// asm ("popq %r10 \n");
asm ("popq %r9 \n");
asm ("popq %r8 \n");
asm ("popq %r10 \n");
asm ("popq %rdx \n");
asm ("popq %rsi \n");
asm ("popq %rdi \n");
asm ("popq %rax \n");//restore to pf stack
asm ("movq %rax, %rsp \n");
asm ("add $0x8, %rsp \n");
asm ("movq $0xfffffe9000905fb0, %rax \n");// change the saved rip in pf stack as the addr of entry_gate
asm ("movq (%rax), %rax \n");
asm ("movq %rax, (%rsp) \n");
asm ("iretq \n");

extern "C" void t_syscall_intercepter (void);
void t_syscall_intercepter (void)
{
    ei_shar_args->fs_base = read_fs();
    write_fs(nme_fsbase);
    printf ("syscall index: %lu. ....., rsp: %lx. \n", board_ctx->rax, target_ctx->rsp);
    // asm volatile ("movq $0xfff, %rax; \n\t"
    //         "vmcall; \n\t");
    // asm volatile ("mov $0xabcdabcd, %rax; \n\t"
    //         "vmcall; \n\t");
    // nt_ctx->sys_num = board_ctx->rax;
    // nt_ctx->rdi = pt_regs->rdi;
    // nt_ctx->rsi = pt_regs->rsi;
    // nt_ctx->rdx = pt_regs->rdx;
    // nt_ctx->r10 = pt_regs->r10;
    // nt_ctx->r8 = pt_regs->r8;
    // nt_ctx->r9 = pt_regs->r9;
    // // nt_ctx->rsp = board_ctx->rsp;
    // // nt_ctx->rflags = pt_regs->rflags;
    // // nt_ctx->rax = pt_regs->rax;
    // nt_ctx->flag = 1;
    // 
    // do {
    // } while (nt_ctx->flag != 0);

    // board_ctx->rax = target_ctx->rax;
    board_ctx->rcx = syscall_exit_gate; 
    board_ctx->rip = ei_shar_args->g_syscall_entry;
    
    // asm volatile ("movq $0x9843211, %%rax; \n\t"
    //         "vmcall; \n\t"
    //         :::"%rax");
    write_fs(ei_shar_args->fs_base);
    
    return;
}

extern "C" void syscall_store_context (void);
void syscall_store_context (void);
asm (" .text");
asm (" .type    syscall_store_context, @function");
asm ("syscall_store_context: \n");
// asm ("movq $0xabcdabcd, %rax \n");
// asm ("vmcall \n");
// // asm ("movq %rsp, %gs: 0xff0 \n");//gs_base contains the lower addr of the stack page used by syscall handler
// // asm ("movq %gs: 0xff8, %rsp \n");// the last element in the stack page is the stack addr used by syscall handler, +0xff0
asm ("movq %rsp, %rax \n"); 
// asm ("movq $0xfffffeffffffecc0, %rsp \n");//switch to analyser's secure stack
// asm ("movq $0x7fffffffecc0, %rsp \n");//switch to analyser's secure stack
asm ("movq $0x7f7fffffecc0, %rsp \n");//switch to analyser's secure stack
asm ("pushq %rax \n");// save target rsp in nme stack
asm ("pushq %rdi \n");// 6 syscall args
asm ("pushq %rsi \n");
asm ("pushq %rdx \n");
asm ("pushq %r10 \n");
asm ("pushq %r8 \n");
asm ("pushq %r9 \n");
// asm ("pushq %r10 \n");
asm ("pushq %r11 \n");
asm ("pushq %rbx \n");//the rest of user context
asm ("pushq %rbp \n");
asm ("pushq %r12 \n");
asm ("pushq %r13 \n");
asm ("pushq %r14 \n");
asm ("pushq %r15 \n");
asm ("pushf \n");
// asm ("movq %rsp, %rdi \n");//pass the address of pt_regs as an arg to sys_handler.

asm ("movsd %xmm0, -0x10(%rsp) \n");
asm ("movsd %xmm1, -0x20(%rsp) \n");
asm ("movsd %xmm2, -0x30(%rsp) \n");
asm ("movsd %xmm3, -0x40(%rsp) \n");
asm ("movsd %xmm4, -0x50(%rsp) \n");
asm ("movsd %xmm5, -0x60(%rsp) \n");
asm ("movsd %xmm6, -0x70(%rsp) \n");
asm ("movsd %xmm7, -0x80(%rsp) \n");
asm ("sub $0x90, %rsp \n");
// asm ("vmcall \n");

// asm ("callq nme_syscall_dispatcher \n");
asm ("callq t_syscall_intercepter \n");

asm ("add $0x90, %rsp \n");
asm ("movsd -0x10(%rsp), %xmm0 \n");
asm ("movsd -0x20(%rsp), %xmm1 \n");
asm ("movsd -0x30(%rsp), %xmm2 \n");
asm ("movsd -0x40(%rsp), %xmm3 \n");
asm ("movsd -0x50(%rsp), %xmm4 \n");
asm ("movsd -0x60(%rsp), %xmm5 \n");
asm ("movsd -0x70(%rsp), %xmm6 \n");
asm ("movsd -0x80(%rsp), %xmm7 \n");

// asm ("vmcall \n");
asm ("popf \n");
asm ("popq %r15 \n");
asm ("popq %r14 \n");
asm ("popq %r13 \n");
asm ("popq %r12 \n");
asm ("popq %rbp \n");
asm ("popq %rbx \n");
asm ("popq %r11 \n");
// asm ("popq %r10 \n");
asm ("popq %r9 \n");
asm ("popq %r8 \n");
asm ("popq %r10 \n");
asm ("popq %rdx \n");
asm ("popq %rsi \n");
asm ("popq %rdi \n");
asm ("popq %rax \n");//restore target rsp into rax
asm ("movq %rax, %rsp \n");
// asm ("movq $0xfffffe9000905fb0, %rax \n");//addr of entry_gate
// asm ("movq $0x7f9000905fb0, %rax \n");//addr of entry_gate
asm ("movq $0x7f1000905fb0, %rax \n");//addr of entry_gate
asm ("jmpq *(%rax) \n");
// asm ("movq $0x, %rax \n"); //addr of entry_gate
// asm ("jmpq *%rax \n");
// asm ("pushq $0x2b \n");//pt_regs->ss
// asm ("pushq %rax \n");//rsp stored in board_ctx
// asm ("pushq %r11 \n");//pt_regs->rflags
// asm ("pushq $0x33 \n");//pt_regs->cs
// asm ("movq $0xfffffe9000905fb0, %rax \n");//the address of board_ctx
// asm ("pushq (%rax) \n");//addr of entry_gate
// // asm ("pushq 0x28(%rax) \n");//syscall #; pt_regs->orig_rax, rax stored in board_ctx
// asm ("iretq \n");

extern "C" void t_syscall_exit (void);
void t_syscall_exit()
{
    ei_shar_args->fs_base = read_fs();
    write_fs(nme_fsbase);
    printf ("syscall ret value: %lx. \n", board_ctx->rax);
    // restore_user_privilege();
    // switch_to_ring0();

    // asm volatile ("movq $0x39, %%rax; \n\t"
    //         "syscall; \n\t"
    //         :::"%rax");
    asm volatile ("movq $0xfff, %rax; \n\t"
            "vmcall; \n\t");
    
    write_fs (ei_shar_args->fs_base);
    return;    
}

extern "C" void syscall_exit_store_context (void);
void syscall_exit_store_context (void);
asm (" .text");
asm (" .type    syscall_exit_store_context, @function");
asm ("syscall_exit_store_context: \n");
// asm ("movq $0xabcdabcd, %rax \n");
// // asm ("movq $0x7f9000905fe0, %rax \n");//addr of rax
// asm ("movq $0x7f1000905fe0, %rax \n");//addr of rax
// asm ("movq (%rax), %rax \n");
// asm ("vmcall \n");
asm ("movq %rsp, %rax \n"); 
// asm ("movq $0xfffffeffffffecc0, %rsp \n");//switch to analyser's secure stack
// asm ("movq $0x7fffffffecc0, %rsp \n");//switch to analyser's secure stack
asm ("movq $0x7f7fffffecc0, %rsp \n");//switch to analyser's secure stack
asm ("pushq %rax \n");// save target rsp in nme stack
asm ("pushq %rdi \n");// 6 syscall args
asm ("pushq %rsi \n");
asm ("pushq %rdx \n");
asm ("pushq %r10 \n");
asm ("pushq %r8 \n");
asm ("pushq %r9 \n");
// asm ("pushq %r10 \n");
asm ("pushq %r11 \n");
asm ("pushq %rbx \n");//the rest of user context
asm ("pushq %rbp \n");
asm ("pushq %r12 \n");
asm ("pushq %r13 \n");
asm ("pushq %r14 \n");
asm ("pushq %r15 \n");
asm ("pushf \n");
// asm ("movq %rsp, %rdi \n");//pass the address of pt_regs as an arg to sys_handler.

asm ("movsd %xmm0, -0x10(%rsp) \n");
asm ("movsd %xmm1, -0x20(%rsp) \n");
asm ("movsd %xmm2, -0x30(%rsp) \n");
asm ("movsd %xmm3, -0x40(%rsp) \n");
asm ("movsd %xmm4, -0x50(%rsp) \n");
asm ("movsd %xmm5, -0x60(%rsp) \n");
asm ("movsd %xmm6, -0x70(%rsp) \n");
asm ("movsd %xmm7, -0x80(%rsp) \n");
asm ("sub $0x90, %rsp \n");
// asm ("vmcall \n");

// asm ("callq nme_syscall_dispatcher \n");
asm ("callq t_syscall_exit \n");

asm ("add $0x90, %rsp \n");
asm ("movsd -0x10(%rsp), %xmm0 \n");
asm ("movsd -0x20(%rsp), %xmm1 \n");
asm ("movsd -0x30(%rsp), %xmm2 \n");
asm ("movsd -0x40(%rsp), %xmm3 \n");
asm ("movsd -0x50(%rsp), %xmm4 \n");
asm ("movsd -0x60(%rsp), %xmm5 \n");
asm ("movsd -0x70(%rsp), %xmm6 \n");
asm ("movsd -0x80(%rsp), %xmm7 \n");

// asm ("vmcall \n");
asm ("popf \n");
asm ("popq %r15 \n");
asm ("popq %r14 \n");
asm ("popq %r13 \n");
asm ("popq %r12 \n");
asm ("popq %rbp \n");
asm ("popq %rbx \n");
asm ("popq %r11 \n");
// asm ("popq %r10 \n");
asm ("popq %r9 \n");
asm ("popq %r8 \n");
asm ("popq %r10 \n");
asm ("popq %rdx \n");
asm ("popq %rsi \n");
asm ("popq %rdi \n");
asm ("popq %rax \n");//restore target rsp into rax
asm ("movq %rax, %rsp \n");
asm ("vmcall \n");
// // asm ("movq %rsp, %gs: 0xff0 \n");//gs_base contains the lower addr of the stack page used by syscall handler
// // asm ("movq %gs: 0xff8, %rsp \n");// the last element in the stack page is the stack addr used by syscall handler, +0xff0
// asm ("movq %rsp, %rax \n"); 
// asm ("movq $0xfffffeffffffecc0, %rsp \n");//switch to analyser's secure stack
// asm ("pushq %rax \n");// save target rsp in nme stack

// /* return the addr of call destination */
// unsigned long emulate_call (void)
// {
//     unsigned long* int3_stack_ptr = (unsigned long*)(t_int3_stack - 0x28);
//     unsigned long saved_rip, saved_rsp, saved_rflags;
//     int probe_idx;
//     unsigned long ret_addr, call_dest;
//     unsigned long* t_stack_ptr;
//     saved_rip = int3_stack_ptr[0];
//     saved_rip -= 1;// for int3, saved rip is the rip next to int3
//     saved_rflags = int3_stack_ptr[2];
//     saved_rsp = int3_stack_ptr[3];
//     printf ("saved rip: %lx. rsp: %lx, rflags: %lx. \n", saved_rip, saved_rsp, saved_rflags);
//     // asm volatile ("movq $0xfff, %%rax; \n\t"
//     //         "vmcall; \n\t"
//     //         :::"%rax");
//     
//     probe_idx = find_probe_idx(saved_rip);
//     call_dest = probe_orig_inst[probe_idx].dest;//resolve the call destination based on the saved rip in int3 stack.
//    
//     // printf ("target_ctx at: %p. target_rsp: %lx, target_rflags: %lx. dest: %lx. rdi: %lx. \n", target_ctx, target_ctx->rsp, target_ctx->eflags, call_dest, target_ctx->rdi);
//     // printf ("rsi: %lx, rdx: %lx. \n", target_ctx->rsi, target_ctx->rdx);
//   
//     ret_addr = saved_rip + probe_orig_inst[probe_idx].len;  
//     t_stack_ptr = (unsigned long*)saved_rsp;
//     t_stack_ptr --;
//     *t_stack_ptr = ret_addr;
//     // printf ("new stack: %p, ret_addr: %lx. \n", t_stack_ptr, ret_addr);
// 
//     target_ctx->rsp = (unsigned long)t_stack_ptr;
//     target_ctx->eflags = saved_rflags;
//     
//     board_ctx->rip = call_dest;
//     
//     printf ("after adjustment... target_rsp: %lx, target_rflags: %lx. rdi: %lx. \n", target_ctx->rsp, target_ctx->eflags, target_ctx->rdi);
//     // printf ("rsi: %lx, rdx: %lx. \n", target_ctx->rsi, target_ctx->rdx);
// 
//     return call_dest;
// }
// 

extern "C" void int3_handler (void);
void int3_handler(void)
{
    ei_shar_args->fs_base = read_fs();
    write_fs (nme_fsbase);
    
    unsigned long* int3_stack_ptr = (unsigned long*)(t_int3_stack - 0x28);
    unsigned long saved_rip, saved_rsp, saved_rflags;
    // int probe_idx;
    // unsigned long ret_addr, call_dest;
    unsigned long* t_stack_ptr;
    saved_rip = int3_stack_ptr[0];
    saved_rip -= 1;// for int3, saved rip is the rip next to int3
    saved_rsp = int3_stack_ptr[3];
    saved_rflags = int3_stack_ptr[2];
    
    
    target_ctx->rax = board_ctx->rax;
    target_ctx->rcx = board_ctx->rcx;
    target_ctx->rip = saved_rip + 0x5;
    target_ctx->rsp = saved_rsp;
    target_ctx->eflags = saved_rflags;
    printf ("int3 invoked. saved rip: %lx. saved rsp: %lx, saved_rflags: %lx, ret address on stack: %lx. \n", saved_rip, saved_rsp, saved_rflags, *((unsigned long*)saved_rsp));
    
    // // asm volatile ("movq $0xfff, %%rax; \n\t"
    // //         "vmcall; \n\t"
    // //         :::"%rax");
    // unsigned long call_dest = emulate_call();
    // 
    // install_int3_func(call_dest);
    
    native_to_SE_ctx_switch();
    execState->SynRegsFromNative(&machRegs);
    
    // execState->SynFsGsBaseToSE(ei_shar_args->msr_kernel_gs_base, ei_shar_args->fs_base);
    // execState->INT3Handler();
    // printf ("int3 invoked. saved rip: %lx. saved rsp: %lx, ret address on stack: %lx. \n", regs.rip, saved_rsp, *((unsigned long*)saved_rsp));
    
    // asm volatile ("movq $0xfff, %%rax; \n\t"
    //         "vmcall; \n\t"
    //         :::"%rax");

    execState->processAt(machRegs.regs.rip);
    
    execState->SynRegsToNative(&machRegs);
    SE_to_native_ctx_switch();
    
    // // concretExecTest(call_dest);
    // concretBlockExecTest(call_dest);
    
    board_ctx->rax = target_ctx->rax; 
    board_ctx->rcx = target_ctx->rcx; 
    
    write_fs (ei_shar_args->fs_base);
    return;
}

extern "C" void int3_store_context (void);
void int3_store_context (void);
asm (" .text");
asm (" .type    int3_store_context, @function");
asm ("int3_store_context: \n");
// asm ("movq $0xabcdabcd, %rax \n");
// asm ("vmcall \n");
asm ("movq %rsp, %rax \n"); 
// asm ("movq $0xfffffeffffffecc0, %rsp \n");//switch to analyser's secure stack
// asm ("movq $0x7fffffffecc0, %rsp \n");//switch to analyser's secure stack
asm ("movq $0x7f7fffffecc0, %rsp \n");//switch to analyser's secure stack
asm ("pushq %rax \n");// save target rsp in nme stack
asm ("pushq %rdi \n");// 6 syscall args
asm ("pushq %rsi \n");
asm ("pushq %rdx \n");
asm ("pushq %r10 \n");
asm ("pushq %r8 \n");
asm ("pushq %r9 \n");
// asm ("pushq %r10 \n");
asm ("pushq %r11 \n");
asm ("pushq %rbx \n");//the rest of user context
asm ("pushq %rbp \n");
asm ("pushq %r12 \n");
asm ("pushq %r13 \n");
asm ("pushq %r14 \n");
asm ("pushq %r15 \n");
asm ("pushf \n");
// asm ("movq %rsp, %rdi \n");//pass the address of pt_regs as an arg to sys_handler.

asm ("movsd %xmm0, -0x10(%rsp) \n");
asm ("movsd %xmm1, -0x20(%rsp) \n");
asm ("movsd %xmm2, -0x30(%rsp) \n");
asm ("movsd %xmm3, -0x40(%rsp) \n");
asm ("movsd %xmm4, -0x50(%rsp) \n");
asm ("movsd %xmm5, -0x60(%rsp) \n");
asm ("movsd %xmm6, -0x70(%rsp) \n");
asm ("movsd %xmm7, -0x80(%rsp) \n");
// asm ("sub $0x90, %rsp \n");
asm ("sub $0x98, %rsp \n");//To ensure the stack 16-byte aligned
// asm ("vmcall \n");

// asm ("callq nme_syscall_dispatcher \n");
asm ("callq int3_handler \n");

asm ("add $0x98, %rsp \n");
// asm ("add $0x90, %rsp \n");
asm ("movsd -0x10(%rsp), %xmm0 \n");
asm ("movsd -0x20(%rsp), %xmm1 \n");
asm ("movsd -0x30(%rsp), %xmm2 \n");
asm ("movsd -0x40(%rsp), %xmm3 \n");
asm ("movsd -0x50(%rsp), %xmm4 \n");
asm ("movsd -0x60(%rsp), %xmm5 \n");
asm ("movsd -0x70(%rsp), %xmm6 \n");
asm ("movsd -0x80(%rsp), %xmm7 \n");

// asm ("vmcall \n");
asm ("popf \n");
asm ("popq %r15 \n");
asm ("popq %r14 \n");
asm ("popq %r13 \n");
asm ("popq %r12 \n");
asm ("popq %rbp \n");
asm ("popq %rbx \n");
asm ("popq %r11 \n");
// asm ("popq %r10 \n");
asm ("popq %r9 \n");
asm ("popq %r8 \n");
asm ("popq %r10 \n");
asm ("popq %rdx \n");
asm ("popq %rsi \n");
asm ("popq %rdi \n");
asm ("popq %rax \n");//restore target rsp into rax
asm ("movq %rax, %rsp \n");
// asm ("movq $0xfffffe9000905fb0, %rax \n");//addr of entry_gate
// asm ("movq $0x7f9000905fb0, %rax \n");//addr of entry_gate
asm ("movq $0x7f1000905fb0, %rax \n");//addr of entry_gate
asm ("jmpq *(%rax) \n");
// asm ("movq $0x, %rax \n"); //addr of entry_gate
// asm ("jmpq *%rax \n");
// asm ("pushq $0x2b \n");//pt_regs->ss
// asm ("pushq %rax \n");//rsp stored in board_ctx
// asm ("pushq %r11 \n");//pt_regs->rflags
// asm ("pushq $0x33 \n");//pt_regs->cs
// asm ("movq $0xfffffe9000905fb0, %rax \n");//the address of board_ctx
// asm ("pushq (%rax) \n");//addr of entry_gate
// // asm ("pushq 0x28(%rax) \n");//syscall #; pt_regs->orig_rax, rax stored in board_ctx
// asm ("iretq \n");

static void read_dr (void)
{
    unsigned long dr0, dr1, dr2, dr3, dr7;
    asm volatile ("movq %%DR0, %%rax; \n\t"
            "movq %%rax, %0; \n\t"
            "movq %%DR1, %%rax; \n\t"
            "movq %%rax, %1; \n\t"
            "movq %%DR2, %%rax; \n\t"
            "movq %%rax, %2; \n\t"
            "movq %%DR3, %%rax; \n\t"
            "movq %%rax, %3; \n\t"
            "movq %%DR7, %%rax; \n\t"
            "movq %%rax, %4; \n\t"
            :"=m"(dr0),"=m"(dr1),"=m"(dr2),"=m"(dr3),"=m"(dr7)::"%rax");
    printf ("dr0: %lx, dr1: %lx, dr2: %lx, dr3: %lx, dr7: %lx. \n", dr0, dr1, dr2, dr3, dr7);
    return;
}

/* no need to clear dr0-dr3, disable through dr7 */
static void clear_dr(int idx)
{
    int dr7;
    switch (idx)
    {
        case 0:
            dr7 = 0xfff0fffc;
            break;
        case 1:
            dr7 = 0xff0ffff3;
            break;
        case 2: 
            dr7 = 0xf0ffffcf;
            break;
        case 3: 
            dr7 = 0x0fffff3f;
            break;
        default: 
            asm volatile ("mov $0xabcdabcd, %rax; \n\t"
                    "vmcall; \n\t");
            break;
    }

    asm volatile (
            // "movq $0x0, %%rax; \n\t"
            // "movq %%rax, %%DR0; \n\t"
            "mov %0, %%ebx; \n\t"
            "mov %%DR7, %%eax; \n\t"
            "and %%ebx, %%eax; \n\t"
            "mov %%eax, %%DR7; \n\t"
            ::"m"(dr7):"%eax","%ebx");
    return;
}
// static void clear_dr1(void)
// {
//     asm volatile (
//             "movq $0x0, %%rax; \n\t"
//             "movq %%rax, %%DR0; \n\t"
//             "mov %%DR7, %%eax; \n\t"
//             "and $0xfff0fffe, %%eax; \n\t"
//             "mov %%eax, %%DR7; \n\t"
//             :::"%rax");
//     return;
// }

static __attribute__ ((noinline)) void set_dr3(int size, unsigned long addr)
{
    int dr7;
    /* bit 26-27 control the size to monitor */
    switch (size)
    {
        case 1:
            dr7 = 0;
            break;
        case 2: 
            dr7 = 0x40000000; 
            break;
        case 4: 
            dr7 = 0xc0000000;
            break;
        case 8:
            dr7 = 0x80000000;
            break;
        default: 
            asm volatile ("mov $0xabcdabcd, %rax; \n\t"
                    "vmcall; \n\t");
            break;
    }
    /* bit 6 enables dr1, bit 28-29 control operations to monitor (both rw) */
    dr7 |= 0x30000040;
            
    asm volatile (
            "movq %0, %%rax; \n\t"
            "movq %%rax, %%DR3; \n\t"
            "movq %1, %%rbx; \n\t"
            "mov %%DR7, %%eax; \n\t"
            "or %%ebx, %%eax; \n\t"
            "mov %%eax, %%DR7; \n\t"
            ::"m"(addr), "m"(dr7):"%rax", "%rbx");
    return;
}

static __attribute__ ((noinline)) void set_dr2(int size, unsigned long addr)
{
    int dr7;
    /* bit 26-27 control the size to monitor */
    switch (size)
    {
        case 1:
            dr7 = 0;
            break;
        case 2: 
            dr7 = 0x4000000; 
            break;
        case 4: 
            dr7 = 0xc000000;
            break;
        case 8:
            dr7 = 0x8000000;
            break;
        default: 
            asm volatile ("mov $0xabcdabcd, %rax; \n\t"
                    "vmcall; \n\t");
            break;
    }
    /* bit 4 enables dr1, bit 24-25 control operations to monitor (both rw) */
    dr7 |= 0x3000010;
            
    asm volatile (
            "movq %0, %%rax; \n\t"
            "movq %%rax, %%DR2; \n\t"
            "movq %1, %%rbx; \n\t"
            "mov %%DR7, %%eax; \n\t"
            "or %%ebx, %%eax; \n\t"
            "mov %%eax, %%DR7; \n\t"
            ::"m"(addr), "m"(dr7):"%rax", "%rbx");
    return;
}

static __attribute__ ((noinline)) void set_dr1(int size, unsigned long addr)
{
    int dr7;
    /* bit 22-23 control the size to monitor */
    switch (size)
    {
        case 1:
            dr7 = 0;
            break;
        case 2: 
            dr7 = 0x400000; 
            break;
        case 4: 
            dr7 = 0xc00000;
            break;
        case 8:
            dr7 = 0x800000;
            break;
        default: 
            asm volatile ("mov $0xabcdabcd, %rax; \n\t"
                    "vmcall; \n\t");
            break;
    }
    /* bit 2 enables dr1, bit 20-21 control operations to monitor (both rw) */
    dr7 |= 0x300004;
            
    asm volatile (
            "movq %0, %%rax; \n\t"
            "movq %%rax, %%DR1; \n\t"
            "movq %1, %%rbx; \n\t"
            "mov %%DR7, %%eax; \n\t"
            "or %%ebx, %%eax; \n\t"
            "mov %%eax, %%DR7; \n\t"
            ::"m"(addr), "m"(dr7):"%rax", "%rbx");
    return;
}

static __attribute__ ((noinline)) void set_dr0(int size, unsigned long addr)
{
    int dr7;
    /* bit 18-19 control the size to monitor */
    switch (size)
    {
        case 1:
            dr7 = 0;
            break;
        case 2: 
            dr7 = 0x40000; 
            break;
        case 4: 
            dr7 = 0xc0000;
            break;
        case 8:
            dr7 = 0x80000;
            break;
        default: 
            asm volatile ("mov $0xabcdabcd, %rax; \n\t"
                    "vmcall; \n\t");
            break;
    }
    /* bit 0 enables dr0, bit 16-17 control operations to monitor (both rw) */
    dr7 |= 0x30001;
            
    asm volatile (
            "movq %0, %%rax; \n\t"
            "movq %%rax, %%DR0; \n\t"
            "movq %1, %%rbx; \n\t"
            "mov %%DR7, %%eax; \n\t"
            "or %%ebx, %%eax; \n\t"
            "mov %%eax, %%DR7; \n\t"
            ::"m"(addr), "m"(dr7):"%rax", "%rbx");
    // printk ("now dr0: %lx, dr7: %lx\n", dr0, dr7);
    return;
}

extern "C" void db_handler (void);
void db_handler(void)
{
    asm volatile("movq $0x99999, %%rax; \n\t"
            "vmcall; \n\t"
            :::"%rax");
    
    // oasis engine to save T and load SE context
    native_to_SE_ctx_switch();
    execState->SynRegsFromNative(&machRegs);
   
    execState->DBHandler();
   
    execState->SynRegsToNative(&machRegs);
    SE_to_native_ctx_switch();

    // oasis engine to resume T
    //
    
    // clear_dr(0);
    // printf ("in DB handler. \n");
    // asm volatile ("mov $0x99999999, %rax; \n\t"
    //         "vmcall; \n\t");
    // // SymExecutor();
    // // update_watchpoint();
    return;
}

extern "C" void db_store_context (void);
void db_store_context (void);
asm (" .text");
asm (" .type    db_store_context, @function");
asm ("db_store_context: \n");
asm ("movq $0xabcdabcd, %rax \n");
asm ("vmcall \n");
asm ("movq %rsp, %rax \n"); 
asm ("movq $0x7f7fffffecc0, %rsp \n");//switch to analyser's secure stack
asm ("pushq %rax \n");// save target rsp in nme stack
asm ("pushq %rdi \n");// 6 syscall args
asm ("pushq %rsi \n");
asm ("pushq %rdx \n");
asm ("pushq %r10 \n");
asm ("pushq %r8 \n");
asm ("pushq %r9 \n");
asm ("pushq %r11 \n");
asm ("pushq %rbx \n");//the rest of user context
asm ("pushq %rbp \n");
asm ("pushq %r12 \n");
asm ("pushq %r13 \n");
asm ("pushq %r14 \n");
asm ("pushq %r15 \n");
asm ("pushf \n");

asm ("movsd %xmm0, -0x10(%rsp) \n");
asm ("movsd %xmm1, -0x20(%rsp) \n");
asm ("movsd %xmm2, -0x30(%rsp) \n");
asm ("movsd %xmm3, -0x40(%rsp) \n");
asm ("movsd %xmm4, -0x50(%rsp) \n");
asm ("movsd %xmm5, -0x60(%rsp) \n");
asm ("movsd %xmm6, -0x70(%rsp) \n");
asm ("movsd %xmm7, -0x80(%rsp) \n");
// asm ("sub $0x90, %rsp \n");
asm ("sub $0x98, %rsp \n");//To ensure the stack 16-byte aligned
// asm ("vmcall \n");

asm ("callq db_handler \n");

asm ("add $0x98, %rsp \n");
// asm ("add $0x90, %rsp \n");
asm ("movsd -0x10(%rsp), %xmm0 \n");
asm ("movsd -0x20(%rsp), %xmm1 \n");
asm ("movsd -0x30(%rsp), %xmm2 \n");
asm ("movsd -0x40(%rsp), %xmm3 \n");
asm ("movsd -0x50(%rsp), %xmm4 \n");
asm ("movsd -0x60(%rsp), %xmm5 \n");
asm ("movsd -0x70(%rsp), %xmm6 \n");
asm ("movsd -0x80(%rsp), %xmm7 \n");

// asm ("vmcall \n");
asm ("popf \n");
asm ("popq %r15 \n");
asm ("popq %r14 \n");
asm ("popq %r13 \n");
asm ("popq %r12 \n");
asm ("popq %rbp \n");
asm ("popq %rbx \n");
asm ("popq %r11 \n");
asm ("popq %r9 \n");
asm ("popq %r8 \n");
asm ("popq %r10 \n");
asm ("popq %rdx \n");
asm ("popq %rsi \n");
asm ("popq %rdi \n");
asm ("popq %rax \n");//restore target rsp into rax
asm ("movq %rax, %rsp \n");
asm ("movq $0x7f1000905fb0, %rax \n");//addr of entry_gate
asm ("jmpq *(%rax) \n");

/* perm: 0 -- recover RW bits; 1 -- clear RW bits */
void update_t_ept_perm (int perm, unsigned long ker_addr)
{
    unsigned long gpa;
    ker_addr &= ~0xFFFUL;
    if (ker_addr < __START_KERNEL_MAP)
        gpa = ker_addr - PAGE_OFFSET;
    else
        gpa = ker_addr - __START_KERNEL_MAP;
    printf ("intercept VE page, va: %lx, gpa: %lx. \n", ker_addr, gpa);

    asm volatile (
            "movq %0, %%rdx; \n\t"
            "movq %1, %%rcx; \n\t"
            "movq %2, %%rbx; \n\t"
            "movq $0xdcba, %%rax; \n\t"
            "vmcall; \n\t"
            // "lea 0x2(%%rip), %%rdx; \n\t"
            // "jmpq *%%rax; \n\t"
            ::"m"(ker_addr), "m"(gpa), "m"(perm):"%rax","%rbx","%rcx","%rdx");
    return;
}

int determ_sym_mem(unsigned long addr)
{
    if (addr == 0)//belong to sym mem
        return 1;
    else
        return 0;
}

extern "C" void ve_handler (void);
void ve_handler(void)
{
    asm volatile("movq $0x99999, %%rax; \n\t"
            "vmcall; \n\t"
            :::"%rax");

    // oasis engine to save T and load SE context
    native_to_SE_ctx_switch();
    execState->SynRegsFromNative(&machRegs);
    
    // execState->VEHandler();
    
    execState->SynRegsToNative(&machRegs);
    SE_to_native_ctx_switch();

    // oasis engine to resume T
    //
    
    // unsigned long* virt_exec_area;
    // unsigned long exit_qual = virt_exce_area[1];
    // if ((exit_qual & 0x4UL) != 0)
    // {
    //     printf ("unexpected EPT violation . \n");
    //     asm volatile("movq $0x99999, %%rax; \n\t"
    //             "vmcall; \n\t"
    //             :::"%rax");
    // }
    // else
    // {
    //     unsigned long va = virt_exce_area[2];
    //     int ret = determ_sym_mem(va);
    //     virt_exce_area[0] = 0x0UL;
    //     crtAddr = va;
    //     if (ret == 0)
    //     {
    //         InsnDispatch();
    //     }
    //     else
    //     {
    //         printf ("invoke symExecutor. \n");
    //         asm volatile ("mov $0x99999999, %rax; \n\t"
    //                 "vmcall; \n\t");
    //         // SymExecutor();
    //         // update_watchpoint();
    //     }
    // }
    return;
}

extern "C" void ve_store_context (void);
void ve_store_context (void);
asm (" .text");
asm (" .type    ve_store_context, @function");
asm ("ve_store_context: \n");
asm ("movq $0xabcdabcd, %rax \n");
asm ("vmcall \n");
asm ("movq %rsp, %rax \n"); 
asm ("movq $0x7f7fffffecc0, %rsp \n");//switch to analyser's secure stack
asm ("pushq %rax \n");// save target rsp in nme stack
asm ("pushq %rdi \n");// 6 syscall args
asm ("pushq %rsi \n");
asm ("pushq %rdx \n");
asm ("pushq %r10 \n");
asm ("pushq %r8 \n");
asm ("pushq %r9 \n");
asm ("pushq %r11 \n");
asm ("pushq %rbx \n");//the rest of user context
asm ("pushq %rbp \n");
asm ("pushq %r12 \n");
asm ("pushq %r13 \n");
asm ("pushq %r14 \n");
asm ("pushq %r15 \n");
asm ("pushf \n");

asm ("movsd %xmm0, -0x10(%rsp) \n");
asm ("movsd %xmm1, -0x20(%rsp) \n");
asm ("movsd %xmm2, -0x30(%rsp) \n");
asm ("movsd %xmm3, -0x40(%rsp) \n");
asm ("movsd %xmm4, -0x50(%rsp) \n");
asm ("movsd %xmm5, -0x60(%rsp) \n");
asm ("movsd %xmm6, -0x70(%rsp) \n");
asm ("movsd %xmm7, -0x80(%rsp) \n");
// asm ("sub $0x90, %rsp \n");
asm ("sub $0x98, %rsp \n");//To ensure the stack 16-byte aligned
// asm ("vmcall \n");

asm ("callq ve_handler \n");

asm ("add $0x98, %rsp \n");
// asm ("add $0x90, %rsp \n");
asm ("movsd -0x10(%rsp), %xmm0 \n");
asm ("movsd -0x20(%rsp), %xmm1 \n");
asm ("movsd -0x30(%rsp), %xmm2 \n");
asm ("movsd -0x40(%rsp), %xmm3 \n");
asm ("movsd -0x50(%rsp), %xmm4 \n");
asm ("movsd -0x60(%rsp), %xmm5 \n");
asm ("movsd -0x70(%rsp), %xmm6 \n");
asm ("movsd -0x80(%rsp), %xmm7 \n");

// asm ("vmcall \n");
asm ("popf \n");
asm ("popq %r15 \n");
asm ("popq %r14 \n");
asm ("popq %r13 \n");
asm ("popq %r12 \n");
asm ("popq %rbp \n");
asm ("popq %rbx \n");
asm ("popq %r11 \n");
asm ("popq %r9 \n");
asm ("popq %r8 \n");
asm ("popq %r10 \n");
asm ("popq %rdx \n");
asm ("popq %rsi \n");
asm ("popq %rdi \n");
asm ("popq %rax \n");//restore target rsp into rax
asm ("movq %rax, %rsp \n");
asm ("movq $0x7f1000905fb0, %rax \n");//addr of entry_gate
asm ("jmpq *(%rax) \n");
/* /========================================================*/

// void init_global_var (int shar_mem_fd)
void init_global_var ()
{
    void* temp;
    // per_hook[0] = 0xcc;
    // crt_int3_idx = 0;
    // // /* initialize redirect_page_pool */
    // // page_pool = (POOL*) malloc (sizeof(page_pool));
    // // pool_create (0x1000*max_redirect_idx);
    
    // /* empty the memory where stores the global data structures */
    // temp = &redirected_pages[0];
    // memset(temp, 0x0, sizeof(redirected_pages));
    // temp = &new_pages[0];
    // memset(temp, 0x0, sizeof(new_pages));
    // temp = &offsets[0];
    // memset(temp, 0x0, sizeof(offsets));
    // // temp = &bb_recording[0][0];
    // // memset(temp, 0x0, sizeof(bb_recording));
    // crt_redir_idx = crt_max_redir_idx = 0;
    
    // uk_offset = 0xffff7f0000000000;
    // uk_offset = 0x0;
    uk_offset = 0xffffff8000000000;
    exit_gate_va = 0x7f9000900000+uk_offset;
    idt_va = 0x7f9000901000 + uk_offset;
    gdt_va = 0x7f9000902000 + uk_offset;
    tss_va = 0x7f9000903000 + uk_offset;
    data_page = 0x7f9000905000 + uk_offset;//a writable data page
    // // t_pf_stack = data_page + 0x1000 - 0x100;//The higher 0x100 is used to store board_ctx. 
    t_int3_stack = data_page + 0x1000 - 0x200;//#INT3 uses part of oasis_lib's data page as its stack
    // t_ve_stack = t_int3_stack;//#VE shares the same stack as #INT3 
    // t_db_stack = t_int3_stack;//#DB sahres the same stack as #INT3 
    // t_pf_stack = data_page + 0x1000 - 0x100;//The higher 0x100 is used to store board_ctx. 
    execState->m_emeta->t_int3_stack = data_page + 0x1000 - 0x200;//#INT3 uses part of oasis_lib's data page as its stack
    execState->m_emeta->t_ve_stack = t_int3_stack;//#VE shares the same stack as #INT3 
    execState->m_emeta->t_db_stack = t_int3_stack;//#DB sahres the same stack as #INT3 
    
    root_pt_va = 0x7f9000906000 + uk_offset;
    ei_shar_args = (struct shar_arg*)(0x7f90000907000 + uk_offset);
    // ana_t_tss = 0xfffffef020908000 + shar_args->tss_pg_off;//0x200 is guest_tss_page_offset
    ana_t_tss = 0x7f9000908000 + uk_offset;//0x200 is guest_tss_page_offset
    ana_t_gdt = 0x7f9000909000 + uk_offset;
    ana_t_idt = 0x7f900090a000 + uk_offset;
    // virt_exce_area = (unsigned long*)(0x7f900090c000 + uk_offset);
    execState->m_emeta->virt_exce_area = (unsigned long*)(0x7f900090c000 + uk_offset);
    // int i;
    // for (i = 0; i < 5; i ++)
    // {
    //     printf ("i: %d, virt_exec_area: %p, content: %lx. \n", i, virt_exec_area, *((unsigned long*)(virt_exec_area+0x8*i)));
    //     // printf ("i: %d, gdt entry: %lx. \n", i, gdt_base[i]);
    // }
    // pf_stack = 0xfffffef02090e000; 

    // /* copy TSS_STRUCT from Guest VM. Let T_PF use its orginal stack */
    // // memcpy((void*)ana_t_tss, (void*)(ei_shar_args->tss_base), 0x68);
    // // *((unsigned long*)(ana_t_tss+0x4)) = t_pf_stack;//setup t_pf_stack in t_tss structure
    // *((unsigned long*)(ana_t_tss+0x4 + 0x8*10)) = int3_stack;//setup t_int3_stack in t_tss structure, ist[7]

    // klee_shar_mem = 0x7f900090b000 + uk_offset;
    // klee_shar_mem = 0x7f900090c000 + uk_offset;
    ana_stack = 0x7fffffffecc0 + uk_offset;
    entry_gate = exit_gate_va + 0x261;
    exit_gate = exit_gate_va + 0x292;
    syscall_exit_gate = exit_gate_va + 0x2fd;

    target_ctx = (struct target_context*)(ana_stack - 0x78);
    // board_ctx = (struct board_context*)(data_page + 0xfa8);
    board_ctx = (struct board_context*)(data_page + 0xf98);
    board_ctx->syscall_handler = (unsigned long)syscall_store_context;
    board_ctx->syscall_exit_handler = (unsigned long)syscall_exit_store_context;
    // board_ctx->ret_handler = (unsigned long)nme_ret_handler;
    board_ctx->t_int3_handler = (unsigned long)int3_store_context;
    board_ctx->t_ve_handler = (unsigned long)ve_store_context;
    board_ctx->t_db_handler = (unsigned long)db_store_context;
    board_ctx->pf_handler = (unsigned long)pf_store_context;
    board_ctx->entry_gate = entry_gate;

    nme_fsbase = read_fs();
    printf ("nme_fsbase: %lx. \n", nme_fsbase);

    unsigned long* tmp_ptr = (unsigned long*)0x555555554760;
    printf ("test: %p. %lx. \n", tmp_ptr, *tmp_ptr);
    
    /* initialize addr_gdt_base, addr_tss_base */
    unsigned char gdtr[10];
    unsigned long tss_base0, tss_base1, tss_base2;
    asm ("sgdt %0; \n\t"
            :"=m"(gdtr)
            :
            :);
    gdt_base = (unsigned long*)(*(unsigned long*)(gdtr + 2));
    printf ("gdt base: %lx. \n", (unsigned long)gdt_base);
   
    init_call_gate();
    
    // /* Make the three ConcretExecutor pages writable */
    // int ret;
    // void* execPage = (void*)(((unsigned long)InsnExecNonRIP) & ~0xFFF);
    // ret = mprotect(execPage, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
    // execPage = (void*)(((unsigned long)InsnExecRIP) & ~0xFFF);
    // ret = mprotect(execPage, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
    // execPage = (void*)(((unsigned long)BlockExecRIP) & ~0xFFF);
    // ret = mprotect(execPage, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
    // printf ("ret of mprotect: %d. \n", ret);
    // 
    // /* Init a RIP-relative Jmp Insn which is the last Insn of Block T-Insn Executor */
    // auto init = std::initializer_list<unsigned char>({0xff, 0x25, 0x00, 0x00, 0x00, 0x00});
    // std::copy(init.begin(), init.end(), Jmp_RIP_Insn);
    // T_page = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE | MAP_POPULATE, -1, 0);
    // if (T_page == MAP_FAILED)
    // {
    //     printf ("Init T_page failed: %lx. \n", T_page);
    //     asm volatile ("movq $0xffff, %rax; \n\t"
    //             "vmcall; \n\t");
    // }
    // /* Init the addr of T_page on BlockExec */
    // unsigned long* BlkExec_T_page = (unsigned long*)(BlockExecRIP + 0x74);
    // *BlkExec_T_page = (unsigned long)T_page;
    // /* / */

    // unsigned long tmp = gdt_base[8];
    // tmp |= 0x20000000000;
    // gdt_base[8] = tmp;
    // int i;
    // for (i = 0; i < 20; i ++)
    // {
    //     printf ("i: %d, gdt entry: %lx. \n", i, gdt_base[i]);
    // }
    // asm volatile ("movq $0xffff, %rax; \n\t"
    //         "vmcall; \n\t");
    
    /* check root pt entry */
    unsigned long test_addr = 0x555555554700;
    int idx = test_addr >> 39;
    unsigned long* root_pt_ptr = (unsigned long*)root_pt_va;
    printf ("idx: %d, entry: %lx. \n", idx, root_pt_ptr[idx]);
    root_pt_ptr[idx] &= 0xFFFFFFFFF;
    // test_addr = 0x7fffffffa000;
    // idx = test_addr >> 39;
    // printf ("idx: %d, entry: %lx. \n", idx, root_pt_ptr[idx]);
    // // root_pt_ptr[idx] &= 0xFFFFFFFFF;

    // int i = 0;
    // for (i = 0; i < 512; i++)
    // {
    //     if(root_pt_ptr[i])
    //     {
    //         printf ("i: %d, entry: %lx. \n", i, root_pt_ptr[i]);
    //     }
    // }
    
    // asm volatile ("movq $0xffff, %rax; \n\t"
    //         "vmcall; \n\t");

    return;
}

void init_t_ctx()
{
    machRegs.regs.r8 = ei_shar_args->r8; 
    machRegs.regs.r9 = ei_shar_args->r9;
    machRegs.regs.r10 = ei_shar_args->r10; 
    machRegs.regs.r11 = ei_shar_args->r11;
    machRegs.regs.r12 = ei_shar_args->r12;
    machRegs.regs.r13 = ei_shar_args->r13;
    machRegs.regs.r14 = ei_shar_args->r14;
    machRegs.regs.r15 = ei_shar_args->r15;
    machRegs.regs.rax = ei_shar_args->rax;
    machRegs.regs.rbx = ei_shar_args->rbx;
    machRegs.regs.rcx = ei_shar_args->rcx;
    machRegs.regs.rdx = ei_shar_args->rdx;
    machRegs.regs.rsi = ei_shar_args->rsi;
    machRegs.regs.rdi = ei_shar_args->rdi;
    machRegs.regs.rbp = ei_shar_args->rbp;
    machRegs.regs.rsp = ei_shar_args->rsp;
    machRegs.regs.rip = ei_shar_args->rip;
    machRegs.regs.eflags = ei_shar_args->eflags;
    assert(ei_shar_args->msr_kernel_gs_base != 0);
    assert(ei_shar_args->fs_base != 0);
    machRegs.fs_base = ei_shar_args->fs_base;
    machRegs.gs_base = ei_shar_args->msr_kernel_gs_base;
    return;
}

void native_to_SE_ctx_switch()
{
    // board_ctx->rip;
    // target_ctx->rax = board_ctx->rax;
    // target_ctx->rcx = board_ctx->rcx;
    machRegs.regs.r8 = target_ctx->r8; 
    machRegs.regs.r9 = target_ctx->r9;
    machRegs.regs.r10 = target_ctx->r10; 
    machRegs.regs.r11 = target_ctx->r11;
    machRegs.regs.r12 = target_ctx->r12;
    machRegs.regs.r13 = target_ctx->r13;
    machRegs.regs.r14 = target_ctx->r14;
    machRegs.regs.r15 = target_ctx->r15;
    machRegs.regs.rax = target_ctx->rax;
    machRegs.regs.rbx = target_ctx->rbx;
    machRegs.regs.rcx = target_ctx->rcx;
    machRegs.regs.rdx = target_ctx->rdx;
    machRegs.regs.rsi = target_ctx->rsi;
    machRegs.regs.rdi = target_ctx->rdi;
    machRegs.regs.rbp = target_ctx->rbp;
    machRegs.regs.rsp = target_ctx->rsp;
    machRegs.regs.rip = target_ctx->rip;
    machRegs.regs.eflags = target_ctx->eflags;
    assert(ei_shar_args->msr_kernel_gs_base != 0);
    assert(ei_shar_args->fs_base != 0);
    machRegs.fs_base = ei_shar_args->fs_base;
    machRegs.gs_base = ei_shar_args->msr_kernel_gs_base;
    return;
}

void SE_to_native_ctx_switch()
{
     target_ctx->r8 = machRegs.regs.r8; 
     target_ctx->r9 = machRegs.regs.r9;
    target_ctx->r10 = machRegs.regs.r10; 
    target_ctx->r11 = machRegs.regs.r11;
    target_ctx->r12 = machRegs.regs.r12;
    target_ctx->r13 = machRegs.regs.r13;
    target_ctx->r14 = machRegs.regs.r14;
    target_ctx->r15 = machRegs.regs.r15;
    target_ctx->rax = machRegs.regs.rax;
    target_ctx->rbx = machRegs.regs.rbx;
    target_ctx->rcx = machRegs.regs.rcx;
    target_ctx->rdx = machRegs.regs.rdx;
    target_ctx->rsi = machRegs.regs.rsi;
    target_ctx->rdi = machRegs.regs.rdi;
    target_ctx->rbp = machRegs.regs.rbp;
    target_ctx->rsp = machRegs.regs.rsp;
    target_ctx->rip = machRegs.regs.rip;
    target_ctx->eflags = machRegs.regs.eflags;
    return;
}
// void ana_native_ctx_switch()
// {
// 
// }

// void to_native(void)
// {
//     /* update board_ctx based on target vcpu context */
//     board_ctx->rip = ei_shar_args->rip;
//     board_ctx->rax = ei_shar_args->rax;
//     board_ctx->rcx = ei_shar_args->rcx;
//     // t_fsbase = ei_shar_args->fs_base;
//     int index = 0xc0000102;
//     wrmsr(index, ei_shar_args->msr_kernel_gs_base);
// 
//     write_fs(ei_shar_args->fs_base);
//     write_gs(ei_shar_args->gs_base);
//     
//     // wr_cr2(ei_shar_args->cr2);
//     restore_user_privilege ();
//     // printf ("...............\n");
//     // // int msr_lstar_idx = 0xc00000082;
//     // rdmsr(msr_lstar_idx);
//     // asm volatile ("movq $0xffff, %rax; \n\t"
//     //         "vmcall; \n\t");
// 
// 
//     /* transfer to trampoline */
//     asm volatile (
//             // /* prepare stack for iret */
//             "movq %0, %%rbx; \n\t"
//             "movq %1, %%rax; \n\t"//f_trampoline
//             "movq 0x50(%%rbx), %%rcx; \n\t"//eflags
//             "pushq %%rcx; \n\t"
//             "popfq; \n\t"
//             // "pushq %%rax; \n\t"
// 
//             /* load all registers */
//             "movq 0x8(%%rbx), %%rdi; \n\t"
//             "movq 0x10(%%rbx), %%rsi; \n\t"
//             "movq 0x18(%%rbx), %%rdx; \n\t"
//             "movq 0x28(%%rbx), %%r8; \n\t"
//             "movq 0x30(%%rbx), %%r9; \n\t"
//             "movq 0x38(%%rbx), %%r11; \n\t"
//             "movq 0x40(%%rbx), %%r10; \n\t"
//             "movq 0x70(%%rbx), %%rbp; \n\t"
//             "movq 0x78(%%rbx), %%r12; \n\t"
//             "movq 0x80(%%rbx), %%r13; \n\t"
//             "movq 0x88(%%rbx), %%r14; \n\t"
//             "movq 0x90(%%rbx), %%r15; \n\t"
//             "movq 0x60(%%rbx), %%rsp; \n\t"
//             "movq 0x68(%%rbx), %%rbx; \n\t"
//             "jmpq *%%rax; \n\t"
//             // "movq $0x0, %%rax; \n\t"
//             // "movq $0x1, %%rcx; \n\t"
// 
//             // "retq; \n\t"
// 
//             ::"m"(ei_shar_args),"m"(entry_gate):"%rcx","%rax", "%rdx", "%rbx", "%rdi", "%rsi");
// }

void get_target()
{
    printf ("ei_shar_args at: %p \n", ei_shar_args);
    
    ei_shar_args = (struct shar_arg*)(0x7f9000907000 + uk_offset);
    /* copy TSS_STRUCT from Guest VM. Let T_PF use its orginal stack */
    memcpy((void*)ana_t_tss, (void*)(ei_shar_args->tss_base), 0x68);
    // *((unsigned long*)(ana_t_tss+0x4)) = t_pf_stack;//setup t_pf_stack in t_tss structure
    // t_pf_stack = *((unsigned long*)(ana_t_tss+0x4));//#PF uses original stack as in the guest VM
    // execState->m_FattCtrl->m_emeta->t_pf_stack = *((unsigned long*)(ana_t_tss+0x4));//#PF uses original stack as in the guest VM
    execState->m_emeta->t_pf_stack = *((unsigned long*)(ana_t_tss+0x4));//#PF uses original stack as in the guest VM
    *((unsigned long*)(ana_t_tss+0x4 + 0x8*10)) = t_int3_stack;//setup t_int3_stack in t_tss structure, ist[7]
    unsigned long t_rsp0 = *((unsigned long*)(ana_t_tss+0x4));
    unsigned long t_rsp1 = *((unsigned long*)(ana_t_tss+0xc));
    printf ("t_rsp0 stack: %lx. \n", t_rsp0);
    printf ("t_rsp1 stack: %lx. \n", t_rsp1);
    ei_shar_args->flag = 1;
    
    do {
        asm volatile("mfence; \n\t");
    } while (ei_shar_args->flag != 2);

    printf ("onsite receive request. \n");
    printf ("ei_shar_args->rip: %lx, rsp: %lx. \n", ei_shar_args->rip, ei_shar_args->rsp);
    ei_shar_args->rip += 3;

    printf ("ei_shar_args at: %p \n", ei_shar_args);
    
    unsigned long* tt = (unsigned long*)(ei_shar_args->rip);
    printf ("target rip: %p, target code: %lx. \n", tt, *tt);
    tt = (unsigned long*)(ei_shar_args->g_syscall_entry);
    printf ("syscall entry at: %p, code: %lx. \n", tt, *tt);
    
    // asm volatile ("movq $0xffff, %rax; \n\t"
    //         "vmcall; \n\t");
   
    /* To syn TSC_OFFSET through hyp */
    asm volatile ("movq $0x378, %rax; \n\t"
            "vmcall; \n\t");
    return;
}

void dump_regs()
{
    printf ("rax: %lx\n", machRegs.regs.rax);
    printf ("rbx: %lx ", machRegs.regs.rbx);
    printf ("rcx: %lx ", machRegs.regs.rcx);
    printf ("rdx: %lx ", machRegs.regs.rdx);
    printf ("rdi: %lx ", machRegs.regs.rdi);
    printf ("rsi: %lx ", machRegs.regs.rsi);
    printf ("r8: %lx ", machRegs.regs.r8);
    printf ("r9: %lx ", machRegs.regs.r9);
    printf ("r10: %lx \n", machRegs.regs.r10);
    printf ("rip: %lx \n", machRegs.regs.rip);
    printf ("rsp: %lx \n", machRegs.regs.rsp);
    return;
}

void to_native(void)
{
    /* update board_ctx based on target vcpu context */
    board_ctx->rip = ei_shar_args->rip;
    board_ctx->rax = ei_shar_args->rax;
    board_ctx->rcx = ei_shar_args->rcx;
    // t_fsbase = ei_shar_args->fs_base;
    int index = 0xc0000102;
    wrmsr(index, ei_shar_args->msr_kernel_gs_base);

    write_fs(ei_shar_args->fs_base);
    write_gs(ei_shar_args->gs_base);
    
    // wr_cr2(ei_shar_args->cr2);
    restore_user_privilege ();
    // printf ("...............\n");
    // // int msr_lstar_idx = 0xc00000082;
    // rdmsr(msr_lstar_idx);
    // asm volatile ("movq $0xffff, %rax; \n\t"
    //         "vmcall; \n\t");


    /* transfer to trampoline */
    asm volatile (
            // /* prepare stack for iret */
            "movq %0, %%rbx; \n\t"
            "movq %1, %%rax; \n\t"//f_trampoline
            "movq 0x50(%%rbx), %%rcx; \n\t"//eflags
            "pushq %%rcx; \n\t"
            "popfq; \n\t"
            // "pushq %%rax; \n\t"

            /* load all registers */
            "movq 0x8(%%rbx), %%rdi; \n\t"
            "movq 0x10(%%rbx), %%rsi; \n\t"
            "movq 0x18(%%rbx), %%rdx; \n\t"
            "movq 0x28(%%rbx), %%r8; \n\t"
            "movq 0x30(%%rbx), %%r9; \n\t"
            "movq 0x38(%%rbx), %%r11; \n\t"
            "movq 0x40(%%rbx), %%r10; \n\t"
            "movq 0x70(%%rbx), %%rbp; \n\t"
            "movq 0x78(%%rbx), %%r12; \n\t"
            "movq 0x80(%%rbx), %%r13; \n\t"
            "movq 0x88(%%rbx), %%r14; \n\t"
            "movq 0x90(%%rbx), %%r15; \n\t"
            "movq 0x60(%%rbx), %%rsp; \n\t"
            "movq 0x68(%%rbx), %%rbx; \n\t"
            "jmpq *%%rax; \n\t"
            // "movq $0x0, %%rax; \n\t"
            // "movq $0x1, %%rcx; \n\t"

            // "retq; \n\t"

            ::"m"(ei_shar_args),"m"(entry_gate):"%rcx","%rax", "%rdx", "%rbx", "%rdi", "%rsi");
    
    asm volatile ("movq $0xffff, %rax; \n\t"
            "vmcall; \n\t");

}

int main(void) {
    printf("start ana : krover\n");

    unsigned long ts, te;
    ts = rdtsc();
    rdtsc();
    te = rdtsc();
    printf("cost %lu\n", te- ts);

    unsigned long adds, adde;
    adds = 0x0;
    adde = 0xfffffffffffff000;
    
    execState = new ExecState(adds, adde);
    init_global_var();
    dump_regs();
    get_target();
    execState->InitRediPagePool();

    execState->MoniStartOfSE(0xffffffff810b9710);//addr of x64_sys_setpriority
    //execState->MoniStartOfSE(0xffffffff810b5fb0);//addr of x64_sys_getpriority
    //execState->MoniStartOfSE(0xffffffff812de980);//addr of x64_sys_writev
    //execState->MoniStartOfSE(0xffffffff812ddc30);//addr of x64_sys_lseek
    //execState->MoniStartOfSE(0xffffffff81910660);//addr of x64_sys_bind
    //execState->MoniStartOfSE(0xffffffff81910250);//addr of x64_sys_socket
    //execState->MoniStartOfSE(0xffffffff812db390);//addr of x64_sys_access
    //execState->MoniStartOfSE(0xffffffff812ea7f0);//addr of x64_sys_pipe
    //execState->MoniStartOfSE(0xffffffff81303310);//addr of x64_sys_dup
    //execState->MoniStartOfSE(0xffffffff813023e0);//addr of x64_sys_dup2
    //execState->MoniStartOfSE(0xffffffff81144110);//addr of x64_sys_alarm
    //execState->MoniStartOfSE(0xffffffff812f4ed0);//addr of x64_sys_fcntl
    //execState->MoniStartOfSE(0xffffffff812e0410);//addr of x64_sys_write
    //execState->MoniStartOfSE(0xffffffff812dabc0);//addr of x64_sys_truncate
    //execState->MoniStartOfSE(0xffffffff8131cfc0);//addr of x64_sys_getcwd
    //execState->MoniStartOfSE(0xffffffff812db4a0);//addr of x64_sys_chdir
    //execState->MoniStartOfSE(0xffffffff812f2510);//addr of x64_sys_rename
    //execState->MoniStartOfSE(0xffffffff812f30b0);//addr of x64_sys_mkdir
    //execState->MoniStartOfSE(0xffffffff812f3300);//addr of x64_sys_rmdir
    //execState->MoniStartOfSE(0xffffffff812dc170);//addr of x64_sys_creat
    //execState->MoniStartOfSE(0xffffffff810b5f10);//addr of x64_sys_umask
    //execState->MoniStartOfSE(0xffffffff810badc0);//addr of x64_sys_getrlimit
    //execState->MoniStartOfSE(0xffffffff810bad40);//addr of x64_sys_setrlimit
    //execState->MoniStartOfSE(0xffffffff812f3c90);//addr of x64_sys_link
    //execState->MoniStartOfSE(0xffffffff812f36e0);//addr of x64_sys_unlink
    //execState->MoniStartOfSE(0xffffffff812f38b0);//addr of x64_sys_symlink
    //execState->MoniStartOfSE(0xffffffff812db7e0);//addr of x64_sys_chmod
    //execState->MoniStartOfSE(0xffffffff812f2f10);//addr of x64_sys_mknod
    //execState->MoniStartOfSE(0xffffffff81303920);//addr of x64_sys_sysfs
    //execState->MoniStartOfSE(0xffffffff810cf3b0);//addr of x64_sys_sched_get_priority_max


    //if uncommenting this chenge the declaration of tmp in centralhub.cpp
    //execState->MoniStartOfSE(0xffffffff810041b0);//addr of indirect call in do_syscall_64 

    

    to_native();

    init_t_ctx(); 
    dump_regs(); 

    return 0;
}
