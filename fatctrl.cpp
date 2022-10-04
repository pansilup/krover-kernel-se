#include "fatctrl.h"

#include <asm/ptrace.h>
#include <linux/types.h>
#include <signal.h>
#include <ucontext.h>

#include <iostream>

#include "BPatch.h"
#include "BPatch_basicBlock.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
// #include "CPUState.h"
#include "VMState.h"
#include "defines.h"
#include "interface.h"
#include "thinctrl.h"

/* Jiaqi */
#include "fatctrl.h"
/* /Jiaqi */

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;

/* Jiaqi */
void CFattCtrl::InitFuncDB(const char* filename)
{
    // FILE *fp = fopen("/home/jqhong/Documents/test_user/kernel_se/ker_func.txt", "r");
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf ("open ker_func.txt failed. \n");
        exit(EXIT_FAILURE);
    }

    char chunk[128];
    size_t len = sizeof(chunk);
    char* line = (char*) malloc(len);
    line[0] = '\0';

    int i = 0;
    while ((fgets(chunk, sizeof(chunk), fp)) != NULL){
        strcpy(line, chunk);
        if (line[strlen(line) - 2] == ':')
        {   
            char* tmp = strtok(line, ":");
            // printf ("%s. \n", tmp);
            m_func_call[i].func_addr = strtoul(tmp, NULL, 16);
            if (fgets(chunk, sizeof(chunk), fp) != NULL)
            {
                strcpy(line, chunk);
                int idx = strtol(line, NULL, 10);
                int j;
                // printf ("i: %d. func_addr: %lx. %d of call inst. \n", i, func_call[i].func_addr, idx);
                m_func_call[i].num_call = idx;
                m_func_call[i].call_insts = (struct call_insn*)malloc(sizeof(struct call_insn)*idx);
                for (j = 0; j < idx; j++)
                {
                    line[0] = '\0';
                    if (fgets(chunk, sizeof(chunk), fp) != NULL)
                    {
                        strcpy(line, chunk);
                        tmp = strtok(line, " ");
                        m_func_call[i].call_insts[j].addr = strtoul(tmp, NULL, 16);
                        tmp = strtok(NULL, " ");
                        m_func_call[i].call_insts[j].len = strtol(tmp, NULL, 10);
                        tmp = strtok(NULL, " ");
                        if (tmp[0] != 'f')
                        {
                            m_func_call[i].call_insts[j].dest = 0;
                        }
                        else
                        {
                            m_func_call[i].call_insts[j].dest = strtoul(tmp, NULL, 16);
                        }
                        // printf("call inst addr: %lx. len: %d, dest: %lx \n", func_call[i].call_insts[j].addr, func_call[i].call_insts[j].len, func_call[i].call_insts[j].dest);
                    }
                }
            }
            i ++;
        }
        line[0] = '\0';
    }

    fclose(fp);
    if (line)
        free(line);
    return;
    // printf ("i: %d. \n", i);
}

void CFattCtrl::InitRediPagePool()
{
    //should be after get_target()
    ///* initialize redirect_page_pool */
    m_page_pool = (POOL*) new (POOL);
    PoolInit (0x1000*MAX_Redir_Code_Page);
    return;
}

