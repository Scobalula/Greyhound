#include "Parasyte.h"

// Current Context Information
std::unique_ptr<ps::State> ps::state = nullptr;

ps::State::State() : 
	GameID(0),
	PoolsAddress(0),
	StringsAddress(0) {}

bool ps::State::Load(const std::string& path)
{
	std::ifstream s(path, std::ios::binary);
	uint32_t stringSize = 0;

	if (!s)
		return false;

	if (!s.read((char*)&GameID, sizeof(GameID)))
		return false;
	if (!s.read((char*)&PoolsAddress, sizeof(PoolsAddress)))
		return false;
	if (!s.read((char*)&StringsAddress, sizeof(StringsAddress)))
		return false;
	if (!s.read((char*)&stringSize, sizeof(stringSize)))
		return false;
	if (stringSize == 0)
		return false;

	GameDirectory.resize(stringSize);

	if (!s.read((char*)GameDirectory.data(), stringSize))
		return false;
	
	return true;
}

void ps::PoolParser64(uint64_t offset, std::function<XAsset64(const uint64_t&)> request, std::function<void(XAsset64&)> callback)
{
	for (auto p = request(offset); ; p = request(p.Next))
	{
		if (p.Header != 0)
			callback(p);

		if (p.Next == 0)
		{
			break;
		}
	}
}

