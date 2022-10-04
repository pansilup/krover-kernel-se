#ifndef _VMSTATE__H__
#define _VMSTATE__H__

#include <linux/types.h>

#include <map>

#include "CodeObject.h"
#include "CPUState.h"
#include "InstructionDecoder.h"
#include "defines.h"

class SYCPUState;
class SYMemState;
class EFlagsManager ;
class CPU_EFlags ;


class VMState {
    public: 
    /* memory object specified by user */
    struct SYMemObject {
        std::string name;  // name of object
        ulong addr;        // memory address
        ulong size;        // size in bytes
        bool is_signed; // @THX specify unsigned/signed
        bool has_seed;      //whether there's a seed for symbol
        KVExprPtr expr;    // point to a KVExprPtr
        //concrete value of symbol
        union {
            int64_t i64;
            int32_t i32;
            int16_t i16;
            int8_t i8;
            uint64_t u64;
            uint32_t u32;
            uint16_t u16;
            uint8_t u8;
        };
        SYMemObject() : expr(nullptr) {}
    };
    // private:
    /* register object specified by user */
    struct SYRegObject {
        std::string name;  // name of register
        uint indx;         // register index
        uint size;         // size in bytes
        bool is_signed; // @THX specify unsigned/signed
        bool has_seed;      //whether there's a seed for symbol
        KVExprPtr expr;    // point to a KVExprPtr
        //concrete value of symbol
        union {
            int64_t i64;
            int32_t i32;
            int16_t i16;
            int8_t i8;
            uint64_t u64;
            uint32_t u32;
            uint16_t u16;
            uint8_t u8;
        };
        SYRegObject() : expr(nullptr) {}
    };
    // All memory & regsiter objects specified by user
    std::map<ulong, SYMemObject *> m_SYMemObjects;
    std::map<ulong, SYRegObject *> m_SYRegObjects;

    std::shared_ptr<SYCPUState> m_CPU;
    std::shared_ptr<SYMemState> m_MEM;
 
    //haiqing
    public:
        std::shared_ptr<EFlagsManager> m_EFlagsMgr;
    /* / */

    public:
    VMState();
    ~VMState();

   public:
    // bool createSYMemObject(ulong addr, ulong size, const char *name = NULL);
    //pp-s
    //bool createSYMemObject(ulong addr, ulong size, bool isSigned, long conVal, const char *name = NULL);
    bool createSYMemObject(ulong addr, ulong size, bool isSigned, bool hasSeed, long conVal, const char *name = NULL);
    //pp-e
    void destroySYMemObject(ulong addr, ulong size);
    // bool createSYRegObject(uint index, uint size, const char *name = NULL);
    //pp-s
    //bool createSYRegObject(uint index, uint size, bool isSigned, long conVal, const char *name = NULL);
    bool createSYRegObject(uint index, uint size, bool isSigned, bool hasSeed, long conVal, const char *name = NULL);
    //pp-e
    void destroySYRegObject(uint index, uint size);

    // Set CPU state before running
    // static bool SetCPUState(VMState *VM, struct pt_regs *regs);
    static bool SetCPUState(VMState *VM, struct MacReg *regs);
    /* Jiaqi */
    // static bool ReadCPUState(VMState *VM, struct pt_regs *regs);
    static bool ReadCPUState(VMState *VM, struct MacReg *regs);
    ulong readConReg(uint idx);
    bool writeConReg(uint idx, ulong val);
    /* /Jiaqi */

    // Read & write registers
    bool isSYReg(uint reg_index);
    bool readRegister(RegValue &V);
    bool writeRegister(RegValue &V);

    struct pt_regs* getPTRegs(void);

    // Read & write memory cells
    bool isSYMemoryCell(ulong addr, ulong size);
    bool readMemory(MemValue &V);
    bool writeMemory(MemValue &V);

    // Read & write status flag bit
    bool getFlagBit(uint flag_index, FSInstrPtr &ptr);
    bool setFlagBit(uint flag_index, FSInstrPtr &ptr);

    bool clearAllSymFlag(void);
    
    //haiqing
    bool getFlagBit(uint flag_index, FLAG_STAT &flag);
    bool setFlagBit(uint flag_index, FLAG_STAT &flag);

    bool FlagBitDefinited(uint flag_index);

    bool SaveFlagChangingInstruction (FSInstrPtr &ptr);
    bool SaveFlagChangingInstructionExpr (entryID instrID, KVExprPtr exprPtr);

};

/**************************************************help functions*********************************/
bool ReadConOperand_RM(VMState *vm, OprndInfo *oi);
bool WriteConOPerand_RM(VMState *vm, OprndInfo *oi);

bool ReadSymOPerand_RM(VMState *vm, OprndInfoPtr &oi);
bool WriteSymOPerand_RM(VMState *vm, OprndInfoPtr &oi);

#endif  // _VMSTATE__H__