// CFattCtrl::CFattCtrl(ExecCtrl *EC, VMState *VM) : m_YesUsesymBB(), m_NotUsesymBB() {
// CFattCtrl::CFattCtrl(VMState *VM, EveMeta* meta) : m_YesUsesymBB(), m_NotUsesymBB() {
CFattCtrl::CFattCtrl(VMState *VM, EveMeta* meta) {
    m_VM = VM;
    // m_EC = EC;
    // m_Elf = EC->m_Elf;
    /* Jiaqi */
    // m_regs = new(struct pt_regs);
    m_emeta = meta;

    // /***********************************************/ 
    // //should be after get_target()
    // ///* initialize redirect_page_pool */
    // m_page_pool = (POOL*) new (POOL);
    // PoolInit (0x1000*MAX_Redir_Code_Page);

    // /***********************************************/ 
    
    void* temp;
    /* empty the memory where stores the global data structures */
    // temp = &redirected_pages[0];
    // memset(temp, 0x0, sizeof(redirected_pages));
    // temp = &new_pages[0];
    // memset(temp, 0x0, sizeof(new_pages));
    // temp = &offsets[0];
    // memset(temp, 0x0, sizeof(offsets));
    temp = &redir_code_pages[0];
    memset(temp, 0x0, sizeof(redir_code_pages));
    
    crt_redir_idx = crt_max_redir_idx = 0;
    
    /* Int3 probe related */
    per_hook[0] = 0xcc;
    crt_int3_idx = 0;
    probe_orig_inst = (struct hook_info*)malloc(MAX_INT3*sizeof(struct hook_info));

    /* Init Func call info database */
    std::string filename = "/home/neo/smu/kernel-se/k-test/ker_func.txt";
    m_func_call = (struct CallInAllFuncs*)malloc(sizeof(struct CallInAllFuncs)*44020);
    InitFuncDB(filename.c_str());
    /* /Jiaqi */
    // m_SymExecutor = (EC->m_SymExecutor).get();
    // m_ConExecutor = (EC->m_ConExecutor).get();
}

CFattCtrl::~CFattCtrl() {
    // free resources

    /* Jiaqi, TODO: complete the objects free */
    PoolDestroy(m_page_pool);
    // /* / */
    // m_YesUsesymBB.clear();
    // m_NotUsesymBB.clear();
}

int CFattCtrl::FindFuncCallInfo(unsigned long addr)
{
    int low = 0; 
    int high = 44019;
    while (low <= high) {
        int mid = (low + high)/2;
        int midVal = m_func_call[mid].func_addr;
        if (midVal < addr)
            low = mid + 1;
        else if (midVal > addr)
            high = mid - 1;
        else
            return mid;
    }
    return -1;
}
/* /Jiaqi */

// bool CFattCtrl::processFunction(BPatch_function *F) {
// consult Func database, determine if it should be delivered to thinCtrl 
// bool CFattCtrl::processFunction(ulong addr, struct pt_regs *regs) {
// bool CFattCtrl::StartAt(ulong addr, struct pt_regs *regs) {
bool CFattCtrl::processFunc(ulong addr) {
    // BPatch_flowGraph *cfg = F->getCFG();
    // BPatch_basicBlock *B;
    // RegValue V{(uint)x86_64::rip, 8, false, 0};
    // ulong rip;

    // m_VM->readRegister(V);
    // B = cfg->findBlockByAddr(V.u64 - m_Elf->ba);
    // while (B != NULL) {
    //     m_Thin->processBasicBlock(B);
    //     m_VM->readRegister(V);
    //     B = cfg->findBlockByAddr(V.u64 - m_Elf->ba);
    // }
    // // ==============
    // // test memory
    // unsigned long t_addr = 0xffffffff81098800;
    // MemValue MV{t_addr, 8, false};
    // int res = m_VM->readMemory(MV);
    // // ==============
    if (CheckFuncSym(addr) == 0)//not found in database, should be executed natively
        return false;
    else
    {
        // VMState::SetCPUState(m_VM, regs);
        // // BPatch_function *func;
        // std::cout << m_Thin << std::endl;
        m_Thin->processFunction(addr);

    }
    
    return true;
}

void CFattCtrl::PoolInit (size_t size)
{
    void* temp = valloc(size);
    memset (temp, 0x0, size);
    
    // m_page_pool->init = temp;
    // m_page_pool->next = temp;
    m_page_pool->init = (void*)((unsigned long)temp + 0x1000);//The first page will not encounter #PF, so, start from the second page
    m_page_pool->next = (void*)((unsigned long)temp + 0x1000);
    m_page_pool->end = (void*)((unsigned long)temp + size);
    
    printf ("redirected page start from :%p. ends : %p. \n", temp, m_page_pool->end);
    return;
}

