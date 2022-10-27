#include <iostream>
#include <fstream>

#include "Expr.h"
#include "InstructionDecoder.h"

#include "EFlagsManager.h"

// #include "VMState.h"

using namespace std;
using namespace Dyninst;
using namespace InstructionAPI;
using namespace EXPR;


static const entryID stateChangingInstrList[] = {e_adc, e_add, e_cmp, e_cmpw, e_cmpxch, e_cmpxch8b, e_and, 
    e_bsf, e_bsr, e_bt, e_btc, e_btr, e_bts, e_dec, e_decl, e_div, e_idiv, e_imul, e_inc, e_neg, e_or, 
    e_popcnt, e_popf, e_popfq, e_popfd, e_rdrand, e_rol, e_rolb, e_ror, e_salc, e_sar, e_sarb, e_sbb, e_sbbl, 
    e_scas, e_scasb, e_scasd, e_scasw, e_shl_sal, e_shld, e_shr, e_shrb, e_shrd, e_sub, e_sysret, e_test, 
    e_tzcnt, e_xadd, e_xor, e_sahf
} ;

static int instrAttr[_entry_ids_max_] ;


bool EFlagsManager::ConcreteFlag (entryID instrID, bool bExecute) {

    FLAG_STAT fset = FLAG_SET, fclr = FLAG_CLEAR ;

    if (!isConditionalExecuteInstr(instrID)) 
        return false ;

    switch (instrID)
    {
        case e_jnb:
        case e_jnb_jae_j:
        case e_cmovnb:
        case e_setnb:
            // AE/NB/No Carry;		(CF=0)
            m_VM->setFlagBit(x86_64::cf, bExecute?fclr:fset) ;
            break ;

        case e_ja:
        case e_jnbe:
        case e_cmovnbe:
        case e_setnbe:
            // A/NBE; 			(CF=0 and ZF=0)
            if(bExecute) {
                m_VM->setFlagBit(x86_64::cf, fclr) ;
                m_VM->setFlagBit(x86_64::zf, fclr) ;
            }
            
            break ;

        case e_jbe:
        case e_cmovbe:
        case e_setbe:
            // BE/NA;			(CF=1 or ZF=1)
            // can't do anyting
            break ;
        
        case e_jb:
        case e_jb_jnaej_j:
        case e_cmovnae:
        case e_setb:
            // B/NAE/Carry;		(CF=1)
            m_VM->setFlagBit(x86_64::cf, bExecute?fset:fclr) ;
            
            break ;
        
        case e_jge:
        case e_jnl:
        case e_cmovnl:
        case e_setnl:
            // GE/NL;			(SF=OF)
            // cannot do anything
            break ;

        
        case e_jnle:
        case e_jg:
        // case e_cmovnle:
        case e_setnle:
            // G/NLE;			(ZF=0 and SF=OF)
            if(bExecute) {
                  m_VM->setFlagBit(x86_64::zf, fclr) ;
            }
            break ;
        
        case e_jle:
        case e_cmovng:
        case e_setle:
            // LE/NG;			(ZF=1 or SF≠ OF)
            m_VM->setFlagBit(x86_64::zf, bExecute?fset:fclr) ;
            break ;
        
        case e_jl:
        case e_cmovnge:
        case e_setl:
            // L/NGE; 			(SF≠ OF)
            // cannot do anything
            break ;

        case e_je:
        case e_jz:
        case e_cmove:
        case e_setz:
            // E/Z/NE/NZ (zf)
            m_VM->setFlagBit(x86_64::zf, bExecute?fset:fclr) ;
            break ;

        case e_jne:
        case e_jnz:
        case e_cmovne:
        case e_setnz:
            // E/Z/NE/NZ (zf)
            m_VM->setFlagBit(x86_64::zf, bExecute?fclr:fset) ;
            break ;

        case e_jns:
        case e_cmovns:
        case e_setns:
            // S/NS (sf)
            m_VM->setFlagBit(x86_64::sf, bExecute?fclr:fset) ;
            break ;

        case e_js:
        case e_cmovs:
        case e_sets:
            // S/NS (sf)
            m_VM->setFlagBit(x86_64::sf, bExecute?fset:fclr) ;
            break ;

        case e_jno:
        case e_cmovno:
        case e_setno:
            // O/NO (of)
            m_VM->setFlagBit(x86_64::of, bExecute?fclr:fset) ;
            break ;

        case e_jo:
        case e_cmovo:
        case e_seto:
            m_VM->setFlagBit(x86_64::of, bExecute?fset:fclr) ;
            // O/NO (of)
            
            break ;

        case e_jrcxz:
        case e_jcxz_jec:
        case e_jp:
        case e_jnp:
        case e_cmovpe:
        case e_cmovpo:
        case e_setp:
        case e_setnp:
 //       case e_jmp:
 //       case e_jmpq:
 //       case e_jmpe:

            assert(0);
            // asm("int3");
            break ;   

        default:
            assert(0);
            // asm("int3");
            break;
    }
    return true ;
}

