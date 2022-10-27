#include "symexec.h"

#include <linux/types.h>
#include <signal.h>
#include <ucontext.h>

#include <iostream>

#include "BinaryFunction.h"
#include "CodeObject.h"
#include "Expr.h"
#include "InstructionDecoder.h"
#include "VMState.h"
#include "interface.h"
#include "thinctrl.h"
#include "SymList.h"

// #include "SymFlag.h"
//#define PRINT_INFO_BEFOR
//#define PRINT_INFO_AFTER


using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;

using namespace EXPR;

bool SymExecutor::pushInstr(InstrInfoPtr &I) {
    m_IOIs.push_back(I);
    return true;
}

bool SymExecutor::run(VMState *vm) {
    bool res = false;
    _run_prologue();

    for (auto IOI : m_IOIs) {
        auto &I = IOI->PI;
        auto &vecOI = IOI->vecOI;
        m_RIPUpdated = false;

        entryID id = I->getOperation().getID() ;
        std::cout << "ins operation :" << id << std::endl;
        
        switch (id) {
            case e_mov:
            case e_movbe:
            case e_movsl:
            case e_movabs:
            case e_movapd:
            case e_movaps:
            case e_movd:
            case e_movddup:
            case e_movdq2q:
            case e_movdqa:
            case e_movdqu:
            case e_movhpd:
            case e_movhps:
            case e_movhps_movlhps:
            case e_movlpd:
            case e_movlps:
            case e_movlps_movhlps:
            case e_movmskpd:
            case e_movmskps:
            case e_movntdq:
            case e_movntdqa:  // SSE 4.1
            case e_movnti:
            case e_movntpd:
            case e_movntps:
            case e_movntq:
            case e_movntsd:
            case e_movntss:
            case e_movq:
            case e_movq2dq:
            case e_movsb:
            case e_movsd:
            case e_movsd_sse:
            case e_movshdup:
            case e_movsldup:
            case e_movss:
            case e_movsw:
            case e_movupd:
            case e_movups: {
                process_mov(vm, IOI);
            } break;

            case e_movsx:
            case e_movslq:
            case e_movsxd: {
                process_movsx(vm, IOI);
            } break;

            case e_movzx:{
                process_movzx(vm, IOI);
            } break;

            
            case e_cbw:
            case e_cwde:
            case e_cwtl: {
                //std::cout << "calling process_cbw" << std::endl;
                process_cbw(vm, IOI);
            } break;

            // case e_cqo:
            case e_cdq:
            case e_cwd: {
                process_cdq(vm, IOI);
                break ;
            }

            case e_addpd:
            case e_addps:
            case e_addsd:
            case e_addss:
            case e_addsubpd:
            case e_addsubps: {
                assert(0);
                // asm("int3");
            } break ;

            case e_add: {
                process_add(vm, IOI);
            } break;
            
            case e_subpd:
            case e_subps:
            case e_subsd:
            case e_subss:            {
                // Process substraction
                assert(0);
                // asm("int3");
            } break;

            case e_sub: {
                // Process substraction
                process_sub(vm, IOI);
                break;
            } 
            
            case e_idiv: {
                process_idiv(vm, IOI) ;
                break ;
            }
            
            case e_mul:
            case e_imul:  {
                process_mul(vm, IOI) ;
                break ;
            }

            case e_mulpd:
            case e_mulps:
            case e_mulsd:
            case e_mulss: {
                assert(0);
                // asm("int3") ;
            } break;
            case e_div:
            case e_divpd:
            case e_divps:
            case e_divsd:
            case e_divss: {
                assert(0);
                // asm("int3");
            } break;

            case e_and: {
                process_and(vm, IOI) ;
                break ;
            }
            case e_andnpd:
            case e_andnps:
            case e_andpd:
            case e_andps: {
                //  Process logical and
                assert(0);
                // asm("int3");
            } break;
            case e_or: {
                process_or(vm, IOI);
                break; 
            }
            case e_orpd:
            case e_orps: {
                assert(0);
                // asm("int3");
            } break;
            case e_not: {
                process_not(vm, IOI);
                break; 
            } break;
            case e_neg: {
                process_neg(vm, IOI);
                break ;
            }
            case e_test: {
                process_test(vm, IOI);
            } break;

            case e_cmovbe:
            case e_cmove:
            case e_cmovnae:
            case e_cmovnb:  // or cmovae 
            case e_cmovnbe:
            case e_cmovne:
            case e_cmovng:  // or cmovle
            case e_cmovnge:
            case e_cmovnl:  // or cmovge
            // case e_cmovnle:
            case e_cmovno:
            case e_cmovns:
            case e_cmovo:
            case e_cmovpe:
            case e_cmovpo:
            case e_cmovs: {
                process_cmovxx(vm, IOI) ;
            }   break ;
	        case e_jbe:
	        case e_je:
	        case e_jge:
	        case e_jl:
	        case e_jle:
	        case e_jmp:
	        case e_jmpq:
	        case e_jne:
            case e_jz:
            case e_jnz:
	        case e_jns:
	        case e_js: {
                process_jcc(vm, IOI) ;
                break ;
            }
            case e_setb:
            case e_setbe:
            case e_setl:
            case e_setle:
            case e_setnb:
            case e_setnbe:
            case e_setnl:
            case e_setnle:
            case e_setno:
            case e_setnp:
            case e_setns:
            case e_setnz:
            case e_seto:
            case e_setp:
            case e_sets:
            case e_setz: {
                process_set(vm, IOI) ;
                break;
            }
            case e_cmp: 
            case e_cmpw: {
                process_cmp(vm, IOI) ;
                break ;
            }
            case e_xor :{
                process_xor(vm, IOI) ;
                break ;
            }
            case e_shl_sal: {
                process_shl_sal (vm, IOI) ;
                break ;
            }
            case e_shr: {
                process_shr (vm, IOI) ;
                break ;
            }
            case e_shrd:{
                process_shrd (vm, IOI) ;
                break ;
            }
            case e_sar:{
                process_sar (vm, IOI) ;
                break ;
            }
            case e_xchg: {
                process_xchg (vm, IOI) ;
                break ;
            }

            case e_pop:{
                process_pop (vm, IOI) ;
                break ;
            }

            case e_push:{
                process_push (vm, IOI) ;
                break ;
            }

            case e_lea:
                process_lea (vm, IOI) ;
                break ;

            case e_rdtsc: {
                // read real time clock ...
                break ;
            }

  
            default: {
                cout << "2802: instruction: " << I->format() << "\n";
                assert(0);
                // asm("int3");
            } break;
        }
        
        // if (!m_RIPUpdated) {
        //     // Update RIP
        //     RegValue V{(uint)x86_64::rip, 8, false};
        //     res = vm->readRegister(V);
        //     assert(res);
        //     V.u64 += I->size();
        //     res = vm->writeRegister(V);
        //     assert(res);
        // }
    }  // for (auto IOI : m_IOIs)
    _run_postlogue();
    return true;
}

bool SymExecutor::_run_prologue(void) {}

bool SymExecutor::_run_postlogue(void) {
    m_IOIs.clear();
    return true;
}
bool SymExecutor::process_lea(VMState *vm, InstrInfoPtr &infoptr) {
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oidst = vecOI[0];
    OprndInfoPtr &oisrc = vecOI[1];
    KVExprPtr e;
    bool res;
    long val;

    if(oisrc->symb) {
        // Do reading
        res = oisrc->getSymValue(e);
        assert(res);
        // Do writting
        res = oidst->setSymValue(vm, e);
        assert(res);
    } else {
        assert(oidst->symb) ;
        // Do reading
        res = oisrc->getConValue(val);
        assert(res);
        // Do writting
        res = oidst->setConValue(vm, val);
        assert(res);
    }

    return true ;
}

bool SymExecutor::process_jcc(VMState *vm, InstrInfoPtr &infoptr) {
    return true ;
    //return process_jmp(vm, infoptr);
}

bool SymExecutor::process_jmp(VMState *vm, InstrInfoPtr &infoptr) {
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc = vecOI[0];
    bool res;
    long val;

    // Do reading
    assert(!oisrc->symb);
    res = oisrc->getConValue(val);
    assert(res);

    RegValue V{(uint)x86_64::rip, 8, false, val};
    vm->writeRegister(V);
    m_RIPUpdated = true;
    return true;
}

bool SymExecutor::process_mov(VMState *vm, InstrInfoPtr &infoptr) {
    // Process move instruction
    Instruction *in = new Instruction(*infoptr->PI);
    InstrInfo *ioi = new InstrInfo(in);

    parseOperands(vm, ioi, true);

    auto &vecOI = ioi->vecOI;
    OprndInfoPtr &oidst = vecOI[0];
    OprndInfoPtr &oisrc = vecOI[1];
    SymCellPtr cellList;
    bool res;
    long val;

    if(oisrc->symb) {
        // Do reading
        res = oisrc->getSymValue (cellList, val) ;
        assert(res);
#ifdef _DEBUG_OUTPUT
        printCellList (cellList) ;
#endif
        // Do writting
        res = oidst->setSymValue(vm, cellList, val);
        
        assert(res);
    } else {
        assert(oidst->symb) ;
        // Do reading
        res = oisrc->getConValue(val);
        assert(res);
        // Do writting
        res = oidst->setConValue(vm, val);
        assert(res);
    }

    return true;
}