void CFattCtrl::PoolDestroy (POOL *p)
{
    free(p);
}

size_t CFattCtrl::PoolCheckAvail (POOL* p)
{
    return (unsigned long)p->end - (unsigned long)p->next;
}

void* CFattCtrl::PoolAlloc (POOL* p, size_t size)
{
    if (PoolCheckAvail(p) < size)
    {
        return NULL;
    }
    void* mem = (void*) p->next;
    p->next = (void*)((unsigned long)p->next + size);
    return mem;
}
/* / */



// void CFattCtrl::hypercall (void* ker_addr)
void CFattCtrl::RedirCodePageHyperCall (void* ker_addr)
{
    // ttt0 = rdtsc ();
    
    if (crt_max_redir_idx == MAX_Redir_Code_Page)
    {
        printf ("new_pages used up. \n");
        asm volatile ("movq $0x999999, %%rax; \n\t"
                "vmcall; \n\t"
                :::"%rax");
    }

    // /* vmcall to set PF bit in exception_bitmap */
    // asm volatile ("movq $0xaaaaa, %%rax; \n\t"
    //         "vmcall; \n\t"
    //         :::"%rax");
    // /* / */

    // void* new_va = valloc (0x1000);
    void* new_va = PoolAlloc (m_page_pool, 0x1000);
    printf ("new_va: %lx. ker_addr: %lx. \n", (unsigned long)new_va, (unsigned long)ker_addr);

    memcpy (new_va, ker_addr, 0x1000);
    // memset (new_va, 0x0, 0x1000);
    printf ("about to issue hyper call to redirect page: %lx .\n", (unsigned long)ker_addr);
    // asm volatile ("movq $0x999999, %%rax; \n\t"
    //         "vmcall; \n\t"
    //         :::"%rax");

    // if ((((unsigned long) new_va) & 0xfff) != 0)
    // {
    //     printf ("non-align va: %lx. \n", new_va);
    // }

    /* issue a hypercall to request ept redirection for new page */
    asm volatile ("movq $0xabcd, %%rbx; \n\t"
            "movq %0, %%rax; \n\t"
            "movq %1, %%rcx; \n\t"
            "lea 0x2(%%rip), %%rdx; \n\t"
            "jmpq *%%rax; \n\t"
            ::"m"(ker_addr), "m"(new_va):"%rax","%rbx","%rcx");
    /* / */

    // redirected_pages[crt_max_redir_idx] = (unsigned long) ker_addr;
    // new_pages[crt_max_redir_idx] = (unsigned long) new_va;
    // // if (((unsigned long)ker_addr) < uk_border)
    // // {
    // //     offsets[crt_max_redir_idx] = (((Address)new_va - (Address)ker_addr));
    // // }
    // // else
    // // {
    //     offsets[crt_max_redir_idx] = ((unsigned long)ker_addr) - ((unsigned long)new_va);
    // // }
    redir_code_pages[crt_max_redir_idx].orig_t_page_addr = (unsigned long) ker_addr;
    redir_code_pages[crt_max_redir_idx].new_ana_page_addr = (unsigned long) new_va;
    redir_code_pages[crt_max_redir_idx].offset = ((unsigned long)ker_addr) - ((unsigned long)new_va);
    
    /* update the crt_redir_idx */
    crt_redir_idx = crt_max_redir_idx;
    
    crt_max_redir_idx ++;
    
    // ttt1 = rdtsc();
    // ttt += ttt1 - ttt0;
    // /* vmcall to clear PF bit in exception_bitmap */
    // asm volatile ("movq $0xbbbbb, %%rax; \n\t"
    //         "vmcall; \n\t"
    //         :::"%rax");
    // /* / */
    
    // printf ("new_va: %lx. t_ker_addr: %lx. crt_idx: %d, crt_max_redir_idx: %d. \n", new_va, ker_addr, crt_redir_idx, crt_max_redir_idx);
    // printf ("new_va: %lx. ker_addr: %lx, crt_idx: %d. \n", new_va, ker_addr, crt_idx);

    return;
}