bool EFlagsManager::SaveFlagChangingInstruction (FSInstrPtr &ptr) {
    m_LastInstr = ptr ;
    return true ;
}

bool EFlagsManager::SaveFlagChangingInstructionExpr (entryID instrID, KVExprPtr exprPtr) {
    if(isFlagChangingInstr(instrID) || isFlagSettingInstr(instrID)) {
        FLAG_STAT flag = FLAG_UNCERTAIN ;
        m_LastExpr = exprPtr ;
        m_VM->setFlagBit(x86_64::zf, flag) ;
        m_VM->setFlagBit(x86_64::cf, flag) ;
        m_VM->setFlagBit(x86_64::of, flag) ;
        m_VM->setFlagBit(x86_64::af, flag) ;
        m_VM->setFlagBit(x86_64::pf, flag) ;
        m_VM->setFlagBit(x86_64::sf, flag) ;
        return true ;
    } else
        return false ;
}

// FLAG_STAT EFlagsManager::GetFlag(int type) 

bool EFlagsManager::DoCreateConstraint(int exprID, bool bExecute) {
    
    extern KVExprPtr CreateExprByID(int id, KVExprPtr R, KVExprPtr M, KVExprPtr L, int size = 4, int offset = 0) ;
    KVExprPtr cstnt = NULL ;

    if (m_LastExpr)
        cstnt = (CreateExprByID(exprID, m_LastExpr, NULL, NULL, m_LastExpr->size, m_LastExpr->offset));
    else
        assert(0);
        // asm("int3") ;

    if (!bExecute)
        cstnt.reset(new LNotExpr(cstnt, cstnt->size, cstnt->offset)) ;
    
    m_Constraint.insert(cstnt) ;

#ifdef _SYM_DEBUG_OUTPUT
    cstnt->print() ;
    std::cout << "\n" ;
    
    SolveConstraints ();
#endif
    return true ;
}

