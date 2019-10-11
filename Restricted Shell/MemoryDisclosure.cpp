#include <Windows.h>
#include <iostream>

bool sendrequest(HANDLE hFile)
{
	const int nSize = 5 * sizeof(DWORD64);

	DWORD dwIoControlCode = 0x22203f;
	LPVOID lpInBuffer[nSize] = { 0 };
	DWORD nInBufferSize = 0x13;
	LPVOID lpOutBuffer[nSize] = { 0 };
	DWORD nOutBufferSize = 0x37;
	DWORD lpBytesReturned = 0;

	// Fill up our buffers with C's and D's to distinguish what goes where
	FillMemory(lpInBuffer, nSize, 0x43);
	FillMemory(lpOutBuffer, nSize, 0x44);

	bool status = DeviceIoControl(hFile, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, &lpBytesReturned, NULL);

	// Not an ideal way, but for example purposes
	std::cout << "[+] Contents of Output Buffer: ";
	for (auto i : lpOutBuffer)
	{
		if (i != 0)
		{
			std::cout << i;
		}
	}
	std::cout << std::endl;

	return status;
}

int main()
{
	std::cout << "[+] Grabbing handle to HEVD...";
	HANDLE hFile = CreateFile(L"\\\\.\\HackSysExtremeVulnerableDriver", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		std::cerr << "[!] CreateFile error: " << GetLastError() << std::endl;
		return EXIT_FAILURE;
	}
	else
		std::cout << "got it!" << std::endl;

	std::cout << "[+] Sending DeviceIoControl request..." << std::endl;
	if (!sendrequest(hFile))
	{
		std::cerr << "uh-oh!" << std::endl;
		std::cerr << "[!] DeviceIoControl error: " << GetLastError() << std::endl;
		CloseHandle(hFile);
		return EXIT_FAILURE;
	}

	std::cout << "[+] Done!" << std::endl;
	CloseHandle(hFile);
	return EXIT_SUCCESS;
}