void CFattCtrl::update_crt_redir_idx (unsigned long tempAddr)
{
    int i;
    for (i = 0; i < crt_max_redir_idx; i ++)
    {
        // if (tempAddr == redirected_pages[i])
        if (tempAddr == redir_code_pages[i].orig_t_page_addr)
        {
            crt_redir_idx = i;
            // // printf ("update crt_idx as: %d. tempAddr: %lx. \n", crt_redir_idx, tempAddr);
        }
    }
    if (i == crt_max_redir_idx)
    {
        RedirCodePageHyperCall((void*) tempAddr);
    }
    return;
}

// void CFattCtrl::InstallPerInt3 (unsigned long addr, int len, unsigned long dest)
void CFattCtrl::InstallPerInt3 (unsigned long addr, int len, unsigned long dest)
{
    probe_orig_inst[crt_int3_idx].addr = addr;
    probe_orig_inst[crt_int3_idx].dest = dest;
    probe_orig_inst[crt_int3_idx].len = len;
    // memcpy (&probe_orig_inst[crt_int3_idx].orig_bytes, (void*)addr, len);
    memcpy (&probe_orig_inst[crt_int3_idx].orig_bytes, (void*)addr, 0x1);
    
    // memcpy ((void*)(addr-offsets[crt_redir_idx]), per_hook, 0x1);//install the new hook
    memcpy ((void*)(addr-redir_code_pages[crt_redir_idx].offset), per_hook, 0x1);//install the new hook
    // }
    // printf ("va: %lx, content: %lx. \n", addr-offsets[crt_redir_idx], *((unsigned long*)(addr-offsets[crt_redir_idx])));
    crt_int3_idx ++;
    if (crt_int3_idx >= MAX_INT3)
    {
        printf ("int3 array used up, int3_array_idx: %d. \n", crt_int3_idx);
        asm volatile ("movq $0x999999, %%rax; \n\t"
                "vmcall; \n\t"
                :::"%rax");
    }
    return; 
}

void CFattCtrl::InstallInt3ForFunc (unsigned long func_addr)
{
    printf ("install probe for func : %lx. \n", func_addr);
    int func_idx = FindFuncCallInfo(func_addr); 
    /* no call inst in func */
    if (func_idx == -1)
    {
        printf ("no call inst in func \n");
        // asm volatile ("movq $0x999999, %%rax; \n\t"
        //         "vmcall; \n\t"
        //         :::"%rax");
    }
    else
    {
        int total = m_func_call[func_idx].num_call;
        struct call_insn* ptr = m_func_call[func_idx].call_insts; 
        int i;
        unsigned long addr_l, addr_h;
        int len;
        unsigned long dest;
        addr_l = ptr[0].addr;
        len = ptr[0].len;
        dest = ptr[0].dest;
        
        update_crt_redir_idx(addr_l & ~0xfff);
        InstallPerInt3(addr_l, len, dest);
        printf ("install probe at: %lx. \n", addr_l);
        
        for (i = 1; i < total; i ++)
        {
            addr_h = ptr[i].addr;
            len = ptr[i].len;
            dest = ptr[i].dest;
            // if ((addr_h & ~0xfff) != redirected_pages[crt_redir_idx])
            if ((addr_h & ~0xfff) != redir_code_pages[crt_redir_idx].orig_t_page_addr)
            {
                update_crt_redir_idx(addr_h & ~0xfff);
            }
            InstallPerInt3(addr_h, len, dest);
            printf ("...install probe at: %lx. \n", addr_h);
        }
    }
    return;
}

bool CFattCtrl::MoniStartOfSE (ulong addr)
{
    // unsigned long addr_l = 0xffffffff81c00087;
    unsigned long addr_l = addr;
    int len = 5;
    unsigned long dest = 0xffffffff810b6080;
    
    update_crt_redir_idx(addr_l & ~0xfff);
    InstallPerInt3(addr_l, len, dest);
    printf ("install probe at: %lx. \n", addr_l);
    return true;
}