bool EFlagsManager::CreateConstraint(entryID instrID, bool bExecute) {
    
    int exprID = EXPR_UNDEFINED ;
    if (!isConditionalExecuteInstr(instrID)) 
        return false ;

    switch (instrID)
    {
        case e_jnb:
        case e_jnb_jae_j:
        case e_cmovnb:
        case e_setnb:
            // AE/NB/No Carry;		(CF=0)
            exprID = EXPR_Uge ;
            break ;

        case e_ja:
        case e_jnbe:
        case e_cmovnbe:
        case e_setnbe:
            // A/NBE; 			(CF=0 and ZF=0)
            exprID = EXPR_Ugt ;
            break ;

        case e_jbe:
        case e_cmovbe:
        case e_setbe:
            // BE/NA;			(CF=1 or ZF=1)
            exprID = EXPR_Ule ;
            break ;
        
        case e_jb:
        case e_jb_jnaej_j:
        case e_cmovnae:
        case e_setb:
            // B/NAE/Carry;		(CF=1)
            exprID = EXPR_Ult ;
            break ;
        
        case e_jge:
        case e_jnl:
        case e_cmovnl:
        case e_setnl:
            // GE/NL;			(SF=OF)
            exprID = EXPR_Sge ;
            break ;

        
        case e_jnle:
        case e_jg:
        // case e_cmovnle:
        case e_setnle:
            // G/NLE;			(ZF=0 and SF=OF)
            exprID = EXPR_Sgt ;
            break ;
        
        case e_jle:
        case e_cmovng:
        case e_setle:
            // LE/NG;			(ZF=1 or SF≠ OF)
            exprID = EXPR_Sle ;
            break ;
        
        case e_jl:
        case e_cmovnge:
        case e_setl:
            // L/NGE; 			(SF≠ OF)
            exprID = EXPR_Slt ;
            break ;

        case e_je:
        case e_jz:
        case e_cmove:
        case e_setz:
            exprID = EXPR_Equal ;
            break ;

        case e_jne:
        case e_jnz:
        case e_cmovne:
        case e_setnz:
            // E/Z/NE/NZ (zf)
            exprID = EXPR_Distinct ;
            break ;

        case e_jns:
        case e_cmovns:
        case e_setns:
            exprID = EXPR_NoSign ;
            break ;

        case e_js:
        case e_cmovs:
        case e_sets:
            // S/NS (sf)
            exprID = EXPR_Sign ;
            break ;

        case e_jno:
        case e_cmovno:
        case e_setno:
            exprID = EXPR_NoOverflow ;
            break ;

        case e_jo:
        case e_cmovo:
        case e_seto:
            // O/NO (of)
            exprID = EXPR_Overflow ;
            break ;

        case e_jrcxz:
        case e_jcxz_jec:
        case e_jp:
        case e_jnp:
        case e_cmovpe:
        case e_cmovpo:
        case e_setp:
        case e_setnp:
 //       case e_jmp:
 //       case e_jmpq:
 //       case e_jmpe:

            assert(0);
            // asm("int3");
            break ;   

        default:
            assert(0);
            // asm("int3");
            break;
    }
    
    if(exprID != EXPR_UNDEFINED) {
        DoCreateConstraint (exprID, bExecute) ;
        return true ;
    } else
        return (false) ;
    
}

