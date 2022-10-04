

#include <linux/types.h>
#include <signal.h>
#include <ucontext.h>

#include <iostream>

#include "CodeObject.h"
#include "InstructionDecoder.h"
#include "interface.h"
#include "xcode.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;

int main(int argc, char **argv) {
    Instruction instr;

    const unsigned char *hexstream = (const unsigned char *)"\x41\x5c\x48\x83\xc4\x08\xc3";

    // create an Instruction decoder which will convert the binary opcodes to strings
    InstructionDecoder decoder(hexstream, InstructionDecoder::maxInstructionLength,
                               Architecture::Arch_x86_64);
    // for(;fit != all.end(); ++fit){
    // 	Function *f = *fit;
    // get address of entry point for current function
    Address crtAddr = 0x400000;
    // int instr_count = 0;
    // instr = decoder.decode();
    // auto fbl = f->blocks().end();
    // fbl--;
    // Block *b = *fbl;
    Address lastAddr = crtAddr + 7;
    // if current function has zero instructions, d o n t output it
    //  if(crtAddr == lastAddr)
    //  	continue;
    //  cout << "\n\n\"" << f->name() << "\" :";
    ulong cursor = 0;
    while (crtAddr < lastAddr) {
        // decode current instruction
        instr = decoder.decode(hexstream + cursor);
        cout << hex << crtAddr << ": ";
        cout << instr.format() << "\n";
        // go to the address of the next instruction
        crtAddr += instr.size();
        cursor += instr.size();
        // instr_count++;

        SymbolicQuery(&instr);
    }
    return 0;
}