bool SymExecutor::process_add(VMState *vm, InstrInfoPtr &infoptr) {
    // Process addition
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    OprndInfoPtr &oidst = oisrc1;
    bool res;

    KVExprPtr oe ;

    if (oisrc1->symb && oisrc2->symb) {
        KVExprPtr e1(nullptr), e2(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);
        res = oisrc2->getSymValue(e2);
        assert(res);

        // Generate new expression
        oe.reset(new AddExpr(e1, e2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else if (oisrc1->symb && !oisrc2->symb) {
        KVExprPtr e1(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);

        long v2;
        res = oisrc2->getConValue(v2);
        assert(res);
        ExprPtr c2(new ConstExpr(v2, oisrc2->size, 0));
        oe.reset(new AddExpr(e1, c2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    }

    else if (!oisrc1->symb && oisrc2->symb) {
        KVExprPtr e2(nullptr);
        res = oisrc2->getSymValue(e2);
        assert(res);

        long v1;
        res = oisrc1->getConValue(v1);
        assert(res);
        ExprPtr c1(new ConstExpr(v1, oisrc1->size, 0));
        oe.reset(new AddExpr(c1, e2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else {
        ERRR_ME("Unexpected operands");
        assert(0);
        // asm("int3");
    }
    vm->SaveFlagChangingInstructionExpr(e_add, oe) ;
    return true ;
}

bool SymExecutor::process_test(VMState *vm, InstrInfoPtr &infoptr) {
    
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    OprndInfoPtr &oidst = oisrc1;
    bool res;

    KVExprPtr oe = NULL ;

    if (oisrc1->symb && oisrc2->symb) {
        KVExprPtr e1(nullptr), e2(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);
        res = oisrc2->getSymValue(e2);
        assert(res);

        // Generate new expression
        oe.reset(new AndExpr(e1, e2));

    } else if (oisrc1->symb && !oisrc2->symb) {
        KVExprPtr e1(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);

        long v2;
        res = oisrc2->getConValue(v2);
        assert(res);
        ExprPtr c2(new ConstExpr(v2, oisrc2->size, 0));
        oe.reset(new AndExpr(e1, c2));
    }

    else if (!oisrc1->symb && oisrc2->symb) {
        KVExprPtr e2(nullptr);
        res = oisrc2->getSymValue(e2);
        assert(res);

        long v1;
        res = oisrc1->getConValue(v1);
        assert(res);
        ExprPtr c1(new ConstExpr(v1, oisrc1->size, 0));
        oe.reset(new AndExpr(c1, e2));
    } else {
        ERRR_ME("Unexpected operands");
        assert(0);
        // asm("int3");
    }
    vm->SaveFlagChangingInstructionExpr(e_test, oe) ;
    return true;   
}


// QHQ
bool SymExecutor::process_cmovxx(VMState *vm, InstrInfoPtr &infoptr) {
    // Process conditional move instruction
    bool domov = true ;

    auto &I = infoptr->PI;

    if (domov){
        auto &vecOI = infoptr->vecOI;
        OprndInfoPtr &oidst = vecOI[0];
        OprndInfoPtr &oisrc = vecOI[1];
        KVExprPtr e;
        bool res;
        long val;

        if(oisrc->symb) {
            // Do reading
            res = oisrc->getSymValue(e);
            assert(res);
            // Do writting
            res = oidst->setSymValue(vm, e);
            assert(res);
        } else {
            // assert(oidst->symb) ;
            // Do reading
            res = oisrc->getConValue(val);
            assert(res);
            // Do writting
            res = oidst->setConValue(vm, val);
            assert(res);
        }
    }

    return true;
}

bool SymExecutor::process_jxx(VMState *vm, InstrInfoPtr &infoptr) {

    std::cout << "jump instructions: JMP/Jcc" << "\n" ;
    assert(0) ;
    return true;
}

// QHQ  
bool SymExecutor::process_cmp(VMState *vm, InstrInfoPtr &infoptr) {
        auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    OprndInfoPtr &oidst = oisrc1;
    KVExprPtr oe = NULL;
    bool res;

    if (oisrc1->symb && oisrc2->symb) {
        KVExprPtr e1(nullptr), e2(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);
        res = oisrc2->getSymValue(e2);
        assert(res);

        // Generate new expression
        oe.reset(new SubExpr(e1, e2));

    } else if (oisrc1->symb && !oisrc2->symb) {
        KVExprPtr e1(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);

        long v2;
        res = oisrc2->getConValue(v2);
        assert(res);
        //pp-s
        //ExprPtr c2(new ConstExpr(v2, oisrc2->size, 0));
        //the size of the constant expression set by dyninst(oisrc2->size) is wrong
        //therefore, we use the size of the symbolic operand(oisrc1->size) to generate the constant expression
        ExprPtr c2(new ConstExpr(v2, oisrc1->size, 0)); 
        //std::cout << "--------------oprnd sz:" << oisrc2->size << std::endl;
        //pp-e
        oe.reset(new SubExpr(e1, c2));
    }

    else if (!oisrc1->symb && oisrc2->symb) {
        KVExprPtr e2(nullptr);
        res = oisrc2->getSymValue(e2);
        assert(res);

        long v1;
        res = oisrc1->getConValue(v1);
        assert(res);
        //pp-s
        //ExprPtr c1(new ConstExpr(v1,oisrc1->size, 0));
        //the size of the constant expression set by dyninst(oisrc1->size) is wrong
        //therefore, we use the size of the symbolic operand(oisrc2->size) to generate the constant expression
        ExprPtr c1(new ConstExpr(v1,oisrc2->size, 0));
        //std::cout << "--------------oprnd sz:" << oisrc1->size << std::endl;
        //pp-e
        oe.reset(new SubExpr(c1, e2));
    } else {
        ERRR_ME("Unexpected operands");
        assert(0);
        // asm("int3");
    }
    res = vm->SaveFlagChangingInstructionExpr(e_cmp, oe) ;
    assert (res) ;
    return true;   
}

bool SymExecutor::process_sub(VMState *vm, InstrInfoPtr &infoptr) {
    // Process sub
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    OprndInfoPtr &oidst = oisrc1;
    KVExprPtr oe = NULL;
    bool res;

    if (oisrc1->symb && oisrc2->symb) {
        KVExprPtr e1(nullptr), e2(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);
        res = oisrc2->getSymValue(e2);
        assert(res);

        // Generate new expression
        oe.reset(new SubExpr(e1, e2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else if (oisrc1->symb && !oisrc2->symb) {
        KVExprPtr e1(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);

        long v2;
        res = oisrc2->getConValue(v2);
        assert(res);
        ExprPtr c2(new ConstExpr(v2, oisrc2->size, 0));
        oe.reset(new SubExpr(e1, c2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    }

    else if (!oisrc1->symb && oisrc2->symb) {
        KVExprPtr e2(nullptr);
        res = oisrc2->getSymValue(e2);
        assert(res);

        long v1;
        res = oisrc1->getConValue(v1);
        assert(res);
        ExprPtr c1(new ConstExpr(v1, oisrc1->size, 0));
        oe.reset(new SubExpr(c1, e2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else {
        ERRR_ME("Unexpected operands");
        assert(0);
        // asm("int3");
    }
    res = vm->SaveFlagChangingInstructionExpr(e_sub, oe) ;
    assert (res) ;
    return true ;

}
bool SymExecutor::process_and(VMState *vm, InstrInfoPtr &infoptr) {
    
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    OprndInfoPtr &oidst = oisrc1;
    KVExprPtr oe = NULL ;
    bool res;

    if (oisrc1->symb && oisrc2->symb) {
        KVExprPtr e1(nullptr), e2(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);
        res = oisrc2->getSymValue(e2);
        assert(res);

        // Generate new expression
        oe.reset(new AndExpr(e1, e2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else if (oisrc1->symb && !oisrc2->symb) {
        KVExprPtr e1(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);

        long v2;
        res = oisrc2->getConValue(v2);
        assert(res);
        ExprPtr c2(new ConstExpr(v2, oisrc2->size, 0));
        oe.reset(new AndExpr(e1, c2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    }

    else if (!oisrc1->symb && oisrc2->symb) {
        KVExprPtr e2(nullptr);
        res = oisrc2->getSymValue(e2);
        assert(res);

        long v1;
        res = oisrc1->getConValue(v1);
        assert(res);
        ExprPtr c1(new ConstExpr(v1, oisrc1->size, 0));
        oe.reset(new AndExpr(c1, e2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else {
        ERRR_ME("Unexpected operands");
        assert(0);
        // asm("int3");
    }
    res = vm->SaveFlagChangingInstructionExpr(e_and, oe) ;
    assert (res) ;

    return true;   
}
bool SymExecutor::process_or(VMState *vm, InstrInfoPtr &infoptr) {
    
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    OprndInfoPtr &oidst = oisrc1;
    KVExprPtr oe = NULL ;
    bool res;

    if (oisrc1->symb && oisrc2->symb) {
        KVExprPtr e1(nullptr), e2(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);
        res = oisrc2->getSymValue(e2);
        assert(res);

        // Generate new expression
        oe.reset(new OrExpr(e1, e2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else if (oisrc1->symb && !oisrc2->symb) {
        KVExprPtr e1(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);

        long v2;
        res = oisrc2->getConValue(v2);
        assert(res);
        ExprPtr c2(new ConstExpr(v2, oisrc2->size, 0));
        oe.reset(new OrExpr(e1, c2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    }

    else if (!oisrc1->symb && oisrc2->symb) {
        KVExprPtr e2(nullptr);
        res = oisrc2->getSymValue(e2);
        assert(res);

        long v1;
        res = oisrc1->getConValue(v1);
        assert(res);
        ExprPtr c1(new ConstExpr(v1, oisrc1->size, 0));
        oe.reset(new OrExpr(c1, e2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else {
        ERRR_ME("Unexpected operands");
        assert(0);
        // asm("int3");
    }

    res = vm->SaveFlagChangingInstructionExpr(e_or, oe) ;
    assert (res) ;
    return true;   
}

bool SymExecutor::process_xor(VMState *vm, InstrInfoPtr &infoptr) {
    
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    OprndInfoPtr &oidst = oisrc1;
    KVExprPtr oe = NULL ;
    bool res;

    if (oisrc1->symb && oisrc2->symb) {
        KVExprPtr e1(nullptr), e2(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);
        res = oisrc2->getSymValue(e2);
        assert(res);

        // Generate new expression
        oe.reset(new XorExpr(e1, e2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else if (oisrc1->symb && !oisrc2->symb) {
        KVExprPtr e1(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);

        long v2;
        res = oisrc2->getConValue(v2);
        assert(res);
        ExprPtr c2(new ConstExpr(v2, oisrc2->size, 0));
        oe.reset(new XorExpr(e1, c2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    }

    else if (!oisrc1->symb && oisrc2->symb) {
        KVExprPtr e2(nullptr);
        res = oisrc2->getSymValue(e2);
        assert(res);

        long v1;
        res = oisrc1->getConValue(v1);
        assert(res);
        ExprPtr c1(new ConstExpr(v1, oisrc1->size, 0));
        oe.reset(new XorExpr(c1, e2));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else {
        ERRR_ME("Unexpected operands");
        assert(0);
        // asm("int3");
    }

    res = vm->SaveFlagChangingInstructionExpr(e_xor, oe) ;
    assert (res) ;

    return true;   
}

bool SymExecutor::process_shl_sal(VMState *vm, InstrInfoPtr &infoptr) {
    
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    OprndInfoPtr &oidst = oisrc1;
    bool res;

    assert (oisrc1->symb) ;
    assert (!oisrc2->symb) ;

    KVExprPtr e1(nullptr);
    res = oisrc1->getSymValue(e1);
    assert(res);

    long v2;
    res = oisrc2->getConValue(v2);
    assert(res);
    ExprPtr c2(new ConstExpr(v2, oisrc2->size, 0));
    KVExprPtr oe(new Shl_SalExpr(e1, c2));
    res = oidst->setSymValue(vm, oe);

    assert(res);
    
    res = vm->SaveFlagChangingInstructionExpr(e_shl_sal, oe) ;
    assert (res) ;
    return true;   
}

bool SymExecutor::process_shr(VMState *vm, InstrInfoPtr &infoptr) {
    
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    OprndInfoPtr &oidst = oisrc1;
    bool res;

    assert (oisrc1->symb) ;
    assert (!oisrc2->symb) ;

    KVExprPtr e1(nullptr);
    res = oisrc1->getSymValue(e1);
    assert(res);

    long v2;
    res = oisrc1->getConValue(v2);
    assert(res);
    ExprPtr c2(new ConstExpr(v2, oisrc2->size, 0));
    KVExprPtr oe(new ShrExpr(e1, c2));
    res = oidst->setSymValue(vm, oe);
    assert(res);

    res = vm->SaveFlagChangingInstructionExpr(e_shr, oe) ;
    assert (res) ;

    return true;   
}

bool SymExecutor::process_sar(VMState *vm, InstrInfoPtr &infoptr) {
    
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    OprndInfoPtr &oidst = oisrc1;
    bool res;

    assert (oisrc1->symb) ;
    assert (!oisrc2->symb) ;

    KVExprPtr e1(nullptr);
    res = oisrc1->getSymValue(e1);
    assert(res);

    long v2;
    res = oisrc1->getConValue(v2);
    assert(res);
    ExprPtr c2(new ConstExpr(v2, oisrc2->size, 0));
    KVExprPtr oe(new SarExpr(e1, c2));
    res = oidst->setSymValue(vm, oe);
    assert(res);

    res = vm->SaveFlagChangingInstructionExpr(e_sar, oe) ;
    assert (res) ;
    return true;   
}

bool SymExecutor::process_idiv(VMState *vm, InstrInfoPtr &infoptr) {
    // Process addition
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &o_d = vecOI[0]; 
    OprndInfoPtr &o_a = vecOI[1];
    OprndInfoPtr &o_Divisor = vecOI[2];
    bool res;
    KVExprPtr e(nullptr);
    KVExprPtr e_a(nullptr), e_d(nullptr), e_Dividend(nullptr), e_Divisor(nullptr);
    long v_a, v_d, v_Dividend, v_Divisor ;

    if (o_d->symb || o_a->symb) {
        if(o_d->symb) {
            res = o_d->getSymValue(e_d) ;
            assert (res) ;
        }
        else {
            res = o_d->getConValue(v_d) ;
            assert (res) ;
            e_d.reset(new ConstExpr(v_d, o_d->size, 0)) ;
        }
         if(o_a->symb) {
            res = o_a->getSymValue(e_a) ;
            assert (res) ;
         }
        else {
            res = o_a->getConValue(v_a) ;
            assert (res) ;
            e_a.reset(new ConstExpr(v_a, o_a->size, 0)) ;
        }
    } else {
        // dx:ax both not symbol, we may need a 128bits int.
        res = o_d->getConValue(v_d) ;
        assert (res) ;
        e_d.reset(new ConstExpr(v_d, o_d->size, 0)) ;
        
        res = o_a->getConValue(v_a) ;
        assert (res) ;
        e_a.reset(new ConstExpr(v_a, o_a->size, 0)) ;
    }

    e_Dividend.reset(new CombineExpr(e_d, e_a, o_d->size, o_a->size, o_d->size + o_a->size, 0)) ;

    if (o_Divisor->symb) {
        res = o_Divisor->getSymValue(e_Divisor) ;
        assert (res) ;
    } else {
        res = o_Divisor->getConValue(v_Divisor) ;
        assert (res) ;
        e_Divisor.reset(new ConstExpr(v_Divisor, o_Divisor->size, 0)) ;
    }

    KVExprPtr e_Quotient(new iDivExpr(e_Dividend, e_Divisor, o_Divisor->size, 0));
    KVExprPtr e_Remainder(new iDivExpr(e_Dividend, e_Divisor, o_Divisor->size, 0));

    res = o_a->setSymValue(vm, e_Quotient) ;
    assert (res) ;
    res = o_d->setSymValue(vm, e_Remainder) ;
    assert (res) ;
    
    return true ;
}
__uint128_t SignedTOUnsigned (long v, int size) {
    switch (size) {
        case 1:
            return (__uint128_t)(uint8_t)(v) ;
        case 2:
            return (__uint128_t)(uint16_t)(v) ;
        case 4:
            return (__uint128_t)(uint32_t)(v) ;
        case 8:
            return (__uint128_t)(uint64_t)(v) ;
        default :
            assert (0) ;
    }
}
bool SymExecutor::process_mul(VMState *vm, InstrInfoPtr &infoptr) {
    // Process addition
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &o_d = vecOI[0];        // dx
    OprndInfoPtr &o_a = vecOI[1];        // ax
    OprndInfoPtr &o_m = vecOI[2];        // oper
    KVExprPtr e_d, e_a, e_m ;
    long v_d, v_a, v_m ;
    bool res;
    __uint128_t v_r=0, u_a, u_m ;

    if(o_a->symb) {
        o_a->getSymValue(e_a) ;
        assert (res) ;
    }
    else {
        res = o_a->getConValue(v_a) ;
        assert (res) ;
        e_a.reset(new ConstExpr(v_a, o_a->size, 0)) ;
    }
    if(o_m->symb) {
        res = o_m->getSymValue(e_m) ;
        assert (res) ;
    }
    else {
        o_m->getConValue(v_m) ;
        assert (res) ;
        e_m.reset(new ConstExpr(v_m, o_m->size, 0)) ;
    }

    if(!o_a->symb && !o_m->symb) {
        auto &I = infoptr->PI;    
        entryID id = I->getOperation().getID() ;
        
        if (id == e_mul) {
            u_a = SignedTOUnsigned (v_a, o_a->size) ;
            u_m = SignedTOUnsigned (v_m, o_m->size) ;
            v_r = u_a * u_m ;
        } else {
            v_r = v_a * v_m ;
        }
        uint64_t vrl = (uint64_t)(v_r & ((((__uint128_t)1)<<o_a->size*8)-1)) ;
        uint64_t vrh = (uint64_t)((v_r>>(o_d->size*8)) & ((((__uint128_t)1)<<o_d->size*8)-1)) ;
        res = o_a->setConValue(vm, vrl) ;
        assert (res) ;
        res = o_d->setConValue(vm, vrh) ;
        assert (res) ;

        return true ;
    }

    KVExprPtr e_r(new MulExpr(e_a, e_m, o_m->size, 0)) ;
    e_a.reset(new ExtractExpr(e_r, 0, o_a->size, o_a->size, 0)) ;
    e_d.reset(new ExtractExpr(e_r, o_a->size, o_a->size*2, o_a->size, 0)) ;

    e_a->print () ;
    std::cout << std::endl ;
    e_d->print () ;
    std::cout << std::endl ;
    
    res = o_a->setSymValue(vm, e_a) ;
    assert (res) ;
    res = o_d->setSymValue(vm, e_d) ;
    assert (res) ;

    return true ;
}

bool SymExecutor::process_not(VMState *vm, InstrInfoPtr &infoptr) {
    // Process addition
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oidst = oisrc1;
    KVExprPtr oe = NULL ;
    bool res;

    if (oisrc1->symb) {
        KVExprPtr e1(nullptr) ;
        res = oisrc1->getSymValue(e1);
        assert(res);
        // Generate new expression
        oe.reset(new NotExpr(e1));
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else {
        ERRR_ME("Unexpected operands");
        assert(0);
        // asm("int3");
    }
    res = vm->SaveFlagChangingInstructionExpr(e_not, oe) ;
    assert (res) ;
    return true ;
}

bool SymExecutor::process_neg(VMState *vm, InstrInfoPtr &infoptr) {
    // Process addition
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oidst = oisrc1;
    bool res;

    assert (oisrc1->symb) ;

    KVExprPtr e1(nullptr) ;
    res = oisrc1->getSymValue(e1);
    assert(res);
    // Generate new expression
    KVExprPtr oe(new NegExpr(e1));
    res = oidst->setSymValue(vm, oe);
    assert (res) ;

    res = vm->SaveFlagChangingInstructionExpr(e_neg, oe) ;
    
    assert(res);
    return true ;
}

bool SymExecutor::process_pop(VMState *vm, InstrInfoPtr &infoptr) {
    Instruction *in = new Instruction(*infoptr->PI);
    InstrInfo *ioi = new InstrInfo(in);

    parseOperands(vm, ioi, true);

    auto &vecOI = ioi->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];   // rsp
    bool res;
    RegValue V ;
     
    assert(!oisrc2->symb) ;

    V.indx = oisrc2->reg_index;
    V.size = oisrc2->size ;
    V.isSymList = false ;

    res = vm->readRegister(V);
    assert (res) ;
    assert (V.size==8) ;

// debug log 20220512
#ifdef _DEBUG_OUTPUT
    std::cout << "pop :" << std::hex << V.u64 << " " ;
#endif
// debug log 20220512

    MemValue MV;
    MV.addr = V.u64 ;
    MV.size = oisrc1->size ;
    MV.isSymList = true;
    res = vm->readMemory(MV) ;
    assert(res) ;
    
    if(MV.bsym) {
        res = oisrc1->setSymValue(vm, MV.symcellPtr, MV.i64) ;
    }
    else {
        res = oisrc1->setConValue(vm, MV.u64) ;
// debug log 20220512
#ifdef _DEBUG_OUTPUT
        std::cout << std::hex << "0x" << MV.u64 << std::endl ;
#endif
// debug log 20220512         
    }

    assert(res) ;
    
    V.u64 += V.size ;
    res = vm->writeRegister(V);
    assert(res) ;

    return true;
}

bool SymExecutor::process_push(VMState *vm, InstrInfoPtr &infoptr) {
    Instruction *in = new Instruction(*infoptr->PI);
    InstrInfo *ioi = new InstrInfo(in);

    parseOperands(vm, ioi, true);

    auto &vecOI = ioi->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];    // rsp
    bool res;
    RegValue V ;
     
//    assert(oisrc1->symb) ;
    assert(!oisrc2->symb) ;

    V.indx = oisrc2->reg_index;
    V.size = oisrc2->size ;
    V.isSymList = false ;

    res = vm->readRegister(V);
    assert(res) ;
    assert (V.size==8) ;
    V.u64 -= V.size ;
    res = vm->writeRegister(V);
    assert(res) ;

// debug log 20220512
#ifdef _DEBUG_OUTPUT
    std::cout << "push :" << std::hex << V.u64 << " " ;
#endif
// debug log 20220512

    MemValue MV;
    MV.addr = V.u64 ;
    MV.size = oisrc1->size ;
    MV.isSymList = false ;

    SymCellPtr cellList ;
    long v ;
    if(oisrc1->symb) {
        res = oisrc1->getSymValue(cellList, v);
        assert(res) ;
        MV.bsym = true ;
        MV.symcellPtr = cellList ;
        MV.isSymList = true ;
        MV.i64 = (uint64_t) v ;
    } else {
        long v ;
        res = oisrc1->getConValue(v);
        assert(res) ;
        MV.bsym = false ;
        MV.i64 = v ;
        MV.isSymList = false ;
    }

    res = vm->writeMemory(MV) ;
    assert(res) ;

    return true;
}


bool SymExecutor::process_xchg(VMState *vm, InstrInfoPtr &infoptr) {
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc1 = vecOI[0];
    OprndInfoPtr &oisrc2 = vecOI[1];
    bool res;

    if (oisrc1->symb && oisrc2->symb) {
        KVExprPtr e1(nullptr), e2(nullptr);
        res = oisrc1->getSymValue(e1);
        assert(res);
        res = oisrc2->getSymValue(e2);
        assert(res);

        res = oisrc1->setSymValue(vm, e2);
        assert(res);
        res = oisrc2->setSymValue(vm, e1);
        assert(res);

    } else if (oisrc1->symb && !oisrc2->symb) {
        KVExprPtr e1(nullptr);
        long v2;

        res = oisrc1->getSymValue(e1);
        assert(res);
        res = oisrc2->getConValue(v2);
        assert(res);

        res = oisrc1->setConValue(vm, v2);
        assert(res);
        res = oisrc2->setSymValue(vm, e1);
        assert(res);

    } else if (!oisrc1->symb && oisrc2->symb) {
        KVExprPtr e2(nullptr);
        long v1;
 
        res = oisrc2->getSymValue(e2);
        assert(res);
        res = oisrc1->getConValue(v1);
        assert(res);
 
        res = oisrc1->setSymValue(vm, e2);
        assert(res);
        res = oisrc2->setConValue(vm, v1);
        assert(res);
    } else {
        ERRR_ME("Unexpected operands");
        assert(0);
        // asm("int3");
    }
    // vm->SaveFlagChangingInstructionExpr(e_xchg, oe) ;
    return true;
}

bool SymExecutor::process_movsx(VMState *vm, InstrInfoPtr &infoptr) {
    // Process move instruction
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oidst = vecOI[0];
    OprndInfoPtr &oisrc = vecOI[1];
    bool res;

    if(oisrc->symb) {
        KVExprPtr e;
        // Do reading
        res = oisrc->getSymValue(e);
        assert(res);

        KVExprPtr oe(new SignExtExpr(e));

        // Do writting
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else {
        long val;
        assert  (oidst->symb) ;
        // Do reading
        res = oisrc->getConValue(val);
        assert(res);

        // Do writting
        res = oidst->setConValue(vm, val);
        assert(res);
    }

    return true;
}

bool SymExecutor::process_movzx(VMState *vm, InstrInfoPtr &infoptr) {
    // Process move instruction
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oidst = vecOI[0];
    OprndInfoPtr &oisrc = vecOI[1];
    bool res;

    if(oisrc->symb) {
        KVExprPtr e;
        // Do reading
        res = oisrc->getSymValue(e);
        assert(res);

        KVExprPtr oe(new ZeroExtExpr(e));

        // Do writting
        res = oidst->setSymValue(vm, oe);
        assert(res);
    } else {
        long val;
        assert  (oidst->symb) ;
        // Do reading
        res = oisrc->getConValue(val);
        assert(res);

        // Do writting
        res = oidst->setConValue(vm, val);
        assert(res);
    }

    return true;
}

//pp-s
/*
bool SymExecutor::process_cbw(VMState *vm, InstrInfoPtr &infoptr) {
    // eax sign extend to rax like
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc = vecOI[0];
    KVExprPtr e, oe ;
    long val;
    RegValue rv ;
    bool res ;    
    if (oisrc->symb) {
        res = oisrc->getSymValue(e) ;
        assert (res) ;
    } else {
        res = oisrc->getConValue(val) ;
        assert (res) ;
    }

    rv.bsym = oisrc->symb ;
    rv.size = oisrc->size *2 ;

    switch (oisrc->size) {
        case 1:
            rv.indx = x86_64::ax ;
            break ;
        case 2:
            rv.indx = x86_64::eax ;
            break ;
        case 4:
            rv.indx = x86_64::rax ;
            break ;
        default :
        // WTF?
            assert (0) ;
    }
    
    if(rv.bsym) {
        oe.reset(new SignExpr(e, rv.size, 0)) ;
        rv.expr = oe ;
    } else {
        rv.i64 = val ;
    }

    res = vm->writeRegister(rv) ;
    assert (res) ;

    return true;
}
*/
#if 1
bool SymExecutor::process_cbw(VMState *vm, InstrInfoPtr &infoptr) {
    // eax sign extend to rax like
    std::cout << "at process_cbw" << std::endl;
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc = vecOI[0];
    KVExprPtr e, oe ;
    RegValue rv, rvdest ;
    bool res ;    

    assert (oisrc->symb) ;
    //std::cout << "source reg:" << oisrc->reg_index << ", source reg size:" << oisrc->size << std::endl;
    res = oisrc->getSymValue(e) ;
    assert (res) ;
    rv.isSymList = false ;
    
    switch (oisrc->size) {
        case 2:
            rv.indx = x86_64::al ;
            rv.size = 1 ;
            break ;
        case 4:
            rv.indx = x86_64::ax ;
            rv.size = 2 ;
            break ;
        case 8:
            std::cout << "reg size 4" << std::endl;
            rv.indx = x86_64::eax ;
            rv.size = 4 ;
            break ;
        default :
        // WTF?
            assert (0) ;
    }
    res = vm->readRegister(rv) ;
    assert (res) ;

    if(rv.bsym)
    {
        oe.reset(new SignExtExpr(e, oisrc->size, 0)) ;
        res = oisrc->setSymValue(vm, oe) ;
        assert (res) ;
    }
    else
    {         
        rvdest.bsym = false ;
        rvdest.isSymList = false;
        rvdest.size = oisrc->size;

        switch (oisrc->size) {
        case 2:
            rvdest.indx = x86_64::ax ;
            rvdest.i16 = (rv.i8 & 0x80) ? (0xff00 | rv.i8) : rv.i8;
            break ;
        case 4:
            rvdest.indx = x86_64::eax ;
            rvdest.i32 = (rv.i16 & 0x8000) ? (0xffff0000 | rv.i16) : rv.i16;
            break ;
        case 8:
            rvdest.indx = x86_64::rax ;
            //std::cout << "bfr extnd i64 : " << std::hex << rvdest.i64 << " ,i32 : " << rv.i32 << std::endl;
            rvdest.i64 = (rv.i32 & 0x80000000) ? (0xffffffff00000000 | rv.i32) : rv.i32;
            //std::cout << "aft extnd i64 : " << std::hex << rvdest.i64 << " ,i32 : " << rv.i32 << std::endl;
            break ;
        default :
            assert (0) ;
        }
        res = vm->writeRegister(rvdest);
        assert(res);
    }

    return true;
}
#else if 0
bool SymExecutor::process_cbw(VMState *vm, InstrInfoPtr &infoptr) {
    // eax sign extend to rax like
    std::cout << "at process_cbw" << std::endl;
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oisrc = vecOI[0];
    KVExprPtr e, oe ;
    bool res ;    

    assert (oisrc->symb) ;

    std::cout << "source reg:" << oisrc->reg_index << ", source reg size:" << oisrc->size << std::endl;
    
    res = oisrc->getSymValue(e) ;
    assert (res) ;

    std::cout << "source :" ;
    e->print() ;
    std::cout << std::endl ;

    oe.reset(new ExtractExpr(e, 0, oisrc->size/2, oisrc->size/2, 0)) ;

    std::cout << "ExtractExpr expr:" ;
    oe->print() ;
    std::cout << std::endl ;

    oe.reset(new SignExtExpr(oe, oisrc->size, 0)) ;

    std::cout << "SignExtExpr expr:" ;
    oe->print() ;
    std::cout << std::endl ;

    res = oisrc->setSymValue(vm, oe) ;
    assert (res) ;


    return true;
}
#endif
//pp-e

bool SymExecutor::process_cdq(VMState *vm, InstrInfoPtr &infoptr) {
    // eax sign extend to edx::eax like
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &o_d = vecOI[0];       // dx
    OprndInfoPtr &o_a = vecOI[1];       // ax
    KVExprPtr e_a, e_r, e_d;
    RegValue rv_d ;
    long v_a;
    bool res;

    if (o_a->symb) {
        res = o_a->getSymValue (e_a) ;
        assert (res) ;
        e_r.reset (new SignExtExpr(e_a, o_a->size*2, 0)) ;
        e_d.reset (new ExtractExpr(e_r, o_a->size, o_a->size*2, o_d->size, 0)) ;
        e_d->print() ;
        std::cout << std::endl ;
        res = o_d->setSymValue(vm, e_d) ;
        assert (res) ;
        return true ;
    } else {
        res = o_a->getConValue (v_a) ;
        assert (res) ;
        if (v_a>=0) 
            v_a = 0 ;
        else
            v_a = -1 ;
        res = o_d->setConValue(vm, v_a) ;
        assert (res) ;
        return true ;
    }
    return true;
}
bool SymExecutor::process_set(VMState *vm, InstrInfoPtr &infoptr) {
    
    auto &vecOI = infoptr->vecOI;
    OprndInfoPtr &oidst = vecOI[0];
    long v = 1 ;
    bool res ;

    res = oidst->setConValue(vm, v) ;
    assert (res) ;
    
    return true ;
}

bool SymExecutor::process_shrd(VMState *vm, InstrInfoPtr &infoptr) {
    assert (0) ;
    return true ;
}

bool SymExecutor::Print_Inst(VMState *vm, InstrInfoPtr &infoptr, const char* cstr) {
    auto &vecOI = infoptr->vecOI ;
    int i = 0;
    DAPIInstrPtr &I = infoptr->PI;    
    // entryID id = I->getOperation().getID() ;

    std::cout << cstr << I->format() << std::endl ;
#if 0
    for(i=0; i<vecOI.size(); i++) {
        OprndInfoPtr &o=vecOI[i] ;
        KVExprPtr e ;
        long v ;
        if(o->symb) {
            o->getSymValue (e) ;
            e->print() ;
            std::cout << "," ;
        } else {
            if((o->rdwr & OPAC_RD) != 0){
                o->getConValue(v) ;
                std::cout << std::hex << v << ", " ;
            } else {
                std::cout << "----, " ;
            }
        }
    }
#endif
    std::cout << endl ;
    return true ;   
}

// ulong SymExecutor::isUseGS(VMState* vm, Instruction* in)
ulong SymExecutor::isUseGS(VMState* vm, DAPIInstrPtr& I)
{
    /* check if Insn uses gs as base in mem access, if yes, get gsbase first */
    // std::set<RegisterAST::Ptr> regrd = in->getOperation().implicitReads();
    std::set<RegisterAST::Ptr> regrd = I.get()->getOperation().implicitReads();
    if (regrd.size() != 0)
    {
        for (auto it : regrd)
        {
            if (it->getID() == x86_64::gs)
            {
                RegValue RV{it->getID(), 8};
                // bool ret = m_VM->readRegister(RV);
                bool ret = vm->readRegister(RV);
                assert(ret);
                return RV.u64;
            }
        }
    }
    return 0;
}


bool SymExecutor::_parseOperand_XX(VMState *vm, DAPIInstrPtr& I, OprndInfoPtr &oi) {
    bool res = false;  // Failed to parse the operand;
    DIAPIOperandPtr &O = oi->PO;
    if (O->isRead()) {
        std::set<RegisterAST::Ptr> rdwrRegs;
        oi->rdwr = OPAC_RD;

        O->getReadSet(rdwrRegs);
        if (rdwrRegs.size() == 0) {
            // Read immediate operand:
            // eg1: mov $0x0,0xfffffff4(%rbp) -> $0x0
            oi->opty = OPTY_IMM;
            auto RS = O->getValue()->eval();
            assert(RS.defined);
            oi->imm_value = RS.convert<ulong>();
            return true;
        } else {
            // Read a register operand or RIP-relative instruction:
            // eg3: mov %rax,0xfffffff8(%rbp) -> %rax
            // eg4: jmp 0xb(%rip) -> 0xb(%rip)
            // cout << O->format(Arch_x86_64) << endl;
            oi->opty = OPTY_REG;
            assert(rdwrRegs.size() == 1);
            auto R = *rdwrRegs.begin();
            oi->reg_index = R->getID();

            RegValue RV{oi->reg_index, (uint)R->size()};
            RV.isSymList = true;
            res = vm->readRegister(RV);
            assert(res);
            if (RV.bsym) {
                oi->opty = OPTY_REGSYM;
                oi->symb = true;
                oi->isSymList = true;
                auto V = O->getValue();
                std::vector<Expression::Ptr> exps;
                V->getChildren(exps);
                if (exps.size() > 1) {
                    FIX_ME();  // Add up child expresses
                } else {
                    // oi->reg_symval = RV.expr;
                    oi->conVal = RV.i64;//is it okay if this read is eax, ax, etc?
                    oi->symList = RV.symcellPtr;
                    oi->isSymList = true ;
                }
            } else {
                oi->opty = OPTY_REGCON;
                auto RS = O->getValue()->eval();
                assert(RS.defined);
                oi->reg_conval = RS.convert<ulong>();
            }
            return true;
        }
    } else if (O->isWritten()) {
        // Write into a register oprand:
        // eg2: mov 0xffffffe8(%rbp),%rax -> %rax
        std::set<RegisterAST::Ptr> rdwrRegs;
        oi->rdwr = OPAC_WR;

        // Should be a register operand
        O->getWriteSet(rdwrRegs);
        oi->opty = OPTY_REG;
        assert(rdwrRegs.size() == 1);
        auto R = *rdwrRegs.begin();
        oi->reg_index = R->getID();
        oi->symb = vm->isSYReg(oi->reg_index);
        return true;
    } else {
        ERRR_ME("Unexpected operand");
        exit(EXIT_FAILURE);
        return false;
    }
}

bool SymExecutor::_parseOperand_RX(VMState *vm, DAPIInstrPtr& I, OprndInfoPtr &oi) {
    bool res = false;  // Failed to parse the operand;
    DIAPIOperandPtr &O = oi->PO;
    if (O->isRead()) {
        // Read a memory cell:
        // eg1: mov 0xffffffe8(%rbp),%rax -> 0xffffffe8(%rbp)
        std::set<RegisterAST::Ptr> rdwrRegs;
        oi->rdwr = OPAC_RD;
        oi->opty = OPTY_MEMCELL;

        /* For a mem access insn, if it uses gs, mem access Operand should add gs base */
        // ulong gs_base = isUseGS(I.get()); 
        ulong gs_base = isUseGS(vm, I); 
        /* / */
        
        O->getReadSet(rdwrRegs);
        if (rdwrRegs.size() == 0) {  // Direct memory access
            // assert(false);
            assert(gs_base != 0);
                    
            std::vector<Expression::Ptr> exps;
            auto V = O->getValue();
            V->getChildren(exps);
            assert(exps.size() == 1);  // memory dereference: [xxx] -> xxx

            // Get and eval the address
            auto A = *exps.begin();
            auto RS = A->eval();
            assert(RS.defined);
            // oi->mem_conaddr = RS.convert<ulong>();
            oi->mem_conaddr = RS.convert<ulong>() + gs_base;
                
            printf ("direct mem access through gs, memaddr: %lx. \n", oi->mem_conaddr);
            printf ("size: %d. \n", oi->size);
            
            // asm volatile ("vmcall; \n\t");

            MemValue MV{oi->mem_conaddr, oi->size};
            MV.isSymList = true;
            res = vm->readMemory(MV);
            assert(res);
            if (MV.bsym) {
                oi->opty = OPTY_MEMCELLSYM;
                oi->symb = true;
                // oi->mem_symval = MV.expr;
                oi->conVal = MV.i64;//is it okay if this read is eax, ax, etc?
                oi->symList = MV.symcellPtr;
                oi->isSymList = true ;
            } else {
                oi->opty = OPTY_MEMCELLCON;
                oi->mem_conval = MV.i64;
            }
        } else {
            // Access with one or more registers
            // eg1: mov 0xffffffe8(%rbp),%rax -> 0xffffffe8(%rbp)
            bool bSymbolic;
            bool hasSymReg = false;
            for (auto R : rdwrRegs)
                hasSymReg |= maySymbolicRegister(vm, R.get()->getID());

            if (hasSymReg) {
                assert(false);
                // At least, operand has one symbolic register;
                oi->symb = true;
                oi->opty = OPTY_MEMADDRSYM;
                FIX_ME();
                return false;
            } else {
                // Memory access without symbolic register
                std::vector<Expression::Ptr> exps;
                auto V = O->getValue();
                V->getChildren(exps);
                // memory dereference: [xxx] -> xxx
                assert(exps.size() == 1);

                // Get and eval the address
                auto A = *exps.begin();
                auto RS = A->eval();
                assert(RS.defined);
                // oi->mem_conaddr = RS.convert<ulong>();
#ifdef _DEBUG_OUTPUT                
                std::cout << "read addr " << std::hex << RS.convert<ulong>() << std::endl;
#endif

                if (gs_base == 0)
                    oi->mem_conaddr = RS.convert<ulong>();
                else
                    oi->mem_conaddr = RS.convert<ulong>() + gs_base;

#ifdef _DEBUG_OUTPUT                
                std::cout << "read addr " << std::hex << oi->mem_conaddr << std::endl;
#endif
                
                MemValue MV{oi->mem_conaddr, oi->size};
                MV.isSymList = true;
                res = vm->readMemory(MV);
                assert(res);
                if (MV.bsym) {
                    oi->opty = OPTY_MEMCELLSYM;
                    oi->symb = true;
                    // oi->mem_symval = MV.expr;
                    oi->conVal = MV.i64;//is it okay if this read is eax, ax, etc?
                    oi->symList = MV.symcellPtr;
                    oi->isSymList = true ;

                } else {
                    oi->opty = OPTY_MEMCELLCON;
                    oi->mem_conval = MV.i64;
                }
                return true;
            }
        }
    } else if (O->isWritten()) {
        std::set<RegisterAST::Ptr> rdwrRegs;
        assert(0);
        // asm("int3");
        oi->rdwr = OPAC_WR;
        O->getWriteSet(rdwrRegs);
        // Should be a register operand
        // oi->opty = OPTY_REG;
        assert(rdwrRegs.size() == 1);
        auto R = *rdwrRegs.begin();
        oi->reg_index = R.get()->getID();
        cout << "246: Write: " << O->getValue()->format() << "\n";
        return false;
    } else {
        cerr << "249: Unexpected operand" << O->getValue()->format() << "\n";
        return false;
    }
}

bool SymExecutor::_parseOperand_XW(VMState *vm, DAPIInstrPtr& I, OprndInfoPtr &oi) {
    bool res = false;  // Failed to parse the operand;
    DIAPIOperandPtr &O = oi->PO;
    if (O->isRead()) {
        std::set<RegisterAST::Ptr> rdwrRegs;
        assert(0);
        // asm("int3");
        // Accessing an immeidate value, or reading a register
        oi->rdwr = OPAC_RD;
        O->getReadSet(rdwrRegs);
        if (rdwrRegs.size() == 0) {
            oi->opty = OPTY_IMM;
            auto RS = O->getValue()->eval();
            oi->imm_value = RS.convert<ulong>();
            return false;
        } else {
            bool bSymbolic;
            // A register operand
            // oi->opty = OPTY_REG;
            assert(rdwrRegs.size() == 1);
            auto R = *rdwrRegs.begin();
            oi->reg_index = R.get()->getID();
            oi->symb = bSymbolic = maySymbolicRegister(vm, oi->reg_index);

            if (bSymbolic) {
                oi->symb = true;
                oi->reg_symval = NULL;
                cout << "282: Read: " << O->getValue()->format() << "@SYReg"
                     << "\n";
                return true;
            } else {
                cout << "285: Read: " << O->getValue()->format() << "@NMreg"
                     << "\n";
                return false;
            }
        }
    } else if (O->isWritten()) {
        // eg1: mov $0x0,0xfffffff4(%rbp) -> 0xfffffff4(%rbp)
        std::set<RegisterAST::Ptr> rdwrRegs;
        oi->rdwr = OPAC_WR;       // Write into a memory cell
        oi->opty = OPTY_MEMCELL;  // may be refined later

        /* For a mem access insn, if it uses gs, mem access Operand should add gs base */
        ulong gs_base = isUseGS(vm, I); 
        /* / */
        
        O->getReadSet(rdwrRegs);
        if (rdwrRegs.size() == 0) {
            assert(0);
            // asm("int3");
            // Direct memory access
            std::vector<Expression::Ptr> exps;
            auto V = O->getValue();
            V->getChildren(exps);
            assert(exps.size() == 1);  // memory dereference: [xxx] -> xxx

            // Get and eval the address
            auto A = *exps.begin();
            auto RS = A->eval();
            assert(RS.defined);
            // oi->mem_address = RS.convert<ulong>();
            oi->symb = false;
            // cout << "309: Write: " << O->getValue()->format() << "@" << hex << oi->mem_address << "\n";
            return false;
        } else {
            // Access memory with one or more registers
            // eg1: mov $0x0,0xfffffff4(%rbp) -> 0xfffffff4(%rbp)
            bool hasSymReg = false;
            for (auto R : rdwrRegs)
                hasSymReg |= maySymbolicRegister(vm, R->getID());

            if (hasSymReg) {
                // asm("int3");
                assert(0);
                // Operand has at lease one symbolic register;
                oi->symb = true;
                // oi->mem_address = SYMMEMADDR;
                // oi->SYMemAddr_exp = NULL;
                // cout << "324: Write: " << O->getValue()->format() << "@SYMMEMADDR"
                //      << "\n";
                return false;
            } else {
                // Memory access without symbolic register
                std::vector<Expression::Ptr> exps;
                auto V = O->getValue();
                V->getChildren(exps);
                assert(exps.size() == 1);  // memory dereference: [xxx] -> xxx

                // Get and eval the address
                auto A = *exps.begin();
                auto RS = A->eval();
                assert(RS.defined);
                // oi->mem_conaddr = RS.convert<ulong>();
                
                if (gs_base == 0)
                    oi->mem_conaddr = RS.convert<ulong>();
                else
                    oi->mem_conaddr = RS.convert<ulong>() + gs_base;

                return true;
            }
        }
    } else {
        assert(0);
        // asm("int3");
        cerr << "345: Unexpected operand" << O->getValue()->format() << "\n";
        return false;
    }
}

bool SymExecutor::_parseOperand_RW(VMState *vm, DAPIInstrPtr& I, OprndInfoPtr &oi) {
    bool res = false;  // Failed to parse the operand;
    DIAPIOperandPtr &O = oi->PO;

    oi->rdwr = OPAC_RDWR;
    oi->opty = OPTY_MEMCELL;

    /* For a mem access insn, if it uses gs, mem access Operand should add gs
     * base */
    ulong gs_base = isUseGS(vm, I); 
    /* / */
    
    std::set<RegisterAST::Ptr> rdwrRegs;
    O->getReadSet(rdwrRegs);
    if (rdwrRegs.size() == 0) {
        assert(0);
        // asm("int3");
        // Direct memory access
        std::vector<Expression::Ptr> exps;
        auto V = O->getValue();
        V->getChildren(exps);
        assert(exps.size() == 1);  // memory dereference: [xxx] -> xxx

        // Get and eval the address
        auto A = *exps.begin();
        auto RS = A->eval();
        assert(RS.defined);
        bool bSymbolic;
        // oi->mem_address = RS.convert<ulong>();
        // oi->symb = bSymbolic = maySymbolicMemoryCell(oi->mem_address, V->size());
        if (bSymbolic) {
            // oi->SYMemCell_exp = NULL;
            cout << "385: Read symbolic memory cell:" << O->getValue()->format() << "@" << hex << RS.val.u64val << "\n";
            return true;
        } else {
            cout << "388: Read normal memory cell:" << O->getValue()->format() << "@" << hex << RS.val.u64val << "\n";
            return false;
        }

    } else {
        // Access memory with one or more registers:
        // eg1.add $0x8, 0xfffffff8(%rbp)->0xfffffff8(%rbp)
        bool hasSymReg = false;
        for (auto R : rdwrRegs)
            hasSymReg |= maySymbolicRegister(vm, R->getID());

        if (hasSymReg) {
            FIX_ME();
            assert(0);
            // asm("int3");
            // At least, operand has one symbolic register;
            oi->symb = true;
            // oi->mem_address = SYMMEMADDR;
            // oi->SYMemCell_exp = NULL;
            cout << "406: Read memory with symbolic register:" << O->getValue()->format() << "\n";
            return true;
        } else {
            // Memory access without symbolic register
            std::vector<Expression::Ptr> exps;
            auto V = O->getValue();
            V->getChildren(exps);
            assert(exps.size() == 1);  // memory dereference: [xxx] -> xxx

            // Get and eval the address
            auto A = *exps.begin();
            auto RS = A->eval();
            assert(RS.defined);

            // oi->mem_conaddr = RS.convert<ulong>();
            if (gs_base == 0)
                oi->mem_conaddr = RS.convert<ulong>();
            else
                oi->mem_conaddr = RS.convert<ulong>() + gs_base;

            MemValue MV{oi->mem_conaddr, oi->size};
            MV.isSymList = true;
            res = vm->readMemory(MV);
            assert(res);
            if (MV.bsym) {
                oi->opty = OPTY_MEMCELLSYM;
                oi->symb = true;
                // oi->mem_symval = MV.expr;
                oi->conVal = MV.i64;//is it okay if this read is eax, ax, etc?
                oi->symList = MV.symcellPtr;
                oi->isSymList = true ;
            } else {
                oi->opty = OPTY_MEMCELLCON;
                oi->mem_conval = MV.i64;
            }
            return true;
        }
    }

    return false;
}

bool SymExecutor::parseOperands(VMState *vm, InstrInfo *info, bool isSymList) {
    DAPIInstrPtr &I = info->PI;
    std::vector<OprndInfoPtr> &vecOI = info->vecOI;

    // Set the value of referred regsiters before parsing
    setReadRegs(vm, I);

    bool bUS = false;  // Operands refer to symbolic variable?
    std::vector<Operand> oprands;
    I->getOperands(oprands);
    for (auto O : oprands) {
        OprndInfoPtr oi(new OprndInfo(O));
        oi->size = O.getValue()->size();  // Set the operand size ASAP;
        oi->symb = false;                 // Set to false by default;
        bool res = false;
        if (!O.readsMemory() && !O.writesMemory())
        {
            // res = _parseOperand_XX(vm, oi);
            res = _parseOperand_XX(vm, I, oi);
            // // fixing dyninst
            // if ((oi->opty == OPTY_REGCON) && (I->getOperation().getID() == e_lea))
            //     oi->reg_conval += I->size();

        } else if (O.readsMemory() && !O.writesMemory()) {
            // res = _parseOperand_RX(vm, oi);
            res = _parseOperand_RX(vm, I, oi);
        } else if (!O.readsMemory() && O.writesMemory()) {
            // res = _parseOperand_XW(vm, oi);
            res = _parseOperand_XW(vm, I, oi);
        } else if (O.readsMemory() && O.writesMemory()) {
            // res = _parseOperand_RW(vm, oi);
            res = _parseOperand_RW(vm, I, oi);
        }

        bUS |= res;
        vecOI.push_back(oi);
    }
    // end for
    return bUS;
}


bool SymExecutor::maySymbolicRegister(VMState *vm, uint ID) {
    return vm->isSYReg(ID);
} ;

bool SymExecutor::setReadRegs(VMState *vm, DAPIInstr *I) {
    std::set<RegisterAST::Ptr> readRegs;
    I->getReadSet(readRegs);

    for (auto P : readRegs) {
        uint indx = P->getID();
        uint size = P->size();

        if ((indx & x86::FLAG) == x86::FLAG)
            continue ;

        RegValue V = {indx, size};
        V.isSymList = false;
        bool res = vm->readRegister(V);
        assert(res);
        
        if (V.bsym) {
            // Do nothing
#ifdef _DEBUG_OUTPUT
            cout << "123: " << P->format() << "\n";
#endif
        } else {
            switch (size) {
                case 8:
                    P->setValue(Result(s64, V.i64));
                    break;
                case 4:
                    P->setValue(Result(s32, V.i32));
                    break;
                case 2:
                    P->setValue(Result(s16, V.i16));
                    break;
                case 1:
                    P->setValue(Result(s8, V.i8));
                    break;
                default:
                    FIX_ME();
                    break;
            }
        }
    }
}

bool SymExecutor::setReadRegs(VMState *vm, DAPIInstrPtr &I) {
    return setReadRegs(vm, I.get());
}

void printCellList (SymCellPtr cellList) {
    int i = 0 ;
    std::cout << std::endl;
    while(cellList!=NULL) {
        if(cellList->exprPtr)
            cellList->exprPtr->print();
        std::cout << ", " ;
        cellList = cellList->next ;
        if(i++>20) {
            std::cout << ">20!!!" ;
            return ;
        }
    }
    std::cout << std::endl;
}
// bool SymExecutor::process_mov(VMState *vm, InstrInfoPtr &infoptr) {
//     // Process move instruction
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oidst = vecOI[0];
//     OprndInfoPtr &oisrc = vecOI[1];
//     KVExprPtr e;
//     bool res;
//     long val;
// 
// // #ifdef _DEBUG_OUTPUT
// //     printf ("oisrc index: %lx, is sym: %d .\n", oisrc->reg_index, oisrc->symb);
// //     if (oisrc->symb)
// //     {
// //         KVExprPtr ee1(nullptr);
// //         res = oisrc->getSymValue(ee1);
// //         ee1->print () ;
// //         std::cout << "\n" ;
// //     }
// //     printf ("oidst index: %lx, is sym: %d .\n", oidst->reg_index, oidst->symb);
// //     if (oidst->symb)
// //     {
// //         KVExprPtr ee1(nullptr);
// //         res = oidst->getSymValue(ee1);
// //         ee1->print () ;
// //         std::cout << "\n" ;
// //     }
// // #endif
// 
//     if(oisrc->symb) {
//         // Do reading
//         res = oisrc->getSymValue(e);
//         assert(res);
//         // Do writting
//         res = oidst->setSymValue(vm, e);
//         //e->print() ;
//         //std::cout << "\n" ;
//         assert(res);
//     } else {
//         assert(oidst->symb) ;
//         // Do reading
//         res = oisrc->getConValue(val);
//         assert(res);
//         // Do writting
//         res = oidst->setConValue(vm, val);
//         assert(res);
//     }
// 
//     return true;
// }
// 
// bool SymExecutor::process_add(VMState *vm, InstrInfoPtr &infoptr) {
//     // Process addition
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     OprndInfoPtr &oidst = oisrc1;
//     bool res;
// 
//     KVExprPtr oe ;
// 
//     if (oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e1(nullptr), e2(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         // Generate new expression
//         oe.reset(new AddExpr(e1, e2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else if (oisrc1->symb && !oisrc2->symb) {
//         KVExprPtr e1(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
// 
//         long v2;
//         res = oisrc2->getConValue(v2);
//         assert(res);
//         ExprPtr c2(new ConstExpr(v2));
//         oe.reset(new AddExpr(e1, c2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     }
// 
//     else if (!oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e2(nullptr);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         long v1;
//         res = oisrc1->getConValue(v1);
//         assert(res);
//         ExprPtr c1(new ConstExpr(v1));
//         oe.reset(new AddExpr(c1, e2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else {
//         ERRR_ME("Unexpected operands");
//         asm("int3");
//     }
//     vm->SaveFlagChangingInstructionExpr(e_add, oe) ;
//     return true ;
// }
// 
// bool SymExecutor::process_test(VMState *vm, InstrInfoPtr &infoptr) {
//     
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     OprndInfoPtr &oidst = oisrc1;
//     bool res;
// 
//     KVExprPtr oe = NULL ;
// 
//     if (oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e1(nullptr), e2(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         // Generate new expression
//         oe.reset(new AndExpr(e1, e2));
// 
//     } else if (oisrc1->symb && !oisrc2->symb) {
//         KVExprPtr e1(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
// 
//         long v2;
//         res = oisrc2->getConValue(v2);
//         assert(res);
//         ExprPtr c2(new ConstExpr(v2));
//         oe.reset(new AndExpr(e1, c2));
//     }
// 
//     else if (!oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e2(nullptr);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         long v1;
//         res = oisrc1->getConValue(v1);
//         assert(res);
//         ExprPtr c1(new ConstExpr(v1));
//         oe.reset(new AndExpr(c1, e2));
//     } else {
//         ERRR_ME("Unexpected operands");
//         asm("int3");
//     }
//     vm->SaveFlagChangingInstructionExpr(e_test, oe) ;
//     return true;   
// }
// 
// 
// // QHQ
// bool SymExecutor::process_cmovxx(VMState *vm, InstrInfoPtr &infoptr) {
//     // Process conditional move instruction
//     bool domov = true ;
// 
//     auto &I = infoptr->PI;
// 
//     if (domov){
//         auto &vecOI = infoptr->vecOI;
//         OprndInfoPtr &oidst = vecOI[0];
//         OprndInfoPtr &oisrc = vecOI[1];
//         KVExprPtr e;
//         bool res;
//         long val;
// 
//         if(oisrc->symb) {
//             // Do reading
//             res = oisrc->getSymValue(e);
//             assert(res);
//             // Do writting
//             res = oidst->setSymValue(vm, e);
//             assert(res);
//         } else {
//             // assert(oidst->symb) ;
//             // Do reading
//             res = oisrc->getConValue(val);
//             assert(res);
//             // Do writting
//             res = oidst->setConValue(vm, val);
//             assert(res);
//         }
//     }
// 
//     return true;
// }
// 
// bool SymExecutor::process_jxx(VMState *vm, InstrInfoPtr &infoptr) {
// 
//     std::cout << "jump instructions: JMP/Jcc" << "\n" ;
//     assert(0) ;
//     return true;
// }
// 
// // QHQ  
// bool SymExecutor::process_cmp(VMState *vm, InstrInfoPtr &infoptr) {
//         auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     OprndInfoPtr &oidst = oisrc1;
//     KVExprPtr oe = NULL;
//     bool res;
// 
// #ifdef _DEBUG_OUTPUT
//     printf ("oi1 index: %lx, is sym: %d .\n", oisrc1->reg_index, oisrc1->symb);
//     if (oisrc1->symb)
//     {
//         KVExprPtr ee1(nullptr);
//         res = oisrc1->getSymValue(ee1);
//         ee1->print () ;
//     }
//     printf ("oi2 index: %lx, is sym: %d .\n", oisrc2->reg_index, oisrc2->symb);
//     if (oisrc2->symb)
//     {
//         KVExprPtr ee1(nullptr);
//         res = oisrc2->getSymValue(ee1);
//         ee1->print () ;
//     }
// #endif
// 
//     if (oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e1(nullptr), e2(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         // Generate new expression
//         oe.reset(new SubExpr(e1, e2));
// 
//     } else if (oisrc1->symb && !oisrc2->symb) {
//         KVExprPtr e1(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
// 
//         long v2;
//         res = oisrc2->getConValue(v2);
//         assert(res);
//         ExprPtr c2(new ConstExpr(v2));
//         oe.reset(new SubExpr(e1, c2));
//     }
// 
//     else if (!oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e2(nullptr);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         long v1;
//         res = oisrc1->getConValue(v1);
//         assert(res);
//         ExprPtr c1(new ConstExpr(v1));
//         oe.reset(new SubExpr(c1, e2));
//     } else {
//         ERRR_ME("Unexpected operands");
//         asm("int3");
//     }
//     res = vm->SaveFlagChangingInstructionExpr(e_cmp, oe) ;
//     assert (res) ;
//     return true;   
// }
// 
// bool SymExecutor::process_sub(VMState *vm, InstrInfoPtr &infoptr) {
//     // Process sub
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     OprndInfoPtr &oidst = oisrc1;
//     KVExprPtr oe = NULL;
//     bool res;
// 
//     if (oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e1(nullptr), e2(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         // Generate new expression
//         oe.reset(new SubExpr(e1, e2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else if (oisrc1->symb && !oisrc2->symb) {
//         KVExprPtr e1(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
// 
//         long v2;
//         res = oisrc2->getConValue(v2);
//         assert(res);
//         ExprPtr c2(new ConstExpr(v2));
//         oe.reset(new SubExpr(e1, c2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     }
// 
//     else if (!oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e2(nullptr);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         long v1;
//         res = oisrc1->getConValue(v1);
//         assert(res);
//         ExprPtr c1(new ConstExpr(v1));
//         oe.reset(new SubExpr(c1, e2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else {
//         ERRR_ME("Unexpected operands");
//         assert(0);
//     }
//     res = vm->SaveFlagChangingInstructionExpr(e_sub, oe) ;
//     assert (res) ;
//     return true ;
// 
// }
// bool SymExecutor::process_and(VMState *vm, InstrInfoPtr &infoptr) {
//     
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     OprndInfoPtr &oidst = oisrc1;
//     KVExprPtr oe = NULL ;
//     bool res;
// 
//     if (oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e1(nullptr), e2(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         // Generate new expression
//         oe.reset(new AndExpr(e1, e2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else if (oisrc1->symb && !oisrc2->symb) {
//         KVExprPtr e1(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
// 
//         long v2;
//         res = oisrc2->getConValue(v2);
//         assert(res);
//         ExprPtr c2(new ConstExpr(v2));
//         oe.reset(new AndExpr(e1, c2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     }
// 
//     else if (!oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e2(nullptr);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         long v1;
//         res = oisrc1->getConValue(v1);
//         assert(res);
//         ExprPtr c1(new ConstExpr(v1));
//         oe.reset(new AndExpr(c1, e2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else {
//         ERRR_ME("Unexpected operands");
//         assert(0);
//     }
//     res = vm->SaveFlagChangingInstructionExpr(e_and, oe) ;
//     assert (res) ;
// 
//     return true;   
// }
// bool SymExecutor::process_or(VMState *vm, InstrInfoPtr &infoptr) {
//     
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     OprndInfoPtr &oidst = oisrc1;
//     KVExprPtr oe = NULL ;
//     bool res;
// 
//     if (oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e1(nullptr), e2(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         // Generate new expression
//         oe.reset(new OrExpr(e1, e2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else if (oisrc1->symb && !oisrc2->symb) {
//         KVExprPtr e1(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
// 
//         long v2;
//         res = oisrc2->getConValue(v2);
//         assert(res);
//         ExprPtr c2(new ConstExpr(v2));
//         oe.reset(new OrExpr(e1, c2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     }
// 
//     else if (!oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e2(nullptr);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         long v1;
//         res = oisrc1->getConValue(v1);
//         assert(res);
//         ExprPtr c1(new ConstExpr(v1));
//         oe.reset(new OrExpr(c1, e2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else {
//         ERRR_ME("Unexpected operands");
//         assert(0);
//     }
// 
//     res = vm->SaveFlagChangingInstructionExpr(e_or, oe) ;
//     assert (res) ;
//     return true;   
// }
// 
// bool SymExecutor::process_xor(VMState *vm, InstrInfoPtr &infoptr) {
//     
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     OprndInfoPtr &oidst = oisrc1;
//     KVExprPtr oe = NULL ;
//     bool res;
// 
//     if (oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e1(nullptr), e2(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         // Generate new expression
//         oe.reset(new XorExpr(e1, e2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else if (oisrc1->symb && !oisrc2->symb) {
//         KVExprPtr e1(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
// 
//         long v2;
//         res = oisrc2->getConValue(v2);
//         assert(res);
//         ExprPtr c2(new ConstExpr(v2));
//         oe.reset(new XorExpr(e1, c2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     }
// 
//     else if (!oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e2(nullptr);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         long v1;
//         res = oisrc1->getConValue(v1);
//         assert(res);
//         ExprPtr c1(new ConstExpr(v1));
//         oe.reset(new XorExpr(c1, e2));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else {
//         ERRR_ME("Unexpected operands");
//         assert(0);
//     }
// 
//     res = vm->SaveFlagChangingInstructionExpr(e_xor, oe) ;
//     assert (res) ;
// 
//     return true;   
// }
// 
// bool SymExecutor::process_shl_sal(VMState *vm, InstrInfoPtr &infoptr) {
//     
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     OprndInfoPtr &oidst = oisrc1;
//     bool res;
// 
//     assert (oisrc1->symb) ;
//     assert (!oisrc2->symb) ;
// 
//     KVExprPtr e1(nullptr);
//     res = oisrc1->getSymValue(e1);
//     assert(res);
// 
//     long v2;
//     res = oisrc2->getConValue(v2);
//     assert(res);
//     ExprPtr c2(new ConstExpr(v2));
//     KVExprPtr oe(new Shl_SalExpr(e1, c2));
//     res = oidst->setSymValue(vm, oe);
// 
//     assert(res);
//     
//     res = vm->SaveFlagChangingInstructionExpr(e_shl_sal, oe) ;
//     assert (res) ;
//     return true;   
// }
// 
// bool SymExecutor::process_shr(VMState *vm, InstrInfoPtr &infoptr) {
//     
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     OprndInfoPtr &oidst = oisrc1;
//     bool res;
// 
//     assert (oisrc1->symb) ;
//     assert (!oisrc2->symb) ;
// 
//     KVExprPtr e1(nullptr);
//     res = oisrc1->getSymValue(e1);
//     assert(res);
// 
//     long v2;
//     res = oisrc1->getConValue(v2);
//     assert(res);
//     ExprPtr c2(new ConstExpr(v2));
//     KVExprPtr oe(new ShrExpr(e1, c2));
//     res = oidst->setSymValue(vm, oe);
//     assert(res);
// 
//     res = vm->SaveFlagChangingInstructionExpr(e_shr, oe) ;
//     assert (res) ;
// 
//     return true;   
// }
// 
// bool SymExecutor::process_sar(VMState *vm, InstrInfoPtr &infoptr) {
//     
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     OprndInfoPtr &oidst = oisrc1;
//     bool res;
// 
//     assert (oisrc1->symb) ;
//     assert (!oisrc2->symb) ;
// 
//     KVExprPtr e1(nullptr);
//     res = oisrc1->getSymValue(e1);
//     assert(res);
// 
//     long v2;
//     res = oisrc1->getConValue(v2);
//     assert(res);
//     ExprPtr c2(new ConstExpr(v2));
//     KVExprPtr oe(new SarExpr(e1, c2));
//     res = oidst->setSymValue(vm, oe);
//     assert(res);
// 
//     res = vm->SaveFlagChangingInstructionExpr(e_sar, oe) ;
//     assert (res) ;
//     return true;   
// }
// 
// bool SymExecutor::process_idiv(VMState *vm, InstrInfoPtr &infoptr) {
//     // Process addition
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &o_d = vecOI[0]; 
//     OprndInfoPtr &o_a = vecOI[1];
//     OprndInfoPtr &o_Divisor = vecOI[2];
//     bool res;
//     KVExprPtr e(nullptr);
//     KVExprPtr e_a(nullptr), e_d(nullptr), e_Dividend(nullptr), e_Divisor(nullptr);
//     long v_a, v_d, v_Dividend, v_Divisor ;
// 
//     if (o_d->symb || o_a->symb) {
//         if(o_d->symb) {
//             res = o_d->getSymValue(e_d) ;
//             assert (res) ;
//         }
//         else {
//             res = o_d->getConValue(v_d) ;
//             assert (res) ;
//             e_d.reset(new ConstExpr(v_d)) ;
//         }
//          if(o_a->symb) {
//             res = o_a->getSymValue(e_a) ;
//             assert (res) ;
//          }
//         else {
//             res = o_a->getConValue(v_a) ;
//             assert (res) ;
//             e_a.reset(new ConstExpr(v_a)) ;
//         }
//     } else {
//         // dx:ax both not symbol, we may need a 128bits int.
//         res = o_d->getConValue(v_d) ;
//         assert (res) ;
//         e_d.reset(new ConstExpr(v_d)) ;
//         
//         res = o_a->getConValue(v_a) ;
//         assert (res) ;
//         e_a.reset(new ConstExpr(v_a)) ;
//     }
// 
//     e_Dividend.reset(new CombineExpr(e_d, e_a, o_d->size, o_a->size, o_d->size + o_a->size, 0)) ;
// 
//     if (o_Divisor->symb) {
//         res = o_Divisor->getSymValue(e_Divisor) ;
//         assert (res) ;
//     } else {
//         res = o_Divisor->getConValue(v_Divisor) ;
//         assert (res) ;
//         e_Divisor.reset(new ConstExpr(v_Divisor)) ;
//     }
// 
//     KVExprPtr e_Quotient(new iDivExpr(e_Dividend, e_Divisor, o_Divisor->size, 0));
//     KVExprPtr e_Remainder(new iDivExpr(e_Dividend, e_Divisor, o_Divisor->size, 0));
// 
//     res = o_a->setSymValue(vm, e_Quotient) ;
//     assert (res) ;
//     res = o_d->setSymValue(vm, e_Remainder) ;
//     assert (res) ;
//     
//     return true ;
// }
// __uint128_t SignedTOUnsigned (long v, int size) {
//     switch (size) {
//         case 1:
//             return (__uint128_t)(uint8_t)(v) ;
//         case 2:
//             return (__uint128_t)(uint16_t)(v) ;
//         case 4:
//             return (__uint128_t)(uint32_t)(v) ;
//         case 8:
//             return (__uint128_t)(uint64_t)(v) ;
//         default :
//             assert (0) ;
//     }
// }
// bool SymExecutor::process_mul(VMState *vm, InstrInfoPtr &infoptr) {
//     // Process addition
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &o_d = vecOI[0];        // dx
//     OprndInfoPtr &o_a = vecOI[1];        // ax
//     OprndInfoPtr &o_m = vecOI[2];        // oper
//     KVExprPtr e_d, e_a, e_m ;
//     long v_d, v_a, v_m ;
//     bool res;
//     __uint128_t v_r=0, u_a, u_m ;
// 
//     if(o_a->symb) {
//         o_a->getSymValue(e_a) ;
//         assert (res) ;
//     }
//     else {
//         res = o_a->getConValue(v_a) ;
//         assert (res) ;
//         e_a.reset(new ConstExpr(v_a)) ;
//     }
//     if(o_m->symb) {
//         res = o_m->getSymValue(e_m) ;
//         assert (res) ;
//     }
//     else {
//         o_m->getConValue(v_m) ;
//         assert (res) ;
//         e_m.reset(new ConstExpr(v_m)) ;
//     }
// 
//     if(!o_a->symb && !o_m->symb) {
//         auto &I = infoptr->PI;    
//         entryID id = I->getOperation().getID() ;
//         
//         if (id == e_mul) {
//             u_a = SignedTOUnsigned (v_a, o_a->size) ;
//             u_m = SignedTOUnsigned (v_m, o_m->size) ;
//             v_r = u_a * u_m ;
//         } else {
//             v_r = v_a * v_m ;
//         }
//         uint64_t vrl = (uint64_t)(v_r & ((((__uint128_t)1)<<o_a->size*8)-1)) ;
//         uint64_t vrh = (uint64_t)((v_r>>(o_d->size*8)) & ((((__uint128_t)1)<<o_d->size*8)-1)) ;
//         res = o_a->setConValue(vm, vrl) ;
//         assert (res) ;
//         res = o_d->setConValue(vm, vrh) ;
//         assert (res) ;
// 
//         return true ;
//     }
// 
//     KVExprPtr e_r(new MulExpr(e_a, e_m, o_m->size, 0)) ;
//     e_a.reset(new ExtractExpr(e_r, 0, o_a->size, o_a->size, 0)) ;
//     e_d.reset(new ExtractExpr(e_r, o_a->size, o_a->size*2, o_a->size, 0)) ;
// 
//     e_a->print () ;
//     std::cout << std::endl ;
//     e_d->print () ;
//     std::cout << std::endl ;
//     
//     res = o_a->setSymValue(vm, e_a) ;
//     assert (res) ;
//     res = o_d->setSymValue(vm, e_d) ;
//     assert (res) ;
// 
//     return true ;
// }
// 
// bool SymExecutor::process_not(VMState *vm, InstrInfoPtr &infoptr) {
//     // Process addition
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oidst = oisrc1;
//     KVExprPtr oe = NULL ;
//     bool res;
// 
//     if (oisrc1->symb) {
//         KVExprPtr e1(nullptr) ;
//         res = oisrc1->getSymValue(e1);
//         assert(res);
//         // Generate new expression
//         oe.reset(new NotExpr(e1));
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else {
//         ERRR_ME("Unexpected operands");
//         assert(0);
//     }
//     res = vm->SaveFlagChangingInstructionExpr(e_not, oe) ;
//     assert (res) ;
//     return true ;
// }
// 
// bool SymExecutor::process_neg(VMState *vm, InstrInfoPtr &infoptr) {
//     // Process addition
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oidst = oisrc1;
//     bool res;
// 
//     assert (oisrc1->symb) ;
// 
//     KVExprPtr e1(nullptr) ;
//     res = oisrc1->getSymValue(e1);
//     assert(res);
//     // Generate new expression
//     KVExprPtr oe(new NegExpr(e1));
//     res = oidst->setSymValue(vm, oe);
//     assert (res) ;
// 
//     res = vm->SaveFlagChangingInstructionExpr(e_neg, oe) ;
//     
//     assert(res);
//     return true ;
// }
// 
// bool SymExecutor::process_pop(VMState *vm, InstrInfoPtr &infoptr) {
//         
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];   // rsp
//     bool res;
//     RegValue V ;
//      
//     assert(!oisrc2->symb) ;
// 
//     V.indx = oisrc2->reg_index;
//     V.size = oisrc2->size ;
// 
//     res = vm->readRegister(V);
//     assert (res) ;
//     assert (V.size==8) ;
// 
// // debug log 20220512
// #ifdef _DEBUG_OUTPUT
//     std::cout << "pop :" << std::hex << V.u64 << " " ;
// #endif
// // debug log 20220512
// 
//     MemValue MV;
//     MV.addr = V.u64 ;
//     MV.size = oisrc1->size ;
//     res = vm->readMemory(MV) ;
//     assert(res) ;
//     
//     if(MV.bsym) {
//         res = oisrc1->setSymValue(vm, MV.expr) ;
// // debug log 20220512        
// #ifdef _DEBUG_OUTPUT
//         MV.expr->print () ;
//         std::cout << "\n" ;
// #endif
// // debug log 20220512
//     }
//     else {
//         res = oisrc1->setConValue(vm, MV.u64) ;
// // debug log 20220512
// #ifdef _DEBUG_OUTPUT
//         std::cout << std::hex << "0x" << MV.u64 << std::endl ;
// #endif
// // debug log 20220512         
//     }
// 
//     assert(res) ;
//     
//     V.u64 += V.size ;
//     res = vm->writeRegister(V);
//     assert(res) ;
// 
//     return true;
// }
// 
// bool SymExecutor::process_push(VMState *vm, InstrInfoPtr &infoptr) {
//     
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];    // rsp
//     bool res;
//     RegValue V ;
//      
// //    assert(oisrc1->symb) ;
//     assert(!oisrc2->symb) ;
// 
//     V.indx = oisrc2->reg_index;
//     V.size = oisrc2->size ;
// 
//     res = vm->readRegister(V);
// 
//     assert(res) ;
//     assert (V.size==8) ;
//     V.u64 -= V.size ;
//     res = vm->writeRegister(V);
//     assert(res) ;
// 
// // debug log 20220512
// #ifdef _DEBUG_OUTPUT
//     std::cout << "push :" << std::hex << V.u64 << " " ;
// #endif
// // debug log 20220512
// 
//     MemValue MV;
//     MV.addr = V.u64 ;
//     MV.size = oisrc1->size ;
// 
//     KVExprPtr e1(nullptr) ;
//     if(oisrc1->symb) {
//         res = oisrc1->getSymValue(e1);
//         assert(res) ;
//         MV.bsym = true ;
//         MV.expr = e1 ;
// 
// // debug log 20220512        
// #ifdef _DEBUG_OUTPUT
//         e1->print () ;
//         std::cout << "\n" ;
// #endif
// // debug log 20220512
// 
//     } else {
//         long v ;
//         res = oisrc1->getConValue(v);
//         assert(res) ;
//         MV.bsym = false ;
//         MV.i64 = v ;
// 
// // debug log 20220512
// #ifdef _DEBUG_OUTPUT
//         std::cout << std::hex << "0x" << v << std::endl ;
// #endif
// // debug log 20220512        
//     }
// 
//     res = vm->writeMemory(MV) ;
//     assert(res) ;
// 
//     return true;
// }
// 
// 
// bool SymExecutor::process_xchg(VMState *vm, InstrInfoPtr &infoptr) {
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc1 = vecOI[0];
//     OprndInfoPtr &oisrc2 = vecOI[1];
//     bool res;
// 
//     if (oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e1(nullptr), e2(nullptr);
//         res = oisrc1->getSymValue(e1);
//         assert(res);
//         res = oisrc2->getSymValue(e2);
//         assert(res);
// 
//         res = oisrc1->setSymValue(vm, e2);
//         assert(res);
//         res = oisrc2->setSymValue(vm, e1);
//         assert(res);
// 
//     } else if (oisrc1->symb && !oisrc2->symb) {
//         KVExprPtr e1(nullptr);
//         long v2;
// 
//         res = oisrc1->getSymValue(e1);
//         assert(res);
//         res = oisrc2->getConValue(v2);
//         assert(res);
// 
//         res = oisrc1->setConValue(vm, v2);
//         assert(res);
//         res = oisrc2->setSymValue(vm, e1);
//         assert(res);
// 
//     } else if (!oisrc1->symb && oisrc2->symb) {
//         KVExprPtr e2(nullptr);
//         long v1;
//  
//         res = oisrc2->getSymValue(e2);
//         assert(res);
//         res = oisrc1->getConValue(v1);
//         assert(res);
//  
//         res = oisrc1->setSymValue(vm, e2);
//         assert(res);
//         res = oisrc2->setConValue(vm, v1);
//         assert(res);
//     } else {
//         ERRR_ME("Unexpected operands");
//         assert(0);
//     }
//     // vm->SaveFlagChangingInstructionExpr(e_xchg, oe) ;
//     return true;
// }
// 
// bool SymExecutor::process_movsx(VMState *vm, InstrInfoPtr &infoptr) {
//     // Process move instruction
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oidst = vecOI[0];
//     OprndInfoPtr &oisrc = vecOI[1];
//     bool res;
// 
//     if(oisrc->symb) {
//         KVExprPtr e;
//         // Do reading
//         res = oisrc->getSymValue(e);
//         assert(res);
// 
//         KVExprPtr oe(new SignExtExpr(e));
// 
//         // Do writting
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else {
//         long val;
//         assert  (oidst->symb) ;
//         // Do reading
//         res = oisrc->getConValue(val);
//         assert(res);
// 
//         // Do writting
//         res = oidst->setConValue(vm, val);
//         assert(res);
//     }
// 
//     return true;
// }
// 
// bool SymExecutor::process_movzx(VMState *vm, InstrInfoPtr &infoptr) {
//     // Process move instruction
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oidst = vecOI[0];
//     OprndInfoPtr &oisrc = vecOI[1];
//     bool res;
// 
//     if(oisrc->symb) {
//         KVExprPtr e;
//         // Do reading
//         res = oisrc->getSymValue(e);
//         assert(res);
// 
//         KVExprPtr oe(new ZeroExtExpr(e));
// 
//         // Do writting
//         res = oidst->setSymValue(vm, oe);
//         assert(res);
//     } else {
//         long val;
//         assert  (oidst->symb) ;
//         // Do reading
//         res = oisrc->getConValue(val);
//         assert(res);
// 
//         // Do writting
//         res = oidst->setConValue(vm, val);
//         assert(res);
//     }
// 
//     return true;
// }
// 
// bool SymExecutor::process_cbw(VMState *vm, InstrInfoPtr &infoptr) {
//     // eax sign extend to rax like
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oisrc = vecOI[0];
//     KVExprPtr e, oe ;
//     long val;
//     RegValue rv ;
//     bool res ;    
//     if (oisrc->symb) {
//         res = oisrc->getSymValue(e) ;
//         assert (res) ;
//     } else {
//         res = oisrc->getConValue(val) ;
//         assert (res) ;
//     }
// 
//     rv.bsym = oisrc->symb ;
//     rv.size = oisrc->size *2 ;
// 
//     switch (oisrc->size) {
//         case 1:
//             rv.indx = x86_64::ax ;
//             break ;
//         case 2:
//             rv.indx = x86_64::eax ;
//             break ;
//         case 4:
//             rv.indx = x86_64::rax ;
//             break ;
//         default :
//         // WTF?
//             assert (0) ;
//     }
//     
//     if(rv.bsym) {
//         oe.reset(new SignExpr(e, rv.size, 0)) ;
//         rv.expr = oe ;
//     } else {
//         rv.i64 = val ;
//     }
// 
//     res = vm->writeRegister(rv) ;
//     assert (res) ;
// 
//     return true;
// }
// 
// bool SymExecutor::process_cdq(VMState *vm, InstrInfoPtr &infoptr) {
//     // eax sign extend to edx::eax like
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &o_d = vecOI[0];       // dx
//     OprndInfoPtr &o_a = vecOI[1];       // ax
//     KVExprPtr e_a, e_r, e_d;
//     RegValue rv_d ;
//     long v_a;
//     bool res;
// 
//     if (o_a->symb) {
//         res = o_a->getSymValue (e_a) ;
//         assert (res) ;
//         e_r.reset (new SignExtExpr(e_a, o_a->size*2, 0)) ;
//         e_d.reset (new ExtractExpr(e_r, o_a->size, o_a->size*2, o_d->size, 0)) ;
//         e_d->print() ;
//         std::cout << std::endl ;
//         res = o_d->setSymValue(vm, e_d) ;
//         assert (res) ;
//         return true ;
//     } else {
//         res = o_a->getConValue (v_a) ;
//         assert (res) ;
//         if (v_a>=0) 
//             v_a = 0 ;
//         else
//             v_a = -1 ;
//         res = o_d->setConValue(vm, v_a) ;
//         assert (res) ;
//         return true ;
//     }
//     return true;
// }
// bool SymExecutor::process_set(VMState *vm, InstrInfoPtr &infoptr) {
//     
//     auto &vecOI = infoptr->vecOI;
//     OprndInfoPtr &oidst = vecOI[0];
//     long v = 1 ;
//     bool res ;
// 
//     res = oidst->setConValue(vm, v) ;
//     assert (res) ;
//     
//     return true ;
// }
// 
// bool SymExecutor::process_shrd(VMState *vm, InstrInfoPtr &infoptr) {
//     assert (0) ;
//     return true ;
// }
// 
// bool SymExecutor::Print_Inst(VMState *vm, InstrInfoPtr &infoptr, const char* cstr) {
//     auto &vecOI = infoptr->vecOI ;
//     int i = 0;
//     DAPIInstrPtr &I = infoptr->PI;    
//     // entryID id = I->getOperation().getID() ;
// 
//     std::cout << cstr << I->format() << std::endl ;
// #if 0
//     for(i=0; i<vecOI.size(); i++) {
//         OprndInfoPtr &o=vecOI[i] ;
//         KVExprPtr e ;
//         long v ;
//         if(o->symb) {
//             o->getSymValue (e) ;
//             e->print() ;
//             std::cout << "," ;
//         } else {
//             if((o->rdwr & OPAC_RD) != 0){
//                 o->getConValue(v) ;
//                 std::cout << std::hex << v << ", " ;
//             } else {
//                 std::cout << "----, " ;
//             }
//         }
//     }
// #endif
//     std::cout << endl ;
//     return true ;   
// }
