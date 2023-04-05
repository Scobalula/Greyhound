#include "pch.h"
#include "WraithProgress.h"

WraithProgress::WraithProgress()
	: Forms::Form(), CanClose(false), Canceled(false), AutoClose(false)
{
	this->InitializeComponent();
}

void WraithProgress::UpdateProgress(uint32_t Progress, bool Finished)
{
	this->ExportProgressBar->SetValue(Progress);

	if (Finished)
	{
		this->FinishButton->SetEnabled(true);
		this->CancelButton->SetEnabled(false);
		this->CanClose = true;
		this->SetText("Greyhound | Export Complete");

		if (this->Canceled || this->AutoClose)
			this->Close();
	}
}

bool WraithProgress::IsCanceled()
{
	return this->Canceled;
}

void WraithProgress::SetAutoClose(bool Value)
{
	this->AutoClose = Value;
}

void WraithProgress::InitializeComponent()
{
	this->SuspendLayout();
	this->SetAutoScaleDimensions({ 6, 13 });
	this->SetAutoScaleMode(Forms::AutoScaleMode::Font);
	this->SetText("Greyhound | Exporting Assets...");
	this->SetClientSize({ 409, 119 });
	this->SetFormBorderStyle(Forms::FormBorderStyle::FixedSingle);
	this->SetStartPosition(Forms::FormStartPosition::CenterParent);
	this->SetMinimizeBox(false);
	this->SetMaximizeBox(false);
	this->SetShowInTaskbar(false);

	this->ExportLabel = new UIX::UIXLabel();
	this->ExportLabel->SetSize({ 385, 17 });
	this->ExportLabel->SetLocation({ 12, 18 });
	this->ExportLabel->SetTabIndex(3);
	this->ExportLabel->SetText("Progress:");
	this->ExportLabel->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left | Forms::AnchorStyles::Right);
	this->ExportLabel->SetTextAlign(Drawing::ContentAlignment::TopLeft);
	this->AddControl(this->ExportLabel);

	this->CancelButton = new UIX::UIXButton();
	this->CancelButton->SetSize({ 87, 31 });
	this->CancelButton->SetLocation({ 310, 76 });
	this->CancelButton->SetTabIndex(2);
	this->CancelButton->SetText("Cancel");
	this->CancelButton->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Right);
	this->CancelButton->Click += &OnCancelClick;
	this->AddControl(this->CancelButton);

	this->FinishButton = new UIX::UIXButton();
	this->FinishButton->SetSize({ 87, 31 });
	this->FinishButton->SetLocation({ 217, 76 });
	this->FinishButton->SetTabIndex(1);
	this->FinishButton->SetText("Ok");
	this->FinishButton->SetEnabled(false);
	this->FinishButton->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Right);
	this->FinishButton->Click += &OnFinishClick;
	this->AddControl(this->FinishButton);

	this->ExportProgressBar = new UIX::UIXProgressBar();
	this->ExportProgressBar->SetSize({ 385, 29 });
	this->ExportProgressBar->SetLocation({ 12, 38 });
	this->ExportProgressBar->SetTabIndex(0);
	this->ExportProgressBar->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left | Forms::AnchorStyles::Right);
	this->AddControl(this->ExportProgressBar);

	this->ResumeLayout(false);
	this->PerformLayout();
	// END DESIGNER CODE

	//this->SetBackColor({ 30, 32, 55 });
	this->SetBackColor({ 33, 33, 33 });
}

void WraithProgress::OnFinishClick(Forms::Control* Sender)
{
	((Forms::Form*)Sender->FindForm())->Close();
}

void WraithProgress::OnCancelClick(Forms::Control* Sender)
{
	((WraithProgress*)Sender->FindForm())->CancelProgress();
}

void WraithProgress::CancelProgress()
{
	this->CancelButton->SetEnabled(false);
	this->CancelButton->SetText("Canceling...");
	this->Canceled = true;
}
