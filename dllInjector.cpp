#include <windows.h>
#include <tlhelp32.h>
#include <memoryapi.h>

int main() {
	// get all the running processes
	HANDLE runningProcesses = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32);
	Process32First(runningProcesses, &processEntry);
	DWORD csgoProcessId = NULL;
	const wchar_t* processName = L"csgo.exe";
	const char* dllAddr = "C:\\Users\\dinve\\source\\repos\\InternalMemHack2\\Debug\\InternalMemHack2.dll";
	if (wcscmp(processEntry.szExeFile, processName) == 0) {
		csgoProcessId = processEntry.th32ProcessID;
	}
	else {
		// loop through and break once we come across csgo.exe
		while (Process32Next(runningProcesses, &processEntry)) {
			if (wcscmp(processEntry.szExeFile, processName) == 0) {
				csgoProcessId = processEntry.th32ProcessID;
				break;
			}
		}
	}
	if (csgoProcessId == NULL) {
		exit(1);
	}
	HANDLE csgoProcess = OpenProcess(PROCESS_ALL_ACCESS, true, csgoProcessId);
	// We don't wanmt to risk corrupting game memory so we allocate virtual memory. 
	LPVOID virtualMem = VirtualAllocEx(csgoProcess, NULL, strlen(dllAddr) + 1, MEM_COMMIT, PAGE_READWRITE);
	if (virtualMem == NULL) {
		exit(1);
	}
	// write the dll path into the virtual memory created
	WriteProcessMemory(csgoProcess, virtualMem, dllAddr, strlen(dllAddr) + 1, NULL);
	// we need to call LoadLibraryA through the kernel
	HMODULE kernel = GetModuleHandle(L"kernel32.dll");
	if (kernel != NULL) {
		FARPROC lib = GetProcAddress(kernel, "LoadLibraryA");
		if (lib != NULL) {
			// create thread inside csgo.exe at the memory address our dll path is loaded in and call force the game to call LoadLibraryA
			HANDLE thread = CreateRemoteThread(csgoProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lib, virtualMem, NULL, NULL);
			if (thread != NULL) {
				// wait for the thread to finish execution
				WaitForSingleObject(thread, INFINITE);
				CloseHandle(thread);
			}
			// free allocated memory and clean up handles that are still open
			VirtualFreeEx(csgoProcess, virtualMem, 0, MEM_RELEASE);
			CloseHandle(csgoProcess);
			CloseHandle(runningProcesses);
		}
	}
	
	return 0;
}