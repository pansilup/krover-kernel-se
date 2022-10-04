
#ifndef __CPU_STATE_H__
#define __CPU_STATE_H__

#include <asm/ptrace.h>
#include <linux/types.h>
#include <signal.h>
#include <ucontext.h>

#include <map>

#include "oprand.h"
#include "defines.h"
#include "SymList.h"

// CPU state
struct pt_regs;
// class CFacade;
// class SymInfoDB;

// namespace Dyninst::InstructionAPI {
// class Instruction;
// class Expression;
// class RegisterAST;
// class Result;
// }  // namespace Dyninst::InstructionAPI

struct DyinstEC {
    uint32_t offt, size;
};

struct MacReg {
    struct pt_regs regs;
    ulong fs_base;
    ulong gs_base;
};


// Manage CPU registers
// #define PTREGS_REG_TOTAL 20  //~PTRegsEncoding.size()
// #define DYINST_REG_TOTAL 30  //~DyinstEncoding.size()
#define PTREGS_REG_TOTAL 23  //~PTRegsEncoding.size()
#define DYINST_REG_TOTAL 32  //~DyinstEncoding.size()
class SYCPUState {
    /*
    x86_64::FULL    0
    x86_64::L_REG   1    first  8bits
    x86_64::H_REG   2    second 8bits
    x86_64::W_REG   3    first  16bits
    x86_64::D_REG   f    first  32bits
    */
    #define REG_32H    6
    #define REG_16H    5
    #define REG_D       4
    #define REG_W       3
    #define REG_H       2
    #define REG_L       1
    #define REG_FULL    0

   private:
    static std::map<uint, std::string> PTRegsEncoding;
    static std::map<uint, DyinstEC> DyinstEncoding;
    union {
        ulong m_ArrRegs[DYINST_REG_TOTAL];
        // pt_regs m_PTRegs;
        MacReg m_PTRegs;
    };
    
    uint64_t m_symBitmap[DYINST_REG_TOTAL] ;
    Symbol_List_Map m_symList[DYINST_REG_TOTAL] ;

    // struct SymMachineReg {
    //     bool bSym ;
    //     bool bValid ;
    //     KVExprPtr symval ;
    //     
    //     SymMachineReg () {
    //         bSym = false ;
    //         bValid = false ;
    //         symval = NULL ;
    //     } ;
    // } ;
    // 
    // SymMachineReg m_symMR[DYINST_REG_TOTAL][7] ;
    // // SymMachineReg m_symMR[PTREGS_REG_TOTAL][7] ;
    /* / */

    struct MachineReg {
        uint indx;  // Register index
        uint size;  // number of bytes
        bool bsym_flag;  // is a symbolic value?
        uint64_t *pSymBitmap;
        Symbol_List_Map *pSymList ;
        union {
            int64_t *pi64;
            int32_t *pi32;
            int16_t *pi16;
            int8_t *pi8;
            uint64_t *pu64;
            uint32_t *pu32;
            uint16_t *pu16;
            uint8_t *pu8;
        };
        // SymMachineReg *p_symMR ;
        union {
            KVExprPtr symval;    // symbolic expression
            FSInstrPtr fsinstr;  // used for lazily calculate CPU bit falgs.
            // FLAG_STAT flag;
        };

        MachineReg() : bsym_flag(false), symval(nullptr) {}
        ~MachineReg() {}
    };
    typedef std::shared_ptr<MachineReg> MachineRegPtr;
    // All used registers
    std::map<uint, MachineRegPtr> m_Regs;

   public:
    SYCPUState(void) : m_Regs(), m_symList() {
        MachineReg *R;
        for (auto E : DyinstEncoding) {
            uint iddy = E.first;
            uint idpt = E.second.offt;
            uint size = E.second.size;
            R = new MachineReg();
            R->indx = iddy;
            R->size = size;
            R->pu64 = &m_ArrRegs[idpt];
            R->pSymBitmap = &m_symBitmap[idpt];
            R->pSymList = &m_symList[idpt] ;

            // m_symMR[idpt][0].bValid = true ;
            // R->p_symMR = m_symMR[idpt] ;
            
            m_Regs[iddy].reset(R);
        };
    }

    ~SYCPUState(void) {
        m_Regs.clear();
    }

    // update state with concrete values, always invoke before symbolic execution
    // bool setConcreteCPUState(struct pt_regs *regs);
    bool setConcreteCPUState(struct MacReg *regs);
    
