#include "memory.hpp"

int main() {

    uintptr_t rebase =          0x140000000;                        // rebase
    uintptr_t main =            0x14000150B;                        // main
    uintptr_t patch =           0x140001554;                        // patch
    uintptr_t function1 =       0x140001450;                        // function1
    uintptr_t function2 =       0x14000148B;                        // function2
    uintptr_t function3 =       0x1400014C3;                        // function3
    uintptr_t nop1 =            0x140001545;                        // nop address1
    uintptr_t nop2 =            0x14000154F;                        // nop address2
    uintptr_t sleep =           0x140001518;                        // sleep address

    Memory memory("Target.exe");
    uintptr_t baseAddress = memory.getModuleBase("Target.exe");

    uintptr_t function1Address = baseAddress + function1 - rebase;  // function1
    uintptr_t function2Address = baseAddress + function2 - rebase;  // function2
    uintptr_t function3Address = baseAddress + function3 - rebase;  // function3
    uintptr_t patchAddress = baseAddress + patch - rebase;          // patch in main
    uintptr_t returnAddress = patchAddress + 0x10;                  // return address to after patch
    uintptr_t nop1Address = baseAddress + nop1 - rebase;            // address1 of 5 bytes to nop
    uintptr_t nop2Address = baseAddress + nop2 - rebase;            // address2 of 5 bytes to nop
    uintptr_t sleepAddress = baseAddress + sleep - rebase;          // sleep address

    unsigned char patchData[15] = {
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0 ; index 2
        0xFF, 0xE0,                                                 // jmp rax
        0x90, 0x90, 0x90                                            // nop
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
        0x90, 0x90, 0x90, 0x90, 0x90                                // nop
    };
    
    unsigned char nop2Data[5] = { 
        0x90, 0x90, 0x90, 0x90, 0x90                                // nop
    };

    unsigned char sleepData[7] = { 
        0xC7, 0x45, 0xF8, 0x02, 0x00, 0x00, 0x00                    // mov dword ptr [rbp-8], 2
    };

    BOOL writeNop1 = memory.writeEx(nop1Address, nop1Data, sizeof(nop1Data));
    BOOL writeNop2 = memory.writeEx(nop2Address, nop2Data, sizeof(nop2Data));
    BOOL writeSleep = memory.writeEx(sleepAddress, sleepData, sizeof(sleepData));

    const char* szStr = "Goodbye, World!";
    LPVOID szStrAddress = memory.allocateEx(strlen(szStr));
    LPVOID detourAddress = memory.allocateEx(sizeof(detourData));

    *(uintptr_t *)&detourData[2] = (uintptr_t) szStrAddress;
    *(uintptr_t *)&detourData[12] = (uintptr_t) function1Address;
    *(uintptr_t *)&detourData[29] = (uintptr_t) function2Address;
    *(uintptr_t *)&detourData[51] = (uintptr_t) function3Address;
    *(uintptr_t *)&detourData[63] = (uintptr_t) returnAddress;
    *(uintptr_t *)&patchData[2] = (uintptr_t) detourAddress;

    BOOL writeStr = memory.writeEx(reinterpret_cast<uintptr_t>(szStrAddress), 
    (LPVOID) szStr, strlen(szStr));

    BOOL writeDetour = memory.writeEx(reinterpret_cast<uintptr_t>
    (detourAddress), detourData, sizeof(detourData));

    BOOL writePatch = memory.writeEx(patchAddress, patchData, sizeof(patchData));

    return 0;
}