// #include "CppUTest/CommandLineTestRunner.h"
// #include "CppUTest/MemoryLeakDetectorMallocMacros.h"
// #include "CppUTest/TestHarness.h"

#include <linux/types.h>
#include <signal.h>
#include <ucontext.h>

#include <fstream>
#include <iostream>

#include "BPatch.h"
#include "BPatch_basicBlock.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
#include "CodeObject.h"
#include "InstructionDecoder.h"
#include "dyn_regs.h"

#define private public
#include "ExecCtrl.h"
#include "VMState.h"
#include "thinctrl.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;

#define elf_file "./wrapper"
#define elf_code " void wrapper() { asm(\"MYINSTR\"); }\n int main(int argc, char* argv[]) { return 0; }"

bool string_replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

Instruction* gen_instruction(const char* instr) {
    // Genereate ELF code
    string code = elf_code;
    string_replace(code, "MYINSTR", instr);

    // Remove legacy files
    string szRemove = "rm -rf wrapper.c ";
    szRemove += elf_file;
    system(szRemove.c_str());

    // Generate file
    ofstream of;
    of.open("wrapper.c", ios::out | ios::trunc);
    of << code << "\n";
    of.close();

    // Compile file
    string szCompile = "gcc wrapper.c -o ";
    szCompile += elf_file;
    system(szCompile.c_str());

    // Use dyninst to parsing the generated file
    BPatch bpatch;
    BPatch_binaryEdit* appBin = bpatch.openBinary(elf_file, false);
    if (appBin == NULL) {
        cerr << "Failed to open binary" << endl;
        exit(EXIT_FAILURE);
    }
    BPatch_image* appImage = appBin->getImage();

    BPatch_Vector<BPatch_function*> funcs;
    if (appImage->findFunction("wrapper", funcs) == NULL) {
        cerr << "Failed to find function *myasm*" << endl;
        exit(EXIT_FAILURE);
    }

    // Parsing functions & get basic blocks
    BPatch_function* F = funcs[0];
    BPatch_flowGraph* cfg = F->getCFG();
    std::vector<BPatch_basicBlock*> entries;
    if (!cfg->getEntryBasicBlock(entries)) {
        cerr << "Failed to get basic block" << endl;
        return NULL;
    }
    BPatch_basicBlock* B = entries[0];

    // Parsing instructions
    std::vector<Dyninst::InstructionAPI::Instruction> insns;
    if (!B->getInstructions(insns))
        return NULL;
    // for (auto I : insns) {
    //     cout << I.format() << endl;
    // }

    // Assume the third instruction is what we want
    assert(insns.size() >= 3);
    Instruction* I = new Instruction(insns[2]);

    return I;
}

VMState* VM;
CThinCtrl* T;

void test_init(void) {
    VM = new VMState();
    T = new CThinCtrl(VM);
}

void test_fini(void) {
    if (VM) delete VM;
    if (T) delete T;
}

bool test_je(void) {
    // je $2
    Instruction* I = gen_instruction(".byte 0x74, 0x02");
    if (I != NULL) {
        cout << I->format() << endl;
    }

    InstrInfo* ioi = new InstrInfo(I);
    T->parseOperands(ioi);

    auto& vecOI = ioi->vecOI;
    OprndInfoPtr& oitgt = vecOI[0];

    assert(oitgt->rdwr == OPAC_RD);
    assert((oitgt->opty & OPTY_REG) == OPTY_REG);
    assert(oitgt->reg_index == (uint)x86_64::rip);
    assert(oitgt->reg_conval == 0x4);
    cout << __PRETTY_FUNCTION__ << " -> Successful" << endl;
}

bool test_lea(void) {
    // lea 0x10(%rip),%rdi
    Instruction* I = gen_instruction(".byte 0x48, 0x8d, 0x3d, 0x10, 0x00, 0x00, 0x00");
    if (I != NULL) {
        cout << I->format() << endl;
    }

    InstrInfo* ioi = new InstrInfo(I);
    T->parseOperands(ioi);

    auto& vecOI = ioi->vecOI;
    OprndInfoPtr& oidst = vecOI[0];
    OprndInfoPtr& oisrc = vecOI[1];

    assert(oidst->rdwr == OPAC_WR);
    assert((oidst->opty & OPTY_REG) == OPTY_REG);
    assert(oidst->reg_index == (uint)x86_64::rdi);

    assert(oisrc->rdwr == OPAC_RD);
    assert((oisrc->opty & OPTY_REG) == OPTY_REG);
    assert(oisrc->reg_index == (uint)x86_64::rip);
    assert(oisrc->reg_conval == 0x17);
    cout << __PRETTY_FUNCTION__ << " -> Successful" << endl;
}

bool test_call(void) {
    // call
    Instruction* I = gen_instruction(".byte 0xe8, 0xb5, 0xfe, 0xff, 0xff");
    if (I != NULL) {
        cout << I->format() << endl;
    }

    RegValue V{(uint)x86_64::rip, 8, false, 0x656};
    VM->writeRegister(V);
    InstrInfo* ioi = new InstrInfo(I);
    T->parseOperands(ioi);

    auto& vecOI = ioi->vecOI;
    OprndInfoPtr& oitgt = vecOI[0];

    assert(oitgt->rdwr == OPAC_RD);
    assert((oitgt->opty & OPTY_REG) == OPTY_REG);
    assert(oitgt->reg_index == (uint)x86_64::rip);
    assert(oitgt->reg_conval == 0x510);
    cout << __PRETTY_FUNCTION__ << " -> Successful" << endl;
}

int main(int ac, char** av) {
    test_init();
    // test_je();
    // test_lea();
    test_call();
    test_fini();
}