    // Jiaqi, synchronize the regs back to native
    // bool readConcreteCPUState(struct pt_regs *regs);
    bool readConcreteCPUState(struct MacReg *regs);
    bool clearAllSymFlag(void);
    struct pt_regs* getPTRegs(void);
    
    ulong readConReg(uint idx);
    bool writeConReg(uint idx, ulong val);
    /* / */

    bool isSYReg(uint reg_index);

    bool isSYReg(MachineReg *R, int subRegIndex) ;

    bool setSYReg(MachineReg *R) ;
    bool clrSYReg(MachineReg *R) ;

    SYCPUState::MachineReg* idxToReg(uint reg_index) ;
    int RegToSubIndex (MachineReg *R) ;
    bool regIdxToAddrSize(int idx, uint64_t &addr, int &size) ;

    bool writeRegister(RegValue &v);
    bool readRegister(RegValue &v);

    bool getFlagBit(uint flag_index, FSInstrPtr &ptr);
    bool setFlagBit(uint flag_index, FSInstrPtr &ptr);
    
    bool getFlagBit(uint flag_index, FLAG_STAT &flag);
    bool setFlagBit(uint flag_index, FLAG_STAT &flag);
    
    bool FlagBitDefinited(uint flag_index) ;

   private:

    bool writeConcreteValue(MachineReg *R, RegValue &V);
    bool readConcreteValue(MachineReg *R, RegValue &V);
    // bool isSYReg(uint reg_index);
    // bool writeRegister(RegValue &v);
    // bool readRegister(RegValue &v);
    // 
    // bool getFlagBit(uint flag_index, FSInstrPtr &ptr);
    // bool setFlagBit(uint flag_index, FSInstrPtr &ptr);

    // //haiqing
    // bool getFlagBit(uint flag_index, FLAG_STAT &flag);
    // bool setFlagBit(uint flag_index, FLAG_STAT &flag);
    // 
    // bool FlagBitDefinited(uint flag_index) ;
    // /* / */

   // private:
    // bool writeSymbolicValue(MachineReg *R, KVExprPtr &e);
    // bool readSymbolicValue(MachineReg *R, KVExprPtr &e);

    // bool writeConcreteValue(MachineReg *R, RegValue &V);
    // bool readConcreteValue(MachineReg *R, RegValue &V);


    // int RegToIndex (MachineReg *R);
    // bool ReadSubRegValue (MachineReg* R, RegValue &V) ;

    // bool writeConcreteValue(MachineReg *R, int subRegIdx, RegValue &V);
    // bool readConcreteValue(MachineReg *R, int subRegIdx, RegValue &V);

    // bool combineChildren(MachineReg *R, int subRegIndex) ;
    // bool combineChildren(MachineReg *R, int resultIndex, int child_H, int child_L, int child_size) ;

    // bool invalidateTree(MachineReg *R, int subRegIndex) ;
    // bool splitParentOf(MachineReg *R, int subRegIndex) ;
    // bool splitParent(MachineReg *R, int p, int child_H, int child_L, int child_size) ;

    // bool isSubRegSymbol(MachineReg *R, int subRegIndex, bool &bSym) ;
    // bool isParentRegSymbol(MachineReg *R, int subRegIndex, bool &bSym) ;

    // bool setParentSymbolState(MachineReg *R, int subRegIndex, bool bsym) ;
    // bool setChildSymbolState(MachineReg *R, int subRegIndex, bool bsym) ;
    // bool tryConcreteParentOf(MachineReg *R, int subRegIndex) ;

    // bool printsymMR(MachineReg *R) ;
    
    //    public:
    // Read & write registers
    // bool isSYReg(uint reg_index);
    // bool setRegValue(uint reg_index, ulong val);
    // // bool setRegValue(uint reg_index, KVExpr *e);
    // bool setRegValue(uint reg_index, KVExprPtr &e);
    // bool getRegValue(uint reg_index, ulong *val);
    // // bool getRegValue(uint reg_index, KVExpr **e);
    // bool getRegValue(uint reg_index, KVExprPtr &oute);

    // // Read & write memory cells
    // bool isSYMemoryCell(ulong memory_addr, uint length);
    // bool setMemValue(ulong address, uint length, ulong val);
    // // bool setMemValue(ulong address, uint length, KVExpr *e);
    // bool setMemValue(ulong address, uint length, KVExprPtr &e);
    // bool getMemValue(ulong address, uint length, ulong *outi);
    // // bool getMemValue(ulong address, uint length, KVExpr **oute);
    // bool getMemValue(ulong address, uint length, KVExprPtr &oute);
};

#endif  // !__CPU_STATE_H__
