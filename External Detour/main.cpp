#include "memory.hpp"

int main() {

    uintptr_t rebase =          0x140000000; // rebase
    uintptr_t main =            0x14000150B; // main
    uintptr_t function  =       0x1400014C3; // function
    uintptr_t patch =           0x140001554; // patch

    Memory memory("Target.exe");
    uintptr_t baseAddress = memory.getModuleBase("Target.exe");
    uintptr_t functionAddress = baseAddress + function - rebase;
    uintptr_t patchAddress = baseAddress + patch - rebase;
    uintptr_t returnAddress = patchAddress + 0x10;

    unsigned char patchData[15] = {
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0
        0xFF, 0xE0,                                                 // jmp rax
        0x90, 0x90, 0x90                                            // nop
    };

    unsigned char detourData[34] = {
        0xB9, 0x05, 0x00, 0x00, 0x00,                               // mov ecx, 5
        0xBA, 0x0A, 0x00, 0x00, 0x00,                               // mov edx, 10
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0
        0xFF, 0xD0,                                                 // call rax
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0
        0xFF, 0xE0,                                                 // jmp rax
    };

    *(uintptr_t *)&detourData[12] = functionAddress;
    *(uintptr_t *)&detourData[24] = returnAddress;

    LPVOID detourAddress = memory.allocateEx(sizeof(detourData));

    *(uintptr_t *)&patchData[2] = (uintptr_t) detourAddress;

    BOOL writeDetour = memory.writeEx(reinterpret_cast<uintptr_t>
    (detourAddress), detourData, sizeof(detourData));

    BOOL writePatch = memory.writeEx(patchAddress, patchData, sizeof(patchData));

    return 0;
}