int CFattCtrl::find_probe_idx(unsigned long rip)
{
    int i; 
    for (i = 0; i < crt_int3_idx; i ++)
    {
       if(probe_orig_inst[i].addr == rip)
       {
           if(probe_orig_inst[i].dest)
           {
               return i; //probe_orig_inst[i].dest;
           }
           else//needs a disassembler to find the dest 
           {
                printf ("need a disassembler to parse the dest. \n");
                asm volatile ("movq $0xfff, %%rax; \n\t"
                        "vmcall; \n\t"
                        :::"%rax");
           }
       }
    }
    if (i == crt_int3_idx)
    {
        printf ("addr not found in installed probe. \n");
        asm volatile ("movq $0xfff, %%rax; \n\t"
                "vmcall; \n\t"
                :::"%rax");
    }
}

/* return the addr of call destination */
// unsigned long CFattCtrl::emulCall (void)
unsigned long CFattCtrl::emulCall (struct pt_regs* regs)
{
    // unsigned long* int3_stack_ptr = (unsigned long*)(t_int3_stack - 0x28);
    unsigned long* int3_stack_ptr = (unsigned long*)(m_emeta->t_int3_stack - 0x28);
    unsigned long saved_rip, saved_rsp, saved_rflags;
    int probe_idx;
    unsigned long ret_addr, call_dest;
    unsigned long* t_stack_ptr;
    saved_rip = int3_stack_ptr[0];
    saved_rip -= 1;// for int3, saved rip is the rip next to int3
    saved_rflags = int3_stack_ptr[2];
    saved_rsp = int3_stack_ptr[3];
    printf ("saved rip: %lx. rsp: %lx, rflags: %lx. \n", saved_rip, saved_rsp, saved_rflags);
    // asm volatile ("movq $0xfff, %%rax; \n\t"
    //         "vmcall; \n\t"
    //         :::"%rax");
    
    probe_idx = find_probe_idx(saved_rip);
    call_dest = probe_orig_inst[probe_idx].dest;//resolve the call destination based on the saved rip in int3 stack.
   
    // printf ("target_ctx at: %p. target_rsp: %lx, target_rflags: %lx. dest: %lx. rdi: %lx. \n", target_ctx, target_ctx->rsp, target_ctx->eflags, call_dest, target_ctx->rdi);
    // printf ("rsi: %lx, rdx: %lx. \n", target_ctx->rsi, target_ctx->rdx);
  
    ret_addr = saved_rip + probe_orig_inst[probe_idx].len;  
    t_stack_ptr = (unsigned long*)saved_rsp;
    t_stack_ptr --;
    *t_stack_ptr = ret_addr;
    // printf ("new stack: %p, ret_addr: %lx. \n", t_stack_ptr, ret_addr);

    // target_ctx->rsp = (unsigned long)t_stack_ptr;
    // target_ctx->eflags = saved_rflags;
    // 
    // board_ctx->rip = call_dest;
    regs->rsp = (unsigned long)t_stack_ptr;
    regs->eflags = saved_rflags;
    
    regs->rip = call_dest;
    
    // printf ("after adjustment... target_rsp: %lx, target_rflags: %lx. rdi: %lx. \n", target_ctx->rsp, target_ctx->eflags, target_ctx->rdi);
    printf ("after adjustment... target_rsp: %lx, target_rflags: %lx. rdi: %lx. \n", regs->rsp, regs->eflags, regs->rdi);
    // printf ("rsi: %lx, rdx: %lx. \n", target_ctx->rsi, target_ctx->rdx);

    return call_dest;
}

