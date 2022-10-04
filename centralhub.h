#ifndef _CENTRAL_HUB_H__
#define _CENTRAL_HUB_H__

#include <memory>
#include <vector>

//pp-s
#define SCALL_SETPRIORITY       141
#define SCALL_GETPRIORITY       140
#define SCALL_GETPID            039
#define SCALL_WRITE             001
#define SCALL_LSEEK             8
#define SCALL_SOCKET            41
#define SCALL_BIND              49
//pp-e

// #include "fatctrl.h"
// #include "CodeObject.h"
// #include "CodeSource.h"
// #include "InstructionDecoder.h"
// #include "dyntypes.h"
// #include "InstructionSource.h"
// 
// struct ElfModule {
//     char *fn;  // file name
//     ulong ba;  // base address in memory mapping
// 
//     ElfModule(const char *fname, ulong base_address);
//     ~ElfModule();
// };
// 
// class BPatch;
// class BPatch_image;
// class CodeRegion;
// class CodeSource;
// class CodeObject;
class VMState;
// class ExecCtrl;
class CFattCtrl;
class CThinCtrl;
struct pt_regs;

// using namespace Dyninst;
// using namespace ParseAPI;
// using namespace InstructionAPI;
// using namespace Dyninst::InstructionAPI;

// ulong Address;
// class CFacade {
//     std::shared_ptr<ElfModule> m_Elf;
//     std::shared_ptr<BPatch> m_BPatch;
//     std::shared_ptr<BPatch_image> m_AppImage;
// 
//     std::shared_ptr<VMState> m_VM;
//     std::shared_ptr<ExecCtrl> m_EC;
// 
//    public:
//     // Intialize with a code block;
//     CFacade(const uint8_t *code_block, ulong start_va, ulong size);
//     // Intialize with an elf_file;
//     CFacade(const char *elf_file, ulong base_address = 0);
//     ~CFacade();
// 
//     // Declare a symbolic variable in memory;
//     bool declareSymbolicObject(ulong addr, ulong size);
//     // Declare a register as symbolic variable;
//     bool declareSymbolicObject(uint register_index);
//     // Start processing at \c start_va, with CPU state \regs
//     bool processAt(struct pt_regs *regs);
// };
typedef struct EventMeta {
    unsigned long t_pf_stack;
    unsigned long t_int3_stack;
    unsigned long t_ve_stack;
    unsigned long t_db_stack;
    unsigned long* virt_exce_area;
} EveMeta;

class ExecState {
    std::shared_ptr<VMState> m_VM;
    // std::shared_ptr<ExecCtrl> m_EC;
    /* Jiaqi */
    // std::shared_ptr<CFattCtrl> m_FattCtrl;
    std::shared_ptr<CThinCtrl> m_ThinCtrl;
    /* /Jiaqi */
   
    public:
    /* Jiaqi */
    std::shared_ptr<CFattCtrl> m_FattCtrl;
    std::shared_ptr<EveMeta> m_emeta;
    /* /Jiaqi */
    
    ExecState(ulong adds, ulong adde);
    ~ExecState();

    // Declare a symbolic variable in memory;
    // bool declareSymbolicObject(ulong addr, ulong size, const char *name);
//pp-s
    //bool declareSymbolicObject(ulong addr, ulong size, bool isSigned, long conVal, const char *name);
    bool declareSymbolicObject(ulong addr, ulong size, bool isSigned, bool hasSeed, long conVal, const char *name);
//pp-e
    // Declare a register as symbolic variable;
    // bool declareSymbolicObject(uint register_index);
    // bool declareSymbolicRegister(uint index, uint size, const char *name); 
//pp-s
    //bool declareSymbolicRegister(uint index, uint size, bool isSigned, long conVal, const char *name); 
    bool declareSymbolicRegister(uint index, uint size, bool isSigned, bool hasSeed, long conVal, const char *name); 
//pp-e
    // Start processing at \c start_va, with CPU state \regs
    // bool SynRegsFromNative(struct pt_regs* regs);
    // bool SynRegsToNative(struct pt_regs* regs);
    bool SynRegsFromNative(struct MacReg* regs);
    bool SynRegsToNative(struct MacReg* regs);
    bool processAt(ulong addr);
    bool MoniStartOfSE(ulong addr);
    void InitRediPagePool();
    void DBHandler();
    //pp-s
    bool defineSymbolsForScalls(unsigned long scall_idx, unsigned long pt_regs_base_adr);
    //pp-e
};

#endif  // !_CENTRAL_HUB_H__
