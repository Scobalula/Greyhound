#include "pch.h"
#include "WraithSettings.h"
#include "ExportManager.h"
#include "Process.h"
#include "WraithMain.h"
#include <version.h>
#include <CoDAssets.h>
#include <GreyhoundTheme.h>
#include <KoreTheme.h>
#include <UIXTheme.h>

WraithSettings::WraithSettings()
	: Forms::Form()
{
	this->InitializeComponent();
}

void WraithSettings::InitializeComponent()
{
	const INT WindowX = 710;
	const INT WindowY = 328;

	this->SuspendLayout();
	this->SetAutoScaleDimensions({ 6, 13 });
	this->SetAutoScaleMode(Forms::AutoScaleMode::Font);
	this->SetText("Greyhound | Application Settings");
	this->SetClientSize({ WindowX, WindowY });
	this->SetFormBorderStyle(Forms::FormBorderStyle::FixedSingle);
	this->SetStartPosition(Forms::FormStartPosition::CenterParent);
	this->SetMinimizeBox(false);
	this->SetMaximizeBox(false);

	//
	//	Custom Export Directory Box
	//
	this->groupBox1 = new UIX::UIXGroupBox();
	this->groupBox1->SetSize({ 458, 65 });
	this->groupBox1->SetLocation({ 12, 10 });
	this->groupBox1->SetTabIndex(3);
	this->groupBox1->SetText("General Directory Settings");
	this->groupBox1->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->AddControl(this->groupBox1);

	//
	//	Custom Export Directory
	//
	this->ExportBrowseFolder = new UIX::UIXTextBox();
	this->ExportBrowseFolder->SetSize({ 342, 25 });
	this->ExportBrowseFolder->SetLocation({ 15, 25 });
	this->ExportBrowseFolder->SetTabIndex(5);
	this->ExportBrowseFolder->SetReadOnly(true);
	this->ExportBrowseFolder->SetText("Click on \"Browse\" to set a custom export directory");
	this->ExportBrowseFolder->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Right);
	this->groupBox1->AddControl(this->ExportBrowseFolder);

	this->ExportBrowseButton = new UIX::UIXButton();
	this->ExportBrowseButton->SetSize({ 80, 25 });
	this->ExportBrowseButton->SetLocation({ 365, 25 });
	this->ExportBrowseButton->SetTabIndex(5);
	this->ExportBrowseButton->SetText("Browse");
	this->ExportBrowseButton->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Right);
	this->groupBox1->AddControl(this->ExportBrowseButton);

	// 
	//	Load Settings Box
	//
	this->groupBox2 = new UIX::UIXGroupBox();
	this->groupBox2->SetSize({ 458, 68 });
	this->groupBox2->SetLocation({ 12, 80 });
	this->groupBox2->SetTabIndex(3);
	this->groupBox2->SetText("Load Settings");
	this->groupBox2->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->AddControl(this->groupBox2);

	// 
	//	Load Settings
	//	Load Models
	this->LoadModels = new UIX::UIXCheckBox();
	this->LoadModels->SetSize({ 105, 18 });
	this->LoadModels->SetLocation({ 15, 20 });
	this->LoadModels->SetTabIndex(0);
	this->LoadModels->SetText("Load XModels");
	this->LoadModels->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox2->AddControl(this->LoadModels);

	//	Load Images
	this->LoadImages = new UIX::UIXCheckBox();
	this->LoadImages->SetSize({ 105, 18 });
	this->LoadImages->SetLocation({ 130, 20 });
	this->LoadImages->SetTabIndex(2);
	this->LoadImages->SetText("Load XImages");
	this->LoadImages->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox2->AddControl(this->LoadImages);

	//	Load Animations
	this->LoadAnimations = new UIX::UIXCheckBox();
	this->LoadAnimations->SetSize({ 105, 18 });
	this->LoadAnimations->SetLocation({ 245, 20 });
	this->LoadAnimations->SetTabIndex(1);
	this->LoadAnimations->SetText("Load XAnims");
	this->LoadAnimations->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox2->AddControl(this->LoadAnimations);

	//	Load RawFiles
	this->LoadRawFiles = new UIX::UIXCheckBox();
	this->LoadRawFiles->SetSize({ 105, 18 });
	this->LoadRawFiles->SetLocation({ 15, 43 });
	this->LoadRawFiles->SetTabIndex(2);
	this->LoadRawFiles->SetText("Load Raw Files");
	this->LoadRawFiles->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox2->AddControl(this->LoadRawFiles);

	//	Load Sounds
	this->LoadSounds = new UIX::UIXCheckBox();
	this->LoadSounds->SetSize({ 105, 18 });
	this->LoadSounds->SetLocation({ 130, 43 });
	this->LoadSounds->SetTabIndex(2);
	this->LoadSounds->SetText("Load Sounds");
	this->LoadSounds->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox2->AddControl(this->LoadSounds);

	//	Load Materials
	this->LoadMaterials = new UIX::UIXCheckBox();
	this->LoadMaterials->SetSize({ 105, 18 });
	this->LoadMaterials->SetLocation({ 245, 43 });
	this->LoadMaterials->SetTabIndex(3);
	this->LoadMaterials->SetText("Load Materials");
	this->LoadMaterials->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox2->AddControl(this->LoadMaterials);

	//
	//	About Box
	//
	this->groupBox3 = new UIX::UIXGroupBox();
	this->groupBox3->SetSize({ 218, 138 });
	this->groupBox3->SetLocation({ 480, 10 });
	this->groupBox3->SetTabIndex(3);
	this->groupBox3->SetText("About");
	this->groupBox3->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->AddControl(this->groupBox3);

	//
	//	About Text Label
	//
	this->labelVersion = new UIX::UIXLabel();
	this->labelVersion->SetSize({ 200, 20 });
	this->labelVersion->SetLocation({ 12, 20 });
	this->labelVersion->SetTabIndex(0);