bool EFlagsManager::DependencyFlagConcreted(entryID instrID, bool &bExecute) {
    
    assert (isConditionalExecuteInstr(instrID)) ;
    bool ret = false;
    switch (instrID)
    {
        case e_jnb:
        case e_jnb_jae_j:
        case e_cmovnb:
        case e_setnb:
            // AE/NB/No Carry;		(CF=0)
            ret = (m_VM->FlagBitDefinited(x86_64::cf)) ;
            if (ret) {
                FLAG_STAT cf ;
                assert(m_VM->getFlagBit(x86_64::cf, cf)) ;
                bExecute = (cf==FLAG_CLEAR) ? true : false ;
            }
            break ;

        case e_ja:
        case e_jnbe:
        case e_cmovnbe:
        case e_setnbe:
            // A/NBE; 			(CF=0 and ZF=0)
            ret = ((m_VM->FlagBitDefinited(x86_64::cf)) && (m_VM->FlagBitDefinited(x86_64::zf))) ;
            if (ret) {
                FLAG_STAT cf, zf ;
                assert(m_VM->getFlagBit(x86_64::cf, cf));
                assert(m_VM->getFlagBit(x86_64::zf, zf));

                bExecute = (cf==FLAG_CLEAR && zf==FLAG_CLEAR) ? true : false ;
            }
            break ;

        case e_jbe:
        case e_cmovbe:
        case e_setbe:
            // BE/NA;			(CF=1 or ZF=1)
            ret = ((m_VM->FlagBitDefinited(x86_64::cf)) && (m_VM->FlagBitDefinited(x86_64::zf))) ;
            if (ret) {
                FLAG_STAT cf, zf ;
                assert(m_VM->getFlagBit(x86_64::cf, cf));
                assert(m_VM->getFlagBit(x86_64::zf, zf));

                // bExecute = (cf==FLAG_SET && zf==FLAG_SET) ? true : false ;
                bExecute = (cf==FLAG_SET || zf==FLAG_SET) ? true : false ;
            }
            break ;
        
        case e_jb:
        case e_jb_jnaej_j:
        case e_cmovnae:
        case e_sbb:
        case e_setb:
            // B/NAE/Carry;		(CF=1)
            ret = (m_VM->FlagBitDefinited(x86_64::cf)) ;
            if (ret) {
                FLAG_STAT cf;
                assert(m_VM->getFlagBit(x86_64::cf, cf));

                bExecute = (cf==FLAG_SET) ? true : false ;
            }            
            break ;
        
        case e_jge:
        case e_jnl:
        case e_cmovnl:
        case e_setnl:
            // GE/NL;			(SF=OF)
            ret = ((m_VM->FlagBitDefinited(x86_64::sf)) && (m_VM->FlagBitDefinited(x86_64::of))) ;
            if (ret) {
                FLAG_STAT sf, of ;
                assert(m_VM->getFlagBit(x86_64::sf, sf));
                assert(m_VM->getFlagBit(x86_64::of, of));

                bExecute = (sf == of) ? true : false ;
            }
            break ;

        case e_jnle:
        case e_jg:
        // case e_cmovnle:
        case e_setnle:
            // G/NLE;			(ZF=0 and SF=OF)
            ret = ((m_VM->FlagBitDefinited(x86_64::zf)) && 
                    (m_VM->FlagBitDefinited(x86_64::sf)) && 
                    (m_VM->FlagBitDefinited(x86_64::of))) ;
            if (ret) {
                FLAG_STAT zf, sf, of;
                assert(m_VM->getFlagBit(x86_64::zf, zf));
                assert(m_VM->getFlagBit(x86_64::sf, sf));
                assert(m_VM->getFlagBit(x86_64::of, of));

                bExecute = (zf==FLAG_CLEAR && sf==of) ? true : false ;
            }                    
            break ;
        
        case e_jle:
        case e_cmovng:
        case e_setle:
            // LE/NG;			(ZF=1 or SF≠ OF)
            ret = ((m_VM->FlagBitDefinited(x86_64::zf)) && 
                    (m_VM->FlagBitDefinited(x86_64::sf)) && 
                    (m_VM->FlagBitDefinited(x86_64::of))) ;
            if (ret) {
                FLAG_STAT zf, sf, of;
                assert(m_VM->getFlagBit(x86_64::zf, zf));
                assert(m_VM->getFlagBit(x86_64::sf, sf));
                assert(m_VM->getFlagBit(x86_64::of, of));

                bExecute = (zf==FLAG_SET && sf!=of) ? true : false ;
            } 
            break ;

        case e_jl:
        case e_cmovnge:
        case e_setl:
            // L/NGE; 			(SF≠ OF)
            ret = ((m_VM->FlagBitDefinited(x86_64::sf)) && (m_VM->FlagBitDefinited(x86_64::of))) ;
             if (ret) {
                FLAG_STAT sf, of ;
                assert(m_VM->getFlagBit(x86_64::sf, sf));
                assert(m_VM->getFlagBit(x86_64::of, of));

                bExecute = (sf != of) ? true : false ;
            }
            break ;   

        case e_je:
        case e_jz:
        case e_cmove:
        case e_setz:
            ret = (m_VM->FlagBitDefinited(x86_64::zf)) ;
            if (ret) {
                FLAG_STAT zf ;
                assert(m_VM->getFlagBit(x86_64::zf, zf));
 
                bExecute = (zf == FLAG_SET) ? true : false ;
            } 
            break ;

        case e_jne:
        case e_jnz:
        case e_cmovne:
        case e_setnz:
            // E/Z/NE/NZ (zf)
            ret = (m_VM->FlagBitDefinited(x86_64::zf)) ;
            if (ret) {
                FLAG_STAT zf ;
                assert(m_VM->getFlagBit(x86_64::zf, zf));
 
                bExecute = (zf == FLAG_CLEAR) ? true : false ;
            } 
            break ;

        case e_jns:
        case e_cmovns:
        case e_setns:
            ret = (m_VM->FlagBitDefinited(x86_64::sf)) ;
            if (ret) {
                FLAG_STAT sf ;
                assert(m_VM->getFlagBit(x86_64::sf, sf));
 
                bExecute = (sf == FLAG_CLEAR) ? true : false ;
            } 
            break ;    

        case e_js:
        case e_cmovs:
        case e_sets:
            // S/NS (sf)
            ret = (m_VM->FlagBitDefinited(x86_64::sf)) ;
            if (ret) {
                FLAG_STAT sf ;
                assert(m_VM->getFlagBit(x86_64::sf, sf));
 
                bExecute = (sf == FLAG_SET) ? true : false ;
            } 
            break ;

        case e_jno:
        case e_cmovno:
        case e_setno:
            ret = (m_VM->FlagBitDefinited(x86_64::of)) ;
            if (ret) {
                FLAG_STAT of ;
                assert(m_VM->getFlagBit(x86_64::of, of));
 
                bExecute = (of == FLAG_CLEAR) ? true : false ;
            }             
            break ;

        case e_jo:
        case e_cmovo:
        case e_seto:
            // O/NO (of)
            ret = (m_VM->FlagBitDefinited(x86_64::of)) ;
            if (ret) {
                FLAG_STAT of ;
                assert(m_VM->getFlagBit(x86_64::of, of));
 
                bExecute = (of == FLAG_SET) ? true : false ;
            }             
            break ;

        case e_jrcxz:
        case e_jcxz_jec:
        case e_jp:
        case e_jnp:
        case e_cmovpe:
        case e_cmovpo:
        case e_setp:
        case e_setnp:
 //       case e_jmp:
 //       case e_jmpq:
 //       case e_jmpe:

            assert(0);
            // asm("int3");
            break ;   

        default:
            std::cout << "QHQQHQHQHQ" << instrID << std::endl ;
            assert(0);
            // asm("int3");
            break;
    }
    return (ret) ;
}



