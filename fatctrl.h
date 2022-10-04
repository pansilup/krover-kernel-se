#ifndef _FATCTRL_H__
#define _FATCTRL_H__

#include <linux/types.h>

#include <iostream>
#include <map>

#include "centralhub.h"

// struct ElfModule;
// class BPatch_function;
class VMState;
// class ExecCtrl;
class CThinCtrl;
class SymExecutor;
class ConExecutor;

/* meta and func for kernel func addr + call inst database */
struct call_insn {
    unsigned long addr;
    unsigned long dest;
    char orig_bytes[5];
    int len;
};

struct CallInAllFuncs{
    unsigned long func_addr;
    struct call_insn* call_insts;
    int num_call;
};

// struct func_call_inst func_call[44020];
struct hook_info {
    unsigned long addr;
    unsigned long dest;
    // char orig_bytes[5];
    char orig_bytes[1];//backup only one byte
    int len;//record the len of cur call inst, used when calculate ret addr
};

/* SE analyser's own page pool to hold T's code */
typedef struct pool
{
    void* init;
    void* next;
    void* end;
} POOL;

struct redir_page_info {
    unsigned long orig_t_page_addr;
    unsigned long new_ana_page_addr;
    unsigned long offset;
};

/* If we need more int3 probe or redirect page, adjust here */
#define MAX_INT3 30
#define MAX_Redir_Code_Page 8

class CFattCtrl {
    
    VMState *m_VM;

    // struct pt_regs* m_regs;
    POOL* m_page_pool;

    // /* If we need more int3 probe or redirect page, adjust here */
    // static const int max_int3 = 30;
    // static const int max_redirect_idx = 8;
    /* / */ 
    int crt_max_redir_idx;//the current max number of redirected pages
    int crt_redir_idx; //indicate the idx of the current in use redirected page
    // unsigned long redirected_pages[max_redirect_idx];
    // unsigned long new_pages[max_redirect_idx];
    // unsigned long offsets[max_redirect_idx];
    // struct redir_page_info redir_code_pages[max_redirect_idx];
    struct redir_page_info redir_code_pages[MAX_Redir_Code_Page];
    
    struct CallInAllFuncs* m_func_call;

    char per_hook[0x1];
    struct hook_info* probe_orig_inst;
    // std::map<ulong, struct hook_info*> m_probes;
    int crt_int3_idx;
    

    // SymExecutor *m_SymExecutor;
    // ConExecutor *m_ConExecutor;

    // // cache all basic blocks that don't use symbol: addr -> bool
    // std::map<ulong, bool> m_YesUsesymBB;
    // std::map<ulong, bool> m_NotUsesymBB;

   public:
    EveMeta* m_emeta;
    CThinCtrl *m_Thin;
    // CFattCtrl(ExecCtrl *EC, VMState *VM);
    CFattCtrl(VMState *VM, EveMeta* meta);
    ~CFattCtrl();

    // // import information about whether a basic block uses symbol;
    // bool importdata_BBUseSymbol(const char *config_file);

    // Process a function at each time;
    // bool processFunction(BPatch_function *F);
    bool processFunc(ulong addr);
    void INT3Handler(void);
    void VEHandler(void);
    void DBHandler(void);
    bool MoniStartOfSE (ulong addr);
    void InitRediPagePool(void);

   private:
    /* Jiaqi */
    void PoolInit (size_t size);
    void PoolDestroy (POOL *p);
    size_t PoolCheckAvail (POOL* p);
    void* PoolAlloc (POOL* p, size_t size);
    
    void InitFuncDB(const char* filename);
    int FindFuncCallInfo(unsigned long addr);
    void InstallPerInt3 (unsigned long addr, int len, unsigned long dest);
    void InstallInt3ForFunc (unsigned long func_addr);
    int find_probe_idx(unsigned long rip);
    void update_crt_redir_idx (unsigned long tempAddr);
    // void hypercall (void* ker_addr);
    void RedirCodePageHyperCall (void* ker_addr);
    unsigned long emulCall(struct pt_regs* regs);
    void clear_dr(int idx);
    /* /Jiaqi */
    
    bool CheckFuncSym(unsigned long);
    // test if a basic block use symbol;
    bool mustyesUseSymbol(ulong BB_addr);  // must yes
    bool mustnotUseSymbol(ulong BB_addr);  // must not
    bool mayUseSymbol(ulong BB_addr);      // maybe use
};
#endif  // _FATCTRL_H__
