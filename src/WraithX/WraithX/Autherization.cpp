#include "stdafx.h"

// The class we are implementing
#include "Autherization.h"

// The mask data
static const uint16_t MaskData[5] = { 0x4e25, 0xf4a1, 0x5437, 0xab41, 0x0000 };
static const uint8_t MaskNameData[10] { 0x34, 0x66, 0x21, 0x67, 0x76, 0x86, 0xFF, 0x1, 0x33, 0x44 };

uint16_t Autherization::HashMacAddress(PIP_ADAPTER_INFO Info)
{
	// Hash buffer
	uint16_t Hash = 0;
	// Loop and calculate
	for (uint32_t i = 0; i < (uint32_t)Info->AddressLength; i++)
	{
		Hash += (Info->Address[i] << ((i & 1) * 8));
	}
	// Return it
	return Hash;
}

void Autherization::GetMacHash(uint16_t& Mac1, uint16_t& Mac2)
{
	// Adapther info
	IP_ADAPTER_INFO AdapterInfo[32];
	// Get length
	DWORD BufferLen = sizeof(AdapterInfo);

	// Fetch them
	auto Status = GetAdaptersInfo(AdapterInfo, &BufferLen);

	// Verify
	if (Status != ERROR_SUCCESS) { return; }

	// Get address
	PIP_ADAPTER_INFO Adapters = AdapterInfo;
	// Calculate hash 1
	Mac1 = HashMacAddress(Adapters);
	// Check if we have a second
	if (Adapters->Next)
	{
		// Calculate second
		Mac2 = HashMacAddress(Adapters->Next);
	}

	// Sort them
	if (Mac1 > Mac2)
	{
		// Temp
		auto Tmp = Mac1;
		Mac2 = Mac1;
		Mac1 = Tmp;
	}
}

uint16_t Autherization::GetVolumeHash()
{
	// The buffer
	DWORD SerialNumber = 0;

	// Fetch the volume information
	GetVolumeInformation(L"c:\\", NULL, 0, &SerialNumber, NULL, NULL, NULL, 0);

	// Calculate and return
	return (uint16_t)((SerialNumber + (SerialNumber >> 16)) & 0xFFFF);
}

uint16_t Autherization::GetCPUHash()
{
	// A buffer for info
	int32_t CPUInfo[4] = { 0, 0, 0, 0 };
	// Grab the information
	__cpuid(CPUInfo, 0);
	// A hash buffer
	uint16_t Hash = 0;
	// Fetch a pointer to the data
	uint16_t* Ptr = (uint16_t*)(&CPUInfo[0]);
	// Iterate and hash
	for (int32_t i = 0; i < 8; i++)
	{
		Hash += Ptr[i];
	}
	// Return it
	return Hash;
}

std::string Autherization::GetMachineName()
{
	// The buffer
	char NameBuffer[1024];
	// Set
	std::memset(NameBuffer, 0, 1024);
	// Size
	DWORD BufferSize = 1024;
	// Get it
	GetComputerNameA(NameBuffer, &BufferSize);
	// Return it
	return std::string(NameBuffer);
}

void Autherization::MaskID(uint16_t* ID)
{
	// Iterate and flip the byte order
	for (auto i = 0; i < 5; i++)
	{
		for (auto j = i; j < 5; j++)
		{
			if (i != j)
			{
				ID[i] ^= ID[j];
			}
		}
	}
	// Mask the data
	for (auto i = 0; i < 5; i++)
	{
		ID[i] ^= MaskData[i];
	}
}

void Autherization::UnMaskID(uint16_t* ID)
{
	// Un-Mask the data
	for (auto i = 0; i < 5; i++)
	{
		ID[i] ^= MaskData[i];
	}
	// Iterate and flip back the byte order
	for (auto i = 0; i < 5; i++)
	{
		for (auto j = 0; j < i; i++)
		{
			if (i != j)
			{
				ID[4 - i] ^= ID[4 - j];
			}
		}
	}
}

void Autherization::PrepareName(std::string& Name)
{
	// A buffer for the new one
	std::stringstream Buffer;
	// Grab the length
	auto Length = Name.length();
	// Prepare to iterate and XOR the data
	for (size_t i = 0; i < Length; i++)
	{
		// Prepare to append
		Buffer << std::hex << (Name[i] ^ MaskNameData[i % 10]);
	}
	// Set
	Name = Buffer.str();
}

const std::string Autherization::ComputeSystemID()
{
	// A buffer to hold our ID
	uint16_t ID[5];

	// Produce id slots
	ID[0] = GetCPUHash();
	ID[1] = GetVolumeHash();
	// Get mac
	GetMacHash(ID[2], ID[3]);

	// Verification
	ID[4] = 0;
	// Loop
	for (auto i = 0; i < 4; i++)
	{
		ID[4] += ID[i];
	}

	// Mask it
	MaskID(ID);

	// Prepare to build the unique ID
	std::stringstream Buffer;

	// Fetch the pc name
	auto Result = GetMachineName();

	// Prepare the name for storage
	PrepareName(Result);
	// Add the name
	Buffer << Result;

	// Loop and add other parts
	for (auto i = 0; i < 5; i++)
	{
		// Buffer
		char Num[16];
		// Format it
		sprintf_s(Num, "%x", ID[i]);
		// Separator
		Buffer << "-";
		// Check size
		switch (strlen(Num))
		{
		case 1: Buffer << "000"; break;
		case 2: Buffer << "00"; break;
		case 3: Buffer << "0"; break;
		}
		// Number
		Buffer << Num;
	}

	// Return it
	return Strings::ToUpper(Buffer.str());
}

void Autherization::EnableAntiDebug()
{
	// We can overwrite the data at the function... (Only release builds)
#if (!_DEBUG)
	// Get the address of the library call
	auto Callback = GetProcAddress(LoadLibraryA("ntdll.dll"), "DbgUiRemoteBreakin");

	// Exchange the new logic...
	const uint64_t InvalidSquence = 0xBADC0DED3DD;
	// Exchange it
	InterlockedExchange64((volatile LONG64*)Callback, InvalidSquence);
#endif
}