// instructions only for set flags.
bool EFlagsManager::isFlagSettingInstr(entryID id){
    if (instrAttr[id] & FLAG_SETING_ATTR) {
        return true ;
    }
    return false;
}

// instructions which will change flags.
bool EFlagsManager::isFlagChangingInstr(entryID id){
    int i = 0;

    if(instrAttr[id] & FLAG_CHANGING_ATTR) {
        return true ;
    }
    return false;
}

// insturctions will depend on flags
bool EFlagsManager::isConditionalExecuteInstr(entryID id){
    if(instrAttr[id] & CONDITIONAL_EXEC_ATTR) {
        return true ;
    }
    return false;
}

void EFlagsManager::InitInstructionAttr()
{
    for (int i =0; i< sizeof(instrAttr)/sizeof(instrAttr[0]); i++){
        instrAttr[i] = 0 ;
        for (int j=0; j<sizeof(stateChangingInstrList)/sizeof(stateChangingInstrList[0]); j++) {
            if(i == stateChangingInstrList[j]) {
                instrAttr[i] |= FLAG_CHANGING_ATTR;
                break ;
            }
        }
        if (i == e_cmp ||i == e_cmpw || i == e_test || i == e_bt) {
            instrAttr[i] |= FLAG_SETING_ATTR ;
        }
        if((i >= e_jb && i <= e_jrcxz && i != e_jmp) || 
            (i>=e_cmovbe && i<=e_cmovs) ||
            (i>=e_setb && i<=e_setz)    ||
            (i==e_adc || i==e_sbb || i==e_sbbl) ) {
                instrAttr[i] |= CONDITIONAL_EXEC_ATTR ;
        }
    }
}

EFlagsManager::EFlagsManager(VMState *vm) {
    
    InitInstructionAttr () ;
    m_VM = vm;
    m_Constraint.clear();
    m_LastExpr = NULL ;
    m_LastInstr = NULL ;
    
    m_Z3Handler.reset(new Z3Handler());
    
    m_AddrBool.clear() ;
    ReadAddrBoolMapFromFile() ;
};

bool EFlagsManager::PrintConstraint (void) {

    for(auto it : m_Constraint) {
        
        it->print() ;
        std::cout << "\n" ;
    }

    return true ;
}

bool EFlagsManager::SolveConstraints () {

    std::map<std::string, unsigned long long> ret_result;
    ret_result = m_Z3Handler->Z3SolveOne(m_Constraint); // now the [symbolic name, concrete value] map will be returned
#ifdef _DEBUG_OUTPUT
    printf ("result size: %d. \n", ret_result.size());
    for (auto it = ret_result.begin(); it != ret_result.end(); it ++){
        std::cout << "symbol : " << it->first << "   value : " << it->second << std::endl;
    }
#endif
    return true ;
}

