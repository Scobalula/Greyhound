#include "pch.h"
#include "Utils.h"

// Check whether the specified file path should be written
// Uses the OverwriteExistingFiles config value to determine whether existing files should be overwritten
bool Utils::ShouldWriteFile(string Path)
{
	if (IO::File::Exists(Path))
		return ExportManager::Config.Get<System::SettingType::Boolean>("OverwriteExistingFiles");

	return true;
}

string Utils::GetTimestamp() {
    time_t now;
    time(&now);
    char buf[32];
    tm t;
    localtime_s(&t, &now);
    strftime(buf, sizeof buf, "%H:%M:%S", &t);
    return string(buf);
}

string Utils::GetDate()
{
    time_t now;
    time(&now);
    char buf[32];
    tm t;
    localtime_s(&t, &now);
    strftime(buf, sizeof buf, "%Y-%m-%d", &t);
    return string(buf);
}


string Utils::Vector3ToHexColor(Math::Vector3 vec)
{
    std::stringstream stream;

    stream << std::hex << (uint16_t)vec.X << (uint16_t)vec.Y << (uint16_t)vec.Z;

    return stream.str().c_str();
}