#ifndef NIGHTLY
	this->labelVersion->SetText("Version " UI_VER_STR " (Stable)");
#else
	this->labelVersion->SetText("Version " UI_VER_STR "+" STRINGIZE(NIGHTLY) " (Nightly)");
#endif
	this->labelVersion->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->labelVersion->SetTextAlign(Drawing::ContentAlignment::TopLeft);
	this->groupBox3->AddControl(this->labelVersion);

	this->labelAbout = new UIX::UIXLabel();
	this->labelAbout->SetSize({ 200, 60 });
	this->labelAbout->SetLocation({ 12, 40 });
	this->labelAbout->SetTabIndex(0);
	this->labelAbout->SetText("Developed by DTZxPorter - Maintained by Scobalula - Some game research by id-daemon and help from Eric Maynard's ModelGetter.");
	this->labelAbout->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->labelAbout->SetTextAlign(Drawing::ContentAlignment::TopLeft);
	this->groupBox3->AddControl(this->labelAbout);

	//
	//	Github Link Button
	//
	this->GithubButton = new UIX::UIXButton();
	this->GithubButton->SetSize({ 190, 25 });
	this->GithubButton->SetLocation({ 13, 105 });
	this->GithubButton->SetTabIndex(1);
	this->GithubButton->SetText("Github");
	this->groupBox3->AddControl(this->GithubButton);

	// 
	//	Toggle Settings Box
	//
	this->groupBox4 = new UIX::UIXGroupBox();
	this->groupBox4->SetSize({ 340, 165 });
	this->groupBox4->SetLocation({ 12, 155 });
	this->groupBox4->SetTabIndex(3);
	this->groupBox4->SetText("Toggle Settings");
	this->groupBox4->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->AddControl(this->groupBox4);

	this->ToggleOverwriting = new UIX::UIXCheckBox();
	this->ToggleOverwriting->SetSize({ 108, 18 });
	this->ToggleOverwriting->SetLocation({ 15, 20 });
	this->ToggleOverwriting->SetTabIndex(2);
	this->ToggleOverwriting->SetText("Overwrite files");
	this->ToggleOverwriting->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox4->AddControl(this->ToggleOverwriting);

	this->PatchColor = new UIX::UIXCheckBox();
	this->PatchColor->SetSize({ 108, 18 });
	this->PatchColor->SetLocation({ 15, 43 });
	this->PatchColor->SetTabIndex(2);
	this->PatchColor->SetText("Patch color");
	this->PatchColor->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox4->AddControl(this->PatchColor);

	this->ToggleAudioLanguageFolders = new UIX::UIXCheckBox();
	this->ToggleAudioLanguageFolders->SetSize({ 155, 18 });
	this->ToggleAudioLanguageFolders->SetLocation({ 130, 20 });
	this->ToggleAudioLanguageFolders->SetTabIndex(2);
	this->ToggleAudioLanguageFolders->SetText("Preserve sound file paths");
	this->ToggleAudioLanguageFolders->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox4->AddControl(this->ToggleAudioLanguageFolders);

	this->ToggleGlobalImages = new UIX::UIXCheckBox();
	this->ToggleGlobalImages->SetSize({ 150, 18 });
	this->ToggleGlobalImages->SetLocation({ 130, 66 });
	this->ToggleGlobalImages->SetTabIndex(2);
	this->ToggleGlobalImages->SetText("Global images folder");
	this->ToggleGlobalImages->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox4->AddControl(this->ToggleGlobalImages);

	this->ToggleExportModelImages = new UIX::UIXCheckBox();
	this->ToggleExportModelImages->SetSize({ 150, 18 });
	this->ToggleExportModelImages->SetLocation({ 130, 43 });
	this->ToggleExportModelImages->SetTabIndex(2);
	this->ToggleExportModelImages->SetText("Export Model Images");
	this->ToggleExportModelImages->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox4->AddControl(this->ToggleExportModelImages);

	//	Patch Normals
	this->PatchNormals = new UIX::UIXCheckBox();
	this->PatchNormals->SetSize({ 108, 18 });
	this->PatchNormals->SetLocation({ 15, 66 });
	this->PatchNormals->SetTabIndex(2);
	this->PatchNormals->SetText("Patch normals");
	this->PatchNormals->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox4->AddControl(this->PatchNormals);

	//	Load UI Images
	this->ExportLods = new UIX::UIXCheckBox();
	this->ExportLods->SetSize({ 108, 18 });
	this->ExportLods->SetLocation({ 15, 89 });
	this->ExportLods->SetTabIndex(2);
	this->ExportLods->SetText("Export all LODs");
	this->ExportLods->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox4->AddControl(this->ExportLods);

	//	Load SettingsSets
	this->LoadSettingsSets = new UIX::UIXCheckBox();
	this->LoadSettingsSets->SetSize({ 150, 18 });
	this->LoadSettingsSets->SetLocation({ 130, 89 });
	this->LoadSettingsSets->SetTabIndex(2);
	this->LoadSettingsSets->SetText("Export Model Hitbox");
	this->LoadSettingsSets->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->groupBox4->AddControl(this->LoadSettingsSets);

	//
	//	Assets Export Settings Box
	//
	this->groupBox5 = new UIX::UIXGroupBox();
	this->groupBox5->SetSize({ 340, 165 });
	this->groupBox5->SetLocation({ 359, 155 });
	this->groupBox5->SetTabIndex(0);
	this->groupBox5->SetText("Assets Export Settings");
	this->groupBox5->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Left | Forms::AnchorStyles::Right);
	this->AddControl(this->groupBox5);

	//
	//	Model Export Format
	//
	this->label1 = new UIX::UIXLabel();
	this->label1->SetSize({ 90, 15 });
	this->label1->SetLocation({ 20, 20 });
	this->label1->SetTabIndex(8);
	this->label1->SetText("Model Format");
	this->label1->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->label1->SetTextAlign(Drawing::ContentAlignment::TopLeft);
	this->groupBox5->AddControl(this->label1);

	this->ModelExportFormat = new UIX::UIXComboBox();
	this->ModelExportFormat->SetSize({ 90, 21 });
	this->ModelExportFormat->SetLocation({ 20, 35 });
	this->ModelExportFormat->SetTabIndex(9);
	this->ModelExportFormat->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->ModelExportFormat->SetDropDownStyle(Forms::ComboBoxStyle::DropDownList);
	this->ModelExportFormat->Items.Add("Cast");
	this->ModelExportFormat->Items.Add("GLTF");
	this->ModelExportFormat->Items.Add("Maya");
	this->ModelExportFormat->Items.Add("OBJ");
	this->ModelExportFormat->Items.Add("SEModel");
	this->ModelExportFormat->Items.Add("SMD");
	this->ModelExportFormat->Items.Add("XModel");
	this->ModelExportFormat->Items.Add("XNALara ASCII");
	this->groupBox5->AddControl(this->ModelExportFormat);

	//
	//	Animation Export Format
	//
	this->label2 = new UIX::UIXLabel();
	this->label2->SetSize({ 90, 15 });
	this->label2->SetLocation({ 20, 65 });
	this->label2->SetTabIndex(11);
	this->label2->SetText("Anim Format");
	this->label2->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->label2->SetTextAlign(Drawing::ContentAlignment::TopLeft);
	this->groupBox5->AddControl(this->label2);

	this->AnimExportFormat = new UIX::UIXComboBox();
	this->AnimExportFormat->SetSize({ 90, 21 });
	this->AnimExportFormat->SetLocation({ 20, 80 });
	this->AnimExportFormat->SetTabIndex(0);
	this->AnimExportFormat->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->AnimExportFormat->SetDropDownStyle(Forms::ComboBoxStyle::DropDownList);
	this->AnimExportFormat->Items.Add("Cast");
	this->AnimExportFormat->Items.Add("SEAnim");
	this->AnimExportFormat->Items.Add("XAnim");
	this->groupBox5->AddControl(this->AnimExportFormat);

	//
	//  MaterialCPUExportFormat
	////
	//this->label7 = new UIX::UIXLabel();
	//this->label7->SetSize({ 90, 15 });
	//this->label7->SetLocation({ 230, 20 });
	//this->label7->SetTabIndex(9);
	//this->label7->SetText("Material CPU Format");
	//this->label7->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	//this->label7->SetTextAlign(Drawing::ContentAlignment::TopLeft);
	//this->groupBox5->AddControl(this->label7);

	//this->MatCPUExportFormat = new UIX::UIXComboBox();
	//this->MatCPUExportFormat->SetSize({ 90, 21 });
	//this->MatCPUExportFormat->SetLocation({ 230, 35 });
	//this->MatCPUExportFormat->SetTabIndex(0);
	//this->MatCPUExportFormat->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	//this->MatCPUExportFormat->SetDropDownStyle(Forms::ComboBoxStyle::DropDownList);
	//this->groupBox5->AddControl(this->MatCPUExportFormat);

	//
	//  AudioExportFormat
	//
	this->label8 = new UIX::UIXLabel();
	this->label8->SetSize({ 90, 15 });
	this->label8->SetLocation({ 125, 20 });
	this->label8->SetTabIndex(9);
	this->label8->SetText("Audio Format");
	this->label8->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->label8->SetTextAlign(Drawing::ContentAlignment::TopLeft);
	this->groupBox5->AddControl(this->label8);

	this->AudioExportFormat = new UIX::UIXComboBox();
	this->AudioExportFormat->SetSize({ 90, 21 });
	this->AudioExportFormat->SetLocation({ 125, 35 });
	this->AudioExportFormat->SetTabIndex(0);
	this->AudioExportFormat->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->AudioExportFormat->SetDropDownStyle(Forms::ComboBoxStyle::DropDownList);
	this->AudioExportFormat->Items.Add("WAV");
	this->AudioExportFormat->Items.Add("FLAC");
	this->groupBox5->AddControl(this->AudioExportFormat);

	//
	//	Image Export Format
	//
	this->label3 = new UIX::UIXLabel();
	this->label3->SetSize({ 90, 15 });
	this->label3->SetLocation({ 125, 65 });
	this->label3->SetTabIndex(9);
	this->label3->SetText("Image Format");
	this->label3->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->label3->SetTextAlign(Drawing::ContentAlignment::TopLeft);
	this->groupBox5->AddControl(this->label3);

	this->ImageExportFormat = new UIX::UIXComboBox();
	this->ImageExportFormat->SetSize({ 90, 21 });
	this->ImageExportFormat->SetLocation({ 125, 80 });
	this->ImageExportFormat->SetTabIndex(0);
	this->ImageExportFormat->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->ImageExportFormat->SetDropDownStyle(Forms::ComboBoxStyle::DropDownList);
	this->ImageExportFormat->Items.Add("DDS");
	this->ImageExportFormat->Items.Add("PNG");
	this->ImageExportFormat->Items.Add("TIFF");
	this->ImageExportFormat->Items.Add("TGA");
	this->groupBox5->AddControl(this->ImageExportFormat);

	//
	//	AssetSortType
	//
	this->label6 = new UIX::UIXLabel();
	this->label6->SetSize({ 120, 15 });
	this->label6->SetLocation({ 20, 110 });
	//this->label6->SetLocation({ 125, 110 });
	this->label6->SetTabIndex(9);
	this->label6->SetText("Sort assets by");
	this->label6->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->label6->SetTextAlign(Drawing::ContentAlignment::TopLeft);
	this->groupBox5->AddControl(this->label6);

	this->AssetSortMethod = new UIX::UIXComboBox();
	this->AssetSortMethod->SetSize({ 90, 20 });
	this->AssetSortMethod->SetLocation({ 20, 125 });
	this->AssetSortMethod->SetTabIndex(0);
	this->AssetSortMethod->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->AssetSortMethod->SetDropDownStyle(Forms::ComboBoxStyle::DropDownList);
	this->AssetSortMethod->Items.Add("None");
	this->AssetSortMethod->Items.Add("Name");
	this->AssetSortMethod->Items.Add("Details");
	this->groupBox5->AddControl(this->AssetSortMethod);

	this->ResumeLayout(false);
	this->PerformLayout();
	// END DESIGNER CODE

	//this->SetBackColor({ 30, 32, 55 });
	this->SetBackColor({ 33, 33, 33 });

	this->Load += &OnLoad;
	this->FormClosing += &OnClose;
	this->GithubButton->Click += &OnGreyhoundClick;
	this->ExportBrowseButton->Click += &OnBrowseClick;
}

void WraithSettings::LoadSettings()
{
	ModelExportFormat_t ModelFormat = (ModelExportFormat_t)ExportManager::Config.Get<System::SettingType::Integer>("ModelFormat");
	AnimExportFormat_t AnimFormat = (AnimExportFormat_t)ExportManager::Config.Get<System::SettingType::Integer>("AnimFormat");
	ImageExportFormat_t ImageFormat = (ImageExportFormat_t)ExportManager::Config.Get<System::SettingType::Integer>("ImageFormat");
	AssetSortMethod_t AssetSortType = (AssetSortMethod_t)ExportManager::Config.Get<System::SettingType::Integer>("AssetSortMethod");
	AudioExportFormat_t AudioFormat = (AudioExportFormat_t)ExportManager::Config.Get<System::SettingType::Integer>("AudioFormat");

	if (!ExportManager::Config.Has("AssetSortMethod"))
		AssetSortType = AssetSortMethod_t::Name;

	switch (ModelFormat)
	{
	case ModelExportFormat_t::Cast:
		this->ModelExportFormat->SetSelectedIndex(0);
		break;
	case ModelExportFormat_t::GLTF:
		this->ModelExportFormat->SetSelectedIndex(1);
		break;
	case ModelExportFormat_t::Maya:
		this->ModelExportFormat->SetSelectedIndex(2);
		break;
	case ModelExportFormat_t::OBJ:
		this->ModelExportFormat->SetSelectedIndex(3);
		break;
	case ModelExportFormat_t::SEModel:
		this->ModelExportFormat->SetSelectedIndex(4);
		break;
	case ModelExportFormat_t::SMD:
		this->ModelExportFormat->SetSelectedIndex(5);
		break;
	case ModelExportFormat_t::XModel:
		this->ModelExportFormat->SetSelectedIndex(6);
		break;
	case ModelExportFormat_t::XNALaraText:
		this->ModelExportFormat->SetSelectedIndex(7);
		break;
	}

	switch (AnimFormat)
	{
	case AnimExportFormat_t::Cast:
		this->AnimExportFormat->SetSelectedIndex(0);
		break;
	case AnimExportFormat_t::SEAnim:
		this->AnimExportFormat->SetSelectedIndex(1);
		break;
	case AnimExportFormat_t::XAnim:
		this->AnimExportFormat->SetSelectedIndex(2);
		break;
	}

	switch (ImageFormat)
	{
	case ImageExportFormat_t::Dds:
		this->ImageExportFormat->SetSelectedIndex(0);
		break;
	case ImageExportFormat_t::Png:
		this->ImageExportFormat->SetSelectedIndex(1);
		break;
	case ImageExportFormat_t::Tiff:
		this->ImageExportFormat->SetSelectedIndex(2);
		break;
	case ImageExportFormat_t::Tga:
		this->ImageExportFormat->SetSelectedIndex(3);
		break;
	}

	//switch (CurrentTheme)
	//{
	//case Theme_t::Wraith:
	//	this->CurrentTheme->SetSelectedIndex(0);
	//	break;
	//case Theme_t::Legion:
	//	this->CurrentTheme->SetSelectedIndex(1);
	//	break;
	//}

	switch (AssetSortType)
	{
	case AssetSortMethod_t::None:
		this->AssetSortMethod->SetSelectedIndex(0);
		break;
	case AssetSortMethod_t::Name:
		this->AssetSortMethod->SetSelectedIndex(1);
		break;
	case AssetSortMethod_t::Details:
		this->AssetSortMethod->SetSelectedIndex(2);
		break;
	}

	//switch (MatCPUFormat)
	//{
	//case MatCPUExportFormat_t::None:
	//	this->MatCPUExportFormat->SetSelectedIndex(0);
	//	break;
	//case MatCPUExportFormat_t::Struct:
	//	this->MatCPUExportFormat->SetSelectedIndex(1);
	//	break;
	//case MatCPUExportFormat_t::CPU:
	//	this->MatCPUExportFormat->SetSelectedIndex(2);
	//	break;
	//}

	switch (AudioFormat)
	{
	case AudioExportFormat_t::WAV:
		this->AudioExportFormat->SetSelectedIndex(0);
		break;
	case AudioExportFormat_t::Flak:
		this->AudioExportFormat->SetSelectedIndex(1);
		break;
	}

	this->LoadModels->SetChecked(ExportManager::Config.GetBool("LoadModels"));
	//this->CreateLog->SetChecked(ExportManager::Config.GetBool("CreateXassetLog"));
	this->LoadAnimations->SetChecked(ExportManager::Config.GetBool("LoadAnimations"));
	this->LoadSounds->SetChecked(ExportManager::Config.GetBool("LoadSounds"));
	this->LoadImages->SetChecked(ExportManager::Config.GetBool("LoadImages"));
	this->LoadMaterials->SetChecked(ExportManager::Config.GetBool("LoadMaterials"));
	this->ExportLods->SetChecked(ExportManager::Config.GetBool("ExportLods"));
	this->LoadRawFiles->SetChecked(ExportManager::Config.GetBool("LoadRawFiles"));
	this->PatchNormals->SetChecked(ExportManager::Config.GetBool("PatchNormals"));
	this->LoadSettingsSets->SetChecked(ExportManager::Config.GetBool("ExportHitbox"));
	this->ToggleOverwriting->SetChecked(ExportManager::Config.GetBool("OverwriteExistingFiles"));
	this->ToggleAudioLanguageFolders->SetChecked(ExportManager::Config.GetBool("AudioLanguageFolders"));
	this->PatchColor->SetChecked(ExportManager::Config.GetBool("PatchColor"));
	this->ToggleGlobalImages->SetChecked(ExportManager::Config.GetBool("GlobalImages"));
	this->ToggleExportModelImages->SetChecked(ExportManager::Config.GetBool("ExportModelImages"));

	if (ExportManager::Config.Has<System::SettingType::String>("ExportDirectory"))
	{
		this->ExportBrowseFolder->SetText(ExportManager::Config.Get<System::SettingType::String>("ExportDirectory"));
	}
}