// void CFattCtrl::INT3Handler(bool indicator)
// Here determines whether to invoke ThinCtrl or resume Native
void CFattCtrl::INT3Handler(void)
{
    // if from thinCtrl, syn regs from m_CPU, if from native, syn from oasis
    // engine
    // m_VM->ReadCPUState(m_VM, m_regs);

    struct pt_regs* m_regs = m_VM->getPTRegs();
    unsigned long call_dest = emulCall(m_regs);
    
    InstallInt3ForFunc(call_dest);
    printf ("int3 invoked. \n");
    return;
}

// To complete
void CFattCtrl::VEHandler(void)
{
    unsigned long* virt_exec_area;
    unsigned long exit_qual = m_emeta->virt_exce_area[1];
    if ((exit_qual & 0x4UL) != 0)
    {
        printf ("unexpected EPT violation . \n");
        asm volatile("movq $0x99999, %%rax; \n\t"
                "vmcall; \n\t"
                :::"%rax");
    }
    else
    {
        unsigned long va = m_emeta->virt_exce_area[2];
        // int ret = determ_sym_mem(va);
        // crtAddr = va;
        unsigned long* ve_stack_ptr = (unsigned long*)(m_emeta->t_ve_stack - 0x28);
        unsigned long saved_rip, saved_rsp, saved_rflags;
        saved_rip = ve_stack_ptr[0];
        saved_rflags = ve_stack_ptr[2];
        saved_rsp = ve_stack_ptr[3];
        printf ("saved rip: %lx. rsp: %lx, rflags: %lx. \n", saved_rip, saved_rsp, saved_rflags);
        // ulong insn_addr = m_emeta->ve_stack[];
        bool ret = m_VM->isSYMemoryCell(va, 8);
        m_emeta->virt_exce_area[0] = 0x0UL;
        if (ret == 0)//execute one Instruction then resume native
        {
            // m_ThinCtrl->ExecOneInsn(saved_rip);
            m_Thin->ExecOneInsn(saved_rip);
            // InsnDispatch();
        }
        else
        {
            printf ("invoke symExecutor. \n");
            asm volatile ("mov $0x99999999, %rax; \n\t"
                    "vmcall; \n\t");
            // SymExecutor();
            // update_watchpoint();
        }
    }
    return;
}

/* no need to clear dr0-dr3, disable through dr7 */
void CFattCtrl::clear_dr(int idx)
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

// To complete
void CFattCtrl::DBHandler(void)
{
    clear_dr(0);
    printf ("in DB handler. \n");
    asm volatile ("mov $0x99999999, %rax; \n\t"
            "vmcall; \n\t");
    // SymExecutor();
    // update_watchpoint();
    return;
}

// bool CFattCtrl::importdata_BBUseSymbol(const char *config_file) {
//     FILE *f = fopen(config_file, "r");
//     if (f == NULL)
//         return false;
// 
//     char buf[LINE_MAX];
//     while (fgets(buf, LINE_MAX, f) != 0) {
//         switch (buf[0]) {
//             case 1: {
//                 m_YesUsesymBB[0] = true;
//                 break;
//             }
//             case 2: {
//                 m_NotUsesymBB[0] = true;
//                 break;
//             }
//         }
//     }
// 
//     fclose(f);
//     return true;
// }

// To complete
bool CFattCtrl::CheckFuncSym(unsigned long addr)
{
    return true;
}

// // test if a basic block use symbol;
// bool CFattCtrl::mustyesUseSymbol(ulong BB_addr) {
//     auto it = m_YesUsesymBB.find(BB_addr);
//     if (it != m_YesUsesymBB.end())
//         return true;
//     else
//         return false;  // undefined
// }
// 
// bool CFattCtrl::mustnotUseSymbol(ulong BB_addr) {
//     auto it = m_NotUsesymBB.find(BB_addr);
//     if (it != m_NotUsesymBB.end())
//         return true;
//     else
//         return false;  // undefined
// }
// 
// bool CFattCtrl::mayUseSymbol(ulong BB_addr) {
//     // not checked, return true ?
//     return !mustnotUseSymbol(BB_addr);
// }