bool EFlagsManager::ReadAddrBoolMapFromFile () {

    ifstream theFile;
    string fname = "/home/neo/smu/kernel-se/k-test/addr-bool_map.txt";
    string line ;
    long long unsigned int key ;
    int val;
    bool boolval;
    long long unsigned int  counter = 0, dup = 0;

    theFile.open (fname);

    if(!theFile) {
        std::cout << "error open file!" << std::endl ;
        return false ;
    }
    
    while (std::getline(theFile, line)) {
        
        //std::cout << line.c_str() << std::endl;
        //std::cout << std::endl;

        sscanf(line.c_str(), "%llx, %d", &key, &val) ;
        //std::cout << std::hex << key << " : " << val << std::endl ;
        boolval = (val==1) ;

        //auto it = m_AddrBool.find(key);
        
        //if (it!=m_AddrBool.end()) {
        //    dup ++ ;
            //std::cout << "DUP" << std::endl ;
        //}
        counter ++ ;

        key = key & 0xFFFFFFFFFFFF ;
        key = key | (counter << 48) ;
        m_AddrBool[key] = boolval ;

    }
    std::cout << "map created : " << counter << std::endl ;
    //std::cout << counter << std::endl ;

    //counter = 0 ;

    //for (auto it: m_AddrBool) {
    //    std::cout << std::hex << it.first << " : " << it.second << std::endl ;
    //    counter ++ ;
    //}

    //std::cout << std::dec << counter << " dup " << dup << std::endl ;

    theFile.close();
    return true;
}

bool EFlagsManager::findDecision(ulong addr, long long unsigned int counter) {
    long long unsigned int key = addr;
    
    key = key & 0xFFFFFFFFFFFF ;
    key = key | (counter << 48) ;
    return m_AddrBool[key];
}

bool EFlagsManager::EvalCondition(entryID insnID)
{
    bool bExecute = false;

    //pp-s
    std:cout << "inst id : " << insnID << std::endl;
    //pp-e
    std::set<KVExprPtr> constraints;
    KVExprPtr exprPtr = GetCondition(insnID);
    if (exprPtr)
    {
        std::vector<VMState::SYMemObject*> symobjs;
        for (auto it : m_VM->m_SYMemObjects)
        {
            symobjs.push_back(it.second);
        }
        
        // KVExprPtr exprPtrtmp = exprPtr;
        //exprPtr.reset(new LNotExpr(exprPtr, exprPtr->size, exprPtr->offset)) ;
        // exprPtr->print() ;
        // std::cout << "\n" ;
        // constraints.insert(exprPtr);
        // bExecute = m_Z3Handler->Z3SolveConcretize(symobjs, constraints);
        // std::cout << "((((((((((((bExecute: " << bExecute << std::endl; 
        // constraints.clear();
        // exprPtr = exprPtrtmp;

        
        constraints.insert(exprPtr);
        
        std::cout << "constraints : \n";
        for(auto it : constraints)
        {
            it->print();
            std::cout << "\n";
        }

        bExecute = m_Z3Handler->Z3SolveConcritize(symobjs, constraints);
        printf("ret from Z3SolveConcritize \n");
        //std::cout << "----------bExecute: " << bExecute << std::endl; 
        //exprPtr->print() ;
        //std::cout << "\n" ;
#ifdef DEBUG_LOG
        std::cout << "----------bExecute: " << bExecute << std::endl; 
        exprPtr->print() ;
        std::cout << "\n" ;
#endif

        if (!bExecute)
            exprPtr.reset(new LNotExpr(exprPtr, exprPtr->size, exprPtr->offset));
        

#ifdef _DEBUG_OUTPUT 
        std::cout << "----------bExecute: " << bExecute << std::endl; 
        exprPtr->print() ;
        std::cout << "\n" ;
#endif
        
        m_Constraint.insert(exprPtr) ;

#ifdef _SYM_DEBUG_OUTPUT
        // exprPtr->print() ;
        // std::cout << "\n" ;
#endif
    }
    else
    {
        std::cout << "Get Expr for conditional insn failed! " << std::endl;
        assert(false);
    }
    return bExecute;
}

