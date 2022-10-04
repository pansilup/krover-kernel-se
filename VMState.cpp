
#include "VMState.h"

#include <signal.h>
#include <ucontext.h>

#include <functional>
#include <string>

#include "CPUState.h"
#include "CodeObject.h"
#include "Expr.h"
#include "Expression.h"
#include "InstructionDecoder.h"
#include "MemState.h"

// #include "VMState.h"
#include "EFlagsManager.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;

using namespace EXPR;

VMState::VMState() : m_SYMemObjects(), m_SYRegObjects() {
    m_CPU.reset(new SYCPUState());
    m_MEM.reset(new SYMemState());

    m_EFlagsMgr.reset(new EFlagsManager(this));
}

VMState::~VMState() {
    m_SYMemObjects.clear();
    m_SYRegObjects.clear();
}

/******************************** create symbolic objects ********************/
// bool VMState::createSYMemObject(ulong addr, ulong size, const char *name) {
//pp-s
//bool VMState::createSYMemObject(ulong addr, ulong size, bool isSigned, long conVal, const char *name) {
bool VMState::createSYMemObject(ulong addr, ulong size, bool isSigned, bool hasSeed, long conVal, const char *name) {
//pp-e
    SYMemObject *obj = new SYMemObject();
    // char szBuf[LINE_MAX];
    // sprintf(szBuf, "MEM_0x%lx_%ld", addr, size);
    // obj->name = szBuf;
    obj->name = name;
    obj->addr = addr;
    obj->size = size;
    obj->is_signed = isSigned;
    //pp-s
    obj->has_seed = hasSeed;
    //pp-e
    //setup concrete value of the symbol
    switch (size) {
        case 1: {
            obj->i8 = (char)conVal;
        } break;
        case 2: {
            obj->i16 = (short)conVal;
        } break;
        case 4: {
            obj->i32 = (int)conVal;
        } break;
        case 8: {
            obj->i64 = (long)conVal;
        } break;
        default: {
            cerr << "Unexpected length"
                 << "\n";
        } break;
    }
    //setup UdefExpr for the symbol
    obj->expr.reset(new UDefExpr(obj));

    ulong hash = HashTogether(addr, size);
    m_SYMemObjects[hash] = obj;

    MemValue V{addr, size, true, false, 0, obj->expr, NULL};
    return m_MEM->writeMemoryCell(V);
}

void VMState::destroySYMemObject(ulong addr, ulong size) {
    ulong hash = HashTogether(addr, size);
    auto it = m_SYMemObjects.find(hash);
    assert(it != m_SYMemObjects.end());
    SYMemObject *obj = it->second;
    delete obj;
    m_SYMemObjects.erase(it);

    // Concretize the memory cell to 0;
    MemValue V{addr, size, false, 0};
    m_MEM->writeMemoryCell(V);
}

// bool VMState::createSYRegObject(uint index, uint size, const char *name) {
//pp-s
//bool VMState::createSYRegObject(uint index, uint size, bool isSigned, long conVal, const char *name) {
bool VMState::createSYRegObject(uint index, uint size, bool isSigned, bool hasSeed, long conVal, const char *name) {
//pp-e
    SYRegObject *obj = new SYRegObject();
    char szBuf[LINE_MAX];
    sprintf(szBuf, "REG_0x%x_%d", index, size);
    obj->name = szBuf;
    obj->indx = index;
    obj->size = size;
    obj->is_signed = isSigned;
    obj->has_seed = hasSeed;
    //setup concrete value of the symbol
    switch (size) {
        case 1: {
            obj->i8 = (char)conVal;
        } break;
        case 2: {
            obj->i16 = (short)conVal;
        } break;
        case 4: {
            obj->i32 = (int)conVal;
        } break;
        case 8: {
            obj->i64 = (long)conVal;
        } break;
        default: {
            cerr << "Unexpected length"
                 << "\n";
        } break;
    }
    //setup UdefExpr for the symbol
    obj->expr.reset(new UDefExpr(NULL));

    ulong hash = HashTogether(index, size);
    m_SYRegObjects[hash] = obj;

    RegValue V{index, size, true, false, 0, obj->expr, NULL};
    return m_CPU->writeRegister(V);
}
void VMState::destroySYRegObject(uint index, uint size) {
    ulong hash = HashTogether(index, size);
    auto it = m_SYRegObjects.find(hash);
    assert(it != m_SYRegObjects.end());
    SYRegObject *obj = it->second;
    delete obj;
    m_SYRegObjects.erase(it);

    // Concretize the register to 0;
    RegValue V{index, size, false, 0};
    m_CPU->writeRegister(V);
}

// bool VMState::SetCPUState(VMState *VM, struct pt_regs *regs) {
bool VMState::SetCPUState(VMState *VM, struct MacReg *regs) {
    return VM->m_CPU->setConcreteCPUState(regs);
}

/* Jiaqi */
// bool VMState::ReadCPUState(VMState *VM, struct pt_regs *regs) {
bool VMState::ReadCPUState(VMState *VM, struct MacReg *regs) {
    return VM->m_CPU->readConcreteCPUState(regs);
}
    
ulong VMState::readConReg(uint idx)
{
    return m_CPU->readConReg(idx);
}

bool VMState::writeConReg(uint idx, ulong val)
{
    return m_CPU->writeConReg(idx, val);
}
/* /Jiaqi */

