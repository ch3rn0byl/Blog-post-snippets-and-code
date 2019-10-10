#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include <string>

std::string GetLastErrorAsString()
{
	LPSTR messageBuffer = NULL;

	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, 
		GetLastError(), 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPSTR)& messageBuffer, 
		0, 
		NULL
	);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);
	return message;
}

LPVOID getBaseAddress(const char* drivername)
{
	LPVOID drivers[1024] = { 0 };
	DWORD cbNeeded = 0;
	int nDrivers = 0;

	// Enumerate the system for all the available drivers
	if (EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded) && cbNeeded < sizeof(drivers));
	{
		char szDrivers[1024] = { 0 };
		nDrivers = cbNeeded / sizeof(drivers[0]);

		for (unsigned int i = 0; i < nDrivers; i++)
		{
			// Lets check to see if we found the name of whatever we're looking for
			if (GetDeviceDriverBaseNameA(drivers[i], szDrivers, sizeof(szDrivers) / sizeof(szDrivers[0])))
			{
				if (strcmp(szDrivers, drivername) == 0)
				{
					// Boom, found it
					return drivers[i];
				}
			}
		}
	}
	return NULL;
}

int main(int argc, char* argv[])
{
	const char* lpFileName = NULL;

	if (argc != 2)
	{
		std::wcerr << "[!] Usage: " << argv[0] << " <driver/dll name>" << std::endl;
		return EXIT_FAILURE;
	}
	lpFileName = argv[1];

	std::cout << "[+] Getting base address for " << lpFileName << "...";
	LPVOID baseaddress = getBaseAddress(lpFileName);
	if (baseaddress == NULL && GetLastError() != 0)
	{
		std::cerr << "uh-oh!" << std::endl;
		std::cerr << "[!] " << GetLastErrorAsString();
		return EXIT_FAILURE;
	}
	else if (baseaddress == NULL && GetLastError() == 0)
	{
		std::cerr << "uh-oh!" << std::endl;
		std::cerr << "[!] Unable to find " << lpFileName << std::endl;
		return EXIT_FAILURE;
	}
	else
	{
		std::cout << "got it!" << std::endl;
	}

	std::cout << "[+] Base Address: " << baseaddress << std::endl;
	return EXIT_SUCCESS;
}
