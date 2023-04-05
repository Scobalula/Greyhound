#pragma once
#include <Vector3.h>
namespace Utils
{
	bool ShouldWriteFile(string Path);
	string GetTimestamp();
	string GetDate();
	string Vector3ToHexColor(Math::Vector3 vec);
	static std::string GetIndentation(int level)
	{
		std::string ret = "";

		for (int i = 0; i < level; ++i)
			ret += '\t';
		return ret;
	}
};


#define ASSERT_SIZE(Type, Size) static_assert(sizeof(Type) == Size, "Invalid type size for " #Type)