void WraithSettings::OnLoad(Forms::Control* Sender)
{
	((WraithSettings*)Sender->FindForm())->LoadSettings();
}

void WraithSettings::OnClose(const std::unique_ptr<FormClosingEventArgs>& EventArgs, Forms::Control* Sender)
{
	auto ThisPtr = (WraithSettings*)Sender->FindForm();

	// Fetch settings from controls
	auto ModelExportFormat = ModelExportFormat_t::SEModel;
	auto AnimExportFormat = AnimExportFormat_t::SEAnim;
	auto ImageExportFormat = ImageExportFormat_t::Tiff;
	auto AssetSortMethod = AssetSortMethod_t::Name;
	auto AudioFormat = AudioExportFormat_t::WAV;

	if (ThisPtr->ModelExportFormat->SelectedIndex() > -1)
	{
		switch (ThisPtr->ModelExportFormat->SelectedIndex())
		{
		case 1:
			ModelExportFormat = ModelExportFormat_t::GLTF;
			break;
		case 2:
			ModelExportFormat = ModelExportFormat_t::Maya;
			break;
		case 3:
			ModelExportFormat = ModelExportFormat_t::OBJ;
			break;
		case 4:
			ModelExportFormat = ModelExportFormat_t::SEModel;
			break;
		case 5:
			ModelExportFormat = ModelExportFormat_t::SMD;
			break;
		case 6:
			ModelExportFormat = ModelExportFormat_t::XModel;
			break;
		case 7:
			ModelExportFormat = ModelExportFormat_t::XNALaraText;
			break;
		}
	}

	if (ThisPtr->AnimExportFormat->SelectedIndex() > -1)
	{
		switch (ThisPtr->AnimExportFormat->SelectedIndex())
		{
		case 1:
			AnimExportFormat = AnimExportFormat_t::SEAnim;
			break;
		case 2:
			AnimExportFormat = AnimExportFormat_t::XAnim;
			break;
		}
	}

	if (ThisPtr->ImageExportFormat->SelectedIndex() > -1)
	{
		switch (ThisPtr->ImageExportFormat->SelectedIndex())
		{
		case 1:
			ImageExportFormat = ImageExportFormat_t::Png;
			break;
		case 2:
			ImageExportFormat = ImageExportFormat_t::Tiff;
			break;
		case 3:
			ImageExportFormat = ImageExportFormat_t::Tga;
			break;
		}
	}

	//if (ThisPtr->CurrentTheme->SelectedIndex() > -1)
	//{
	//	switch (ThisPtr->CurrentTheme->SelectedIndex())
	//	{
	//	case 0:
	//		Theme = Theme_t::Wraith;
	//		break;
	//	case 1:
	//		Theme = Theme_t::Legion;
	//		break;
	//	}
	//}

	if (ThisPtr->AssetSortMethod->SelectedIndex() > -1)
	{
		switch (ThisPtr->AssetSortMethod->SelectedIndex())
		{
		case 0:
			AssetSortMethod = AssetSortMethod_t::None;
			break;
		case 1:
			AssetSortMethod = AssetSortMethod_t::Name;
			break;
		case 2:
			AssetSortMethod = AssetSortMethod_t::Details;
			break;
		}
	}

	//if (ThisPtr->MatCPUExportFormat->SelectedIndex() > -1)
	//{
	//	switch (ThisPtr->MatCPUExportFormat->SelectedIndex())
	//	{
	//	case 0:
	//		MatCPUExportFormat = MatCPUExportFormat_t::None;
	//		break;
	//	case 1:
	//		MatCPUExportFormat = MatCPUExportFormat_t::Struct;
	//		break;
	//	case 2:
	//		MatCPUExportFormat = MatCPUExportFormat_t::CPU;
	//		break;
	//	}
	//}

	if (ThisPtr->AudioExportFormat->SelectedIndex() > -1)
	{
		switch (ThisPtr->AudioExportFormat->SelectedIndex())
		{
		case 0:
			AudioFormat = AudioExportFormat_t::WAV;
			break;
		case 1:
			AudioFormat = AudioExportFormat_t::Flak;
			break;
		}
	}

	// have the settings actually changed?
	bool bRefreshView = false;

	if (ThisPtr->LoadModels->Checked() != ExportManager::Config.GetBool("LoadModels"))
		bRefreshView = true;
	if (ThisPtr->LoadAnimations->Checked() != ExportManager::Config.GetBool("LoadAnimations"))
		bRefreshView = true;
	if (ThisPtr->LoadSounds->Checked() != ExportManager::Config.GetBool("LoadSounds"))
		bRefreshView = true;
	if (ThisPtr->LoadImages->Checked() != ExportManager::Config.GetBool("LoadImages"))
		bRefreshView = true;
	if (ThisPtr->LoadMaterials->Checked() != ExportManager::Config.GetBool("LoadMaterials"))
		bRefreshView = true;
	if (ThisPtr->LoadRawFiles->Checked() != ExportManager::Config.GetBool("LoadRawFiles"))
		bRefreshView = true;

	ExportManager::Config.SetBool("LoadModels", ThisPtr->LoadModels->Checked());
	ExportManager::Config.SetBool("LoadAnimations", ThisPtr->LoadAnimations->Checked());
	ExportManager::Config.SetBool("LoadSounds", ThisPtr->LoadSounds->Checked());
	ExportManager::Config.SetBool("LoadImages", ThisPtr->LoadImages->Checked());
	ExportManager::Config.SetBool("LoadMaterials", ThisPtr->LoadMaterials->Checked());
	ExportManager::Config.SetBool("LoadRawFiles", ThisPtr->LoadRawFiles->Checked());

	ExportManager::Config.SetBool("GlobalImages", ThisPtr->ToggleGlobalImages->Checked());
	ExportManager::Config.SetBool("ExportModelImages", ThisPtr->ToggleExportModelImages->Checked());
	ExportManager::Config.SetBool("ExportLods", ThisPtr->ExportLods->Checked());
	ExportManager::Config.SetBool("ExportHitbox", ThisPtr->LoadSettingsSets->Checked());

	ExportManager::Config.SetBool("PatchNormals", ThisPtr->PatchNormals->Checked());
	ExportManager::Config.SetBool("PatchColor", ThisPtr->PatchColor->Checked());
	
	//ExportManager::Config.SetBool("CreateXassetLog", ThisPtr->CreateLog->Checked());
	ExportManager::Config.SetBool("OverwriteExistingFiles", ThisPtr->ToggleOverwriting->Checked());
	ExportManager::Config.SetBool("AudioLanguageFolders", ThisPtr->ToggleAudioLanguageFolders->Checked());

	ExportManager::Config.SetInt("ModelFormat", (uint32_t)ModelExportFormat);
	ExportManager::Config.SetInt("AnimFormat", (uint32_t)AnimExportFormat);
	ExportManager::Config.SetInt("ImageFormat", (uint32_t)ImageExportFormat);
	ExportManager::Config.SetInt("AssetSortMethod", (uint32_t)AssetSortMethod);
	ExportManager::Config.SetInt("AudioFormat", (uint32_t)AudioFormat);

	auto ExportDirectory = ThisPtr->ExportBrowseFolder->Text();

	if (ExportDirectory == "Click on \"Browse\" to set a custom export directory")
	{
		ExportManager::Config.Remove<System::SettingType::String>("ExportDirectory");
	}
	else if (IO::Directory::Exists(ExportDirectory))
	{
		ExportManager::Config.Set<System::SettingType::String>("ExportDirectory", ExportDirectory);
	}

	ExportManager::SaveConfigToDisk();

	if(bRefreshView)
		g_pWraithMain->RefreshView();
}

void WraithSettings::OnGreyhoundClick(Forms::Control* Sender)
{
	Diagnostics::Process::Start("https://github.com/Scobalula/Greyhound");
}

void WraithSettings::OnBrowseClick(Forms::Control* Sender)
{
	auto ThisPtr = (WraithSettings*)Sender->FindForm();
	auto ExportDirectory = ThisPtr->ExportBrowseFolder->Text();

	auto Result = OpenFileDialog::ShowFolderDialog("Select a folder to export assets to or press \"Cancel\" to reset back to default.", ExportDirectory != "Click on \"Browse\" to set a custom export directory" ? ExportDirectory : string(""), ThisPtr);

	if (Result == "" || Result.Length() == 0)
	{
		ThisPtr->ExportBrowseFolder->SetText("Click on \"Browse\" to set a custom export directory");
	}
	else if (IO::Directory::Exists(Result))
	{
		ThisPtr->ExportBrowseFolder->SetText(Result);
	}
}

