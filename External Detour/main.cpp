#include "memory.hpp"

int main() {

    Memory memory("Target.exe");
    uintptr_t baseAddress = memory.getModuleBase("Target.exe");

    uintptr_t rebase =          0x140000000;                        // rebase
    uintptr_t patch =           0x140001554;                        // patch
    uintptr_t function1 =       0x140001450;                        // function1
    uintptr_t function2 =       0x14000148B;                        // function2
    uintptr_t function3 =       0x1400014C3;                        // function3
    uintptr_t nop1 =            0x140001545;                        // nop address1
    uintptr_t nop2 =            0x14000154F;                        // nop address2
    uintptr_t sleep1 =          0x140001518;                        // sleep address

    uintptr_t addrPatch = baseAddress + patch - rebase;             // patch in main
    uintptr_t addrFunction1 = baseAddress + function1 - rebase;     // function1
    uintptr_t addrFunction2 = baseAddress + function2 - rebase;     // function2
    uintptr_t addrFunction3 = baseAddress + function3 - rebase;     // function3
    uintptr_t addrNop1 = baseAddress + nop1 - rebase;               // address1 of 5 bytes to nop
    uintptr_t addrNop2 = baseAddress + nop2 - rebase;               // address2 of 5 bytes to nop
    uintptr_t addrSleep1 = baseAddress + sleep1 - rebase;           // sleep address
    uintptr_t addrReturn = addrPatch + 0xE;                        // return address to after patch

    unsigned char patchData[15] = {
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0 ; index 2
        0xFF, 0xE0,                                                 // jmp rax
        0x90, 0x90, 0x90                                            // nop, nop, nop
    };

    unsigned char detourData[73] = {
        0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rcx, 0 ; index 2
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0 ; index 12
        0xFF, 0xD0,                                                 // call rax

        0xB9, 0x05, 0x00, 0x00, 0x00,                               // mov ecx, 5
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0 ; index 29
        0xFF, 0xD0,                                                 // call rax

        0xB9, 0x05, 0x00, 0x00, 0x00,                               // mov ecx, 5
        0xBA, 0x0A, 0x00, 0x00, 0x00,                               // mov edx, 10
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0 ; index 51
        0xFF, 0xD0,                                                 // call rax

        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0 ; index 63
        0xFF, 0xE0,                                                 // jmp rax
    };

    unsigned char nop1Data[5] = { 
        0x90, 0x90, 0x90, 0x90, 0x90                                // nop, nop, nop, nop, nop
    };
    
    unsigned char nop2Data[5] = { 
        0x90, 0x90, 0x90, 0x90, 0x90                                // nop, nop, nop, nop, nop
    };

    unsigned char sleepData[7] = { 
        0xC7, 0x45, 0xF8, 0x02, 0x00, 0x00, 0x00                    // mov dword ptr [rbp-8], 2
    };

    BOOL writeNop1 = memory.writeEx(addrNop1, nop1Data, sizeof(nop1Data));
    BOOL writeNop2 = memory.writeEx(addrNop2, nop2Data, sizeof(nop2Data));
    BOOL writeSleep = memory.writeEx(addrSleep1, sleepData, sizeof(sleepData));

    const char* szStr = "Goodbye, World!";
    LPVOID addrStr = memory.allocateEx(strlen(szStr));
    LPVOID addrDetour = memory.allocateEx(sizeof(detourData));

    *(uintptr_t *)&detourData[2] = (uintptr_t) addrStr;
    *(uintptr_t *)&detourData[12] = (uintptr_t) addrFunction1;
    *(uintptr_t *)&detourData[29] = (uintptr_t) addrFunction2;
    *(uintptr_t *)&detourData[51] = (uintptr_t) addrFunction3;
    *(uintptr_t *)&detourData[63] = (uintptr_t) addrReturn;
    *(uintptr_t *)&patchData[2] = (uintptr_t) addrDetour;

    BOOL writeStr = memory.writeEx(reinterpret_cast<uintptr_t>(addrStr), 
    (LPVOID) szStr, strlen(szStr));

    BOOL writeDetour = memory.writeEx(reinterpret_cast<uintptr_t>(addrDetour), 
    detourData, sizeof(detourData));

    BOOL writePatch = memory.writeEx(addrPatch, patchData, sizeof(patchData));

    return 0;
}