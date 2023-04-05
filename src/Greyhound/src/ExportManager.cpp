#include "pch.h"
#include "ExportManager.h"
#include "ParallelTask.h"
#include "Path.h"
#include "Directory.h"
#include "File.h"
#include "Environment.h"
#include "WraithMain.h"
#include "CoDAssets.h"

#define CONFIG_PATH "Greyhound.cfg"

#define INIT_SETTING(SType, Name, Val) \
		if (!Config.Has(Name)) \
			Config.Set<System::SettingType::SType>(Name, Val);

System::Settings ExportManager::Config = System::Settings();
string ExportManager::ApplicationPath = "";
string ExportManager::ExportPath = "";

void ExportManager::InitializeExporter()
{
	ApplicationPath = System::Environment::GetApplicationPath();

	string ConfigPath = IO::Path::Combine(ApplicationPath, CONFIG_PATH);

	if (IO::File::Exists(ConfigPath))
	{
		Config.Load(ConfigPath);
	}

	if (!Config.Has("ExportDirectory"))
	{
		ExportPath = IO::Path::Combine(ApplicationPath, "exported_files");
	}
	else
	{
		string ExportDirectory = Config.Get<System::SettingType::String>("ExportDirectory");

		if (IO::Directory::Exists(ExportDirectory))
		{
			ExportPath = ExportDirectory;
		}
		else
		{
			ExportPath = IO::Path::Combine(ApplicationPath, "exported_files");
			Config.Remove<System::SettingType::String>("ExportDirectory");
		}
	}

	// init settings if they don't already exist
	INIT_SETTING(Integer, "ModelFormat", (uint32_t)ModelExportFormat_t::SEModel);
	INIT_SETTING(Integer, "AnimFormat", (uint32_t)AnimExportFormat_t::SEAnim);
	INIT_SETTING(Integer, "ImageFormat", (uint32_t)ImageExportFormat_t::Tiff);
	INIT_SETTING(Integer, "AssetSortMethod", (uint32_t)AssetSortMethod_t::Name);
	INIT_SETTING(Boolean, "CreateXassetLog", false);

	INIT_SETTING(Boolean, "LoadModels", true);
	INIT_SETTING(Boolean, "LoadAnimations", true);
	INIT_SETTING(Boolean, "LoadSounds", false);
	INIT_SETTING(Boolean, "LoadImages", false);
	INIT_SETTING(Boolean, "LoadMaterials", false);
	INIT_SETTING(Boolean, "LoadRawFiles", false);

	INIT_SETTING(Boolean, "GlobalImages", false);
	INIT_SETTING(Boolean, "ExportLods", false); 
	INIT_SETTING(Boolean, "ExportModelImages", true);
	INIT_SETTING(Boolean, "ExportHitbox", false);

	INIT_SETTING(Boolean, "PatchNormals", true);
	INIT_SETTING(Boolean, "PatchColor", true);

	INIT_SETTING(Boolean, "LoadUIImages", false);
	INIT_SETTING(Boolean, "LoadShaderSets", false);

	Config.Save(ConfigPath);
}

void ExportManager::SaveConfigToDisk()
{
	string ExportDirectory = Config.Get<System::SettingType::String>("ExportDirectory");

	if (IO::Directory::Exists(ExportDirectory))
	{
		ExportPath = ExportDirectory;
	}
	else
	{
		ExportPath = IO::Path::Combine(ApplicationPath, "exported_files");
		Config.Remove<System::SettingType::String>("ExportDirectory");
	}

	Config.Save(IO::Path::Combine(ApplicationPath, CONFIG_PATH));
}

string ExportManager::GetMapExportPath()
{
	string Result = IO::Path::Combine(ExportPath, "maps");
	IO::Directory::CreateDirectory(Result);
	return Result;
}