/******************************* Access registers ****************************/
// Read & write registers
bool VMState::isSYReg(uint reg_index) {
    return m_CPU->isSYReg(reg_index);
}

bool VMState::readRegister(RegValue &V) {
    return m_CPU->readRegister(V);
}

bool VMState::writeRegister(RegValue &V) {
    return m_CPU->writeRegister(V);
}

struct pt_regs* VMState::getPTRegs(void)
{
    return m_CPU->getPTRegs();
}
/******************************** Access memory ******************************/
// Read & write memory cells
bool VMState::isSYMemoryCell(ulong addr, ulong size) {
    return m_MEM->isSYMemoryCell(addr, size);
}

bool VMState::readMemory(MemValue &V) {
    return m_MEM->readMemoryCell(V);
}
bool VMState::writeMemory(MemValue &V) {
    return m_MEM->writeMemoryCell(V);
}

bool VMState::getFlagBit(uint flag_index, FSInstrPtr &ptr) {
    return m_CPU->getFlagBit(flag_index, ptr);
}
bool VMState::setFlagBit(uint flag_index, FSInstrPtr &ptr) {
    return m_CPU->setFlagBit(flag_index, ptr);
}

bool VMState::clearAllSymFlag() {
    return m_CPU->clearAllSymFlag();
}

//haiqing
bool VMState::getFlagBit(uint flag_index, FLAG_STAT &flag) {
    return m_CPU->getFlagBit(flag_index, flag);
}
bool VMState::setFlagBit(uint flag_index, FLAG_STAT &flag) {
    return m_CPU->setFlagBit(flag_index, flag);
}

bool VMState::FlagBitDefinited(uint flag_index) {
    return m_CPU->FlagBitDefinited(flag_index);
}


/***************************************help functions************************/
bool ReadConOperand_RM(VMState *vm, OprndInfo *oi) {
    bool res = false;
    switch (oi->opty) {
        case OPTY_REGCON: {
            RegValue V = {oi->reg_index, oi->size, false};
            res = vm->readRegister(V);
            oi->reg_conval = V.u64;
            assert(V.bsym == false);
        } break;
        case OPTY_MEMCELLCON: {
            MemValue V = {oi->mem_conaddr, oi->size, false};
            res = vm->readMemory(V);
            oi->mem_conval = V.u64;
            assert(V.bsym == false);
        } break;
        default: {
            FIX_ME();
            exit(EXIT_FAILURE);  // "Unexpected operand type"
        } break;
    }
    return res;
}

bool WriteConOPerand_RM(VMState *vm, OprndInfo *oi) {
    bool res = false;
    switch (oi->opty) {
        case OPTY_REGCON: {
            // Write to register
            RegValue V{oi->reg_index, oi->size, false, oi->reg_conval};
            res = vm->writeRegister(V);
        } break;
        case OPTY_MEMCELLCON: {
            // write to memory cell
            MemValue V{oi->mem_conaddr, oi->size, true, oi->mem_conval};
            res = vm->writeMemory(V);
        } break;
        default: {
            FIX_ME();
            exit(EXIT_FAILURE);  // "Unexpected operand type"
        } break;
    }
    return res;
}

// KVExprPtr
bool ReadSymOPerand_RM(VMState *vm, OprndInfoPtr &oi) {
    bool res;
    KVExprPtr val;
    switch (oi->opty) {
        case OPTY_REGSYM: {
            RegValue V = {oi->reg_index, oi->size, false};
            // V.expr = &val;
            vm->readRegister(V);
        } break;
        case OPTY_MEMCELLSYM: {
            // if (oi->mem_address == SYMMEMADDR) {
            //     FIX_ME();  // "Not supported"
            // } else {
            //     MemValue V = {oi->mem_address, oi->size, false};
            //     V.expr = &val;
            //     vm->readMemory(V);
            // }
        } break;
        default: {
            FIX_ME();
            exit(EXIT_FAILURE);  // "Unexpected operand type"
        } break;
    }
    return res;
}

bool WriteSymOPerand_RM(VMState *vm, OprndInfoPtr &oi) {
    // , KVExprPtr &e

    bool res = false;
    switch (oi->opty) {
        // case OPTY_REG: {
        //     // Write to register
        //     RegValue V = {oi->reg_index, oi->size, true};
        //     V.expr = &e;
        //     res = vm->writeRegister(V);
        // } break;
        // case OPTY_MEM: {
        //     // write to memory cell
        //     if (oi->mem_address == SYMMEMADDR) {
        //         FIX_ME();
        //         exit(EXIT_FAILURE);  // "Not supported"
        //     } else {
        //         MemValue V = {oi->mem_address, oi->size, true};
        //         V.expr = &e;
        //         res = vm->writeMemory(V);
        //     }
        // } break;
        default: {
            FIX_ME();
            exit(EXIT_FAILURE);  // "Unexpected operand type"
        } break;
    }
    return res;
}

//haiqing
bool VMState::SaveFlagChangingInstruction (FSInstrPtr &ptr) {

    return m_EFlagsMgr->SaveFlagChangingInstruction (ptr) ;
}
bool VMState::SaveFlagChangingInstructionExpr (entryID instrID, KVExprPtr exprPtr) {

    return m_EFlagsMgr->SaveFlagChangingInstructionExpr(instrID, exprPtr);
}
