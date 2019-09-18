#include <Windows.h>
#include <iostream>
#include <string>

int main()
{
	uint8_t obfuscated[] = { 
		0x87, 0xea, 0xfd, 0x9a, 0x4b, 0x73, 
		0x54, 0xa4, 0x5c, 0x8f, 0x00 
	};

	std::string result;
  
  uint8_t offset = 0;
	uint16_t key = 0x5555;
  
	while (obfuscated[offset] != 0)
	{
		uint8_t value = obfuscated[offset];
		uint8_t value2 = value;

		key = (key << 2) + offset;
		value2 = value2 >> 6;

		if (value2 - 1 <= 2)
		{
			value ^= key;
			value -= offset;
			value -= value2;
			value &= 0x3f;

			if (value >= 0x0a)
			{
				if (value <= 0x24)
				{
					result.push_back(value + 0x37);
				}
				else
				{
					if (value <= 0x3e)
					{
						result.push_back(value + 0x3d);
					}
				}
			}
			else
			{
				result.push_back(value + 0x30);
			}
		}
		offset++;
	}

	std::cout << result << std::endl;
}
