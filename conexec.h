#ifndef _CON_EXEC_H__
#define _CON_EXEC_H__

#include <linux/types.h>

#include <iostream>
#include <vector>

#include <asm/ptrace.h>

#include "CodeObject.h"
#include "InstructionDecoder.h"
#include "thinctrl.h"
#include "centralhub.h"
#include "defines.h"

struct OprndInfo;
class SymInfoDB;
class VMState;

namespace Dyninst::InstructionAPI {
class Instruction;
class Expression;
}  // namespace Dyninst::InstructionAPI

class ConExecutor {
   // protected:
   //  std::vector<InstrInfoPtr> m_IOIs;
   //  bool m_RIPUpdated;  // Is RIP already updated in run()?

    /* Jiaqi */
   private:
    // std::shared_ptr<MyCodeRegion> m_cr;
    // InstructionDecoder* decoder;
    // VMState *m_VM;
    // Instruction* crtInsn;//current processing instruction
    // ulong crtAddr;//addr of crtInsn. Caveat: it is not next RIP
    // struct pt_regs* m_regs;

    void* T_page;//The page for ana to execute T_Insn 
    char Jmp_RIP_Insn[6];
    char Lea_RIP_Insn[7];
    // std::list<unsigned char> NopBytes = std::initializer_list<unsigned char>({0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90})
    unsigned char NopBytes[15] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

    bool if_condition_fail (entryID opera_id, struct pt_regs* regs);
    bool bind_value_for_exp(Instruction* instr, Expression::Ptr target, struct pt_regs* regs);
    bool emul_cf_inst(Instruction* instr, InsnCategory Cate, struct pt_regs* regs);
    // bool RewRIPInsn(void* new_insn, char* orig_insn, Instruction* instr);
    // bool RewRIPInsn(void* orig_insn_addr, Instruction* instr);
    int RewRIPInsn(void* T_insn, void* orig_insn_addr, Instruction* instr);
    bool ClearTinsn(void* T_addr, int size);

    bool InsertInsnUpdateR15(void* nt_t_page, ulong newR15);

    bool checkIfMemUseWriteReg(Instruction* in, std::set<int> writeRegIDs);
    bool checkIfImplicitMemUseWriteReg(Instruction* in, std::set<int> writeRegIDs);

    /* /Jiaqi */

   public:
    // ConExecutor() : m_IOIs() {}
    // ConExecutor(CodeRegion* m_cr, InstructionDecoder* decoder); 
    // ConExecutor(VMState* vm); 
    // ConExecutor(CThinCtrl* m_tCtrl); 
    CThinCtrl* m_ThinCtrl;
    ConExecutor(); 
    ~ConExecutor(){}; 
    // static ConExecutor *GetInstance(void);

    /* Jiaqi */
    // void InsnExecNonRIP(struct pt_regs* regs);
    // void InsnExecRIP(struct pt_regs* regs);
    // void BlockExecRIP(struct pt_regs* regs);

    // // bool InsnDispatch(ulong addr);
    // // bool InitRegs(VMState *VM, struct pt_regs* regs);
    // bool ReadRegs(VMState *VM);
    // bool SetRegs(VMState *VM);
    // bool InitRegs(std::shared_ptr<VMState> VM);
    // bool InsnDispatch(Instruction* instr, struct pt_regs* regs);
    // bool InsnDispatch(Instruction* instr);
    // bool InsnDispatch(Instruction* instr, struct pt_regs* regs, bool isRIP);
    bool InsnDispatch(Instruction* instr, struct pt_regs* regs);
    // bool BlockDispatch(Address S_Addr, Address E_Addr);
    bool BlockDispatch(Address S_Addr, struct pt_regs* regs);
    /* /Jiaqi */

    // // bool pushInstr(DAPIInstr *I, std::vector<OprndInfo *> *vecOI);
    // bool pushInstr(InstrInfoPtr &ptr);
    // bool run(VMState *cs);
    // bool _run_prologue(void);
    // bool _run_postlogue(void);

    // bool process_jmp(VMState *vm, InstrInfoPtr &infoptr);
    // bool process_call(VMState *vm, InstrInfoPtr &infoptr);
    // bool process_jz(VMState *vm, InstrInfoPtr &infoptr);
    // bool process_jnz(VMState *vm, InstrInfoPtr &infoptr);
    // bool process_jncc(VMState *vm, InstrInfoPtr &infoptr);
    // bool process_add(VMState *vm, InstrInfoPtr &infoptr);
    // bool process_lea(VMState *vm, InstrInfoPtr &infoptr);
    // bool process_mov(VMState *vm, InstrInfoPtr &infoptr);
    // bool process_test(VMState *vm, InstrInfoPtr &infoptr);
};

// class FSTestCon : public FlagSettingInstr {
//    public:
//     FSTestCon(InstrInfoPtr &ptr) : FlagSettingInstr(ptr) {}
//     ~FSTestCon() {}
//     bool calc_sflag(void);
//     bool calc_zflag(void);
// };

#endif  // !_CON_EXEC_H__