KVExprPtr EFlagsManager::DoGetCondition(int exprID) {
    
    extern KVExprPtr CreateExprByID(int id, KVExprPtr R, KVExprPtr M, KVExprPtr L, int size = 4, int offset = 0) ;
    KVExprPtr cstnt = NULL ;

    if (m_LastExpr)
        cstnt = (CreateExprByID(exprID, m_LastExpr, NULL, NULL, m_LastExpr->size, m_LastExpr->offset));
    else
        assert(0);
        // asm("int3") ;

    // EvalDecision()

    // if (!bExecute)
    //     cstnt.reset(new LNotExpr(cstnt, cstnt->size, cstnt->offset)) ;
    if(cstnt)
        std::cout << "not null" << std::endl;
    
    return cstnt ;
}

KVExprPtr EFlagsManager::GetCondition(entryID instrID) {
    //pp-s
    std:cout << "at GetCondition ; inst id : " << instrID << std::endl;
    //pp-e
    int exprID = EXPR_UNDEFINED ;
    if (!isConditionalExecuteInstr(instrID)) 
        // return false ;
        return nullptr;

    switch (instrID)
    {
        case e_jnb:
        case e_jnb_jae_j:
        case e_cmovnb:
        case e_setnb:
            // AE/NB/No Carry;		(CF=0)
            exprID = EXPR_Uge ;
            break ;

        case e_ja:
        case e_jnbe:
        case e_cmovnbe:
        case e_setnbe:
            // A/NBE; 			(CF=0 and ZF=0)
            exprID = EXPR_Ugt ;
            break ;

        case e_jbe:
        case e_cmovbe:
        case e_setbe:
            // BE/NA;			(CF=1 or ZF=1)
            exprID = EXPR_Ule ;
            break ;
        
        case e_jb:
        case e_jb_jnaej_j:
        case e_cmovnae:
        case e_setb:
        case e_sbb:
            // B/NAE/Carry;		(CF=1)
            exprID = EXPR_Ult ;
            break ;
        
        case e_jge:
        case e_jnl:
        case e_cmovnl:
        case e_setnl:
            // GE/NL;			(SF=OF)
            exprID = EXPR_Sge ;
            break ;

        
        case e_jnle:
        case e_jg:
        // case e_cmovnle:
        case e_setnle:
            // G/NLE;			(ZF=0 and SF=OF)
            exprID = EXPR_Sgt ;
            break ;
        
        case e_jle:
        case e_cmovng:
        case e_setle:
            // LE/NG;			(ZF=1 or SF≠ OF)
            exprID = EXPR_Sle ;
            break ;
        
        case e_jl:
        case e_cmovnge:
        case e_setl:
            // L/NGE; 			(SF≠ OF)
            exprID = EXPR_Slt ;
            break ;

        case e_je:
        case e_jz:
        case e_cmove:
        case e_setz:
            exprID = EXPR_Equal ;
            break ;

        case e_jne:
        case e_jnz:
        case e_cmovne:
        case e_setnz:
            // E/Z/NE/NZ (zf)
            exprID = EXPR_Distinct ;
            break ;

        case e_jns:
        case e_cmovns:
        case e_setns:
            exprID = EXPR_NoSign ;
            break ;

        case e_js:
        case e_cmovs:
        case e_sets:
            // S/NS (sf)
            exprID = EXPR_Sign ;
            break ;

        case e_jno:
        case e_cmovno:
        case e_setno:
            exprID = EXPR_NoOverflow ;
            break ;

        case e_jo:
        case e_cmovo:
        case e_seto:
            // O/NO (of)
            exprID = EXPR_Overflow ;
            break ;

        case e_jrcxz:
        case e_jcxz_jec:
        case e_jp:
        case e_jnp:
        case e_cmovpe:
        case e_cmovpo:
        case e_setp:
        case e_setnp:
 //       case e_jmp:
 //       case e_jmpq:
 //       case e_jmpe:

            assert(0);
            // asm("int3");
            break ;   

        default:
            assert(0);
            // asm("int3");
            break;
    }
    
    if(exprID != EXPR_UNDEFINED) {
        std::cout << " exprid " << exprID << std::endl ;
        return DoGetCondition (exprID) ;
        // return true ;
    } else
        // return (false) ;
        return nullptr;
    
}



