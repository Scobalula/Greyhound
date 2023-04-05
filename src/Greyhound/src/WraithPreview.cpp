#include "pch.h"
#include "WraithPreview.h"

WraithPreview::WraithPreview()
	: Forms::Form()
{
	this->InitializeComponent();
}

void WraithPreview::AssignPreviewModel(const Assets::Model& Model, const string& Name, uint64_t DebugVersion)
{
	this->ModelPreview->SetZUpAxis(true);
	this->ModelPreview->SetViewModel(Model);
	this->ModelPreview->SetAssetName(Name);
	this->ModelPreview->SetDebugVersion(DebugVersion);
}

void WraithPreview::AssignPreviewImage(const Assets::Texture& Texture, const string& Name, uint64_t DebugVersion)
{
	this->ModelPreview->SetViewTexture(Texture);
	this->ModelPreview->SetAssetName(Name);
	this->ModelPreview->SetDebugVersion(DebugVersion);
}

void WraithPreview::SetMaterialStreamer(Assets::AssetRenderer::MaterialStreamCallback Callback)
{
	this->ModelPreview->SetMaterialStreamer(Callback);
}

Assets::AssetRenderer* WraithPreview::GetRenderer()
{
	return this->ModelPreview;
}

void WraithPreview::InitializeComponent()
{
	this->SuspendLayout();
	this->SetAutoScaleDimensions({ 6, 13 });
	this->SetAutoScaleMode(Forms::AutoScaleMode::Font);
	this->SetText("Asset Preview");
	this->SetClientSize({ 781, 510 });
	this->SetSize({ 781, 510 });
	this->SetMinimumSize({ 781, 510 });
	this->SetStartPosition(Forms::FormStartPosition::CenterScreen);

	this->ModelPreview = new Assets::AssetRenderer();
	this->ModelPreview->SetSize({ 781, 481 });
	this->ModelPreview->SetBackColor({ 33, 33, 33 });
	this->ModelPreview->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left | Forms::AnchorStyles::Right | Forms::AnchorStyles::Bottom);
	this->AddControl(this->ModelPreview);

	this->ResumeLayout(false);
	this->PerformLayout();
}
