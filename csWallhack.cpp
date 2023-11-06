#include <Windows.h>

#define STRUCT_SIZE 0x38

struct offsets {
    DWORD player = 0x04E051DC;
    DWORD playerHealth = 0x100;
    DWORD entityList = 0x04E051DC;
    DWORD team = 0xF4;
    DWORD glowObj = 0x535FCB8;
    DWORD glowIndex = 0x10488;
};

void wallHack() {
    bool wallsEnabled = true;
    offsets offsets;
    DWORD module = (DWORD)GetModuleHandle(L"client.dll");
    // don't continue until we have successfully retrieved the handle
    while (module == NULL) {
        module = (DWORD)GetModuleHandle(L"client.dll");
    }
    DWORD* player = (DWORD*)(module + offsets.player);
    DWORD* glowManager = (DWORD*)(module + offsets.glowObj);
    int playerTeam = *(int*)(*player + offsets.team);
    while (true) {
        // toggle one and off
        if (GetAsyncKeyState(VK_END)) {
            wallsEnabled = !wallsEnabled;
        }
        if (wallsEnabled) {
            // skip over first in list since that is the person hacking
            for (int i = 1; i < 10; i++) {
                DWORD* entity = (DWORD*)(module + offsets.entityList + (0x10 * i));
                DWORD* entityHealth = (DWORD*)(*entity + offsets.playerHealth);
                DWORD* entityTeam = (DWORD*)(*entity + offsets.team);
                DWORD* entityGlowInt = (DWORD*)(*entity + offsets.glowIndex);

                if (*entityHealth > 0 && *entityTeam != playerTeam) {
                    float* red = (float*)(*glowManager + (*entityGlowInt * STRUCT_SIZE) + 0x8);
                    *red = 1.5f;
                }
                else {
                    float* blue = (float*)(*glowManager + (*entityGlowInt * STRUCT_SIZE) + 0x10);
                    *blue = 1.5f;
                }
                float* alpha = (float*)(*glowManager + (*entityGlowInt * STRUCT_SIZE) + 0x14);
                *alpha = 1.5f;
                bool* throughWalls = (bool*)(*glowManager + (*entityGlowInt * STRUCT_SIZE) + 0x28);
                *throughWalls = true;
                bool* notThroughWalls = (bool*)(*glowManager + (*entityGlowInt * STRUCT_SIZE) + 0x29);
                *notThroughWalls = false;
            }
        }
        Sleep(1);
    }
}


BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {   
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)wallHack, NULL, 0, NULL);
    }
    return true;
}