#include "stdafx.h"
#include "GreyhoundTheme.h"
#include "MessageBox.h"
#include "CheckBoxImage.h"

namespace Themes
{
	// Constants for brushes
	const static auto DefaultForeground = Drawing::Color(255, 255, 255); // WHITE
	const static auto BorderBrush = Drawing::Color(30, 144, 255); // DODGERBLUE
	const static auto DisabledBorderBrush = Drawing::Color(169, 169, 169); // DARKGRAY

	const static auto ListItemColorBase = Drawing::Color(54, 54, 54);        // A DARK BLACK-GRAY COLOR
	const static auto ListItemColorAlt = Drawing::Color(36, 36, 36);        // A DARK BLACK-GRAY COLOR

	const static auto BackgroundBrush = Drawing::Color(37, 37, 37);
	const static auto BackgroundLightBrush = Drawing::Color(39, 39, 39);

	const static auto BackgroundGrad1 = Drawing::Color(50, 50, 50);
	const static auto BackgroundGrad2 = Drawing::Color(42, 42, 42);

	const static auto BackgroundOverGrad1 = Drawing::Color(50, 50, 50);
	const static auto BackgroundOverGrad2 = Drawing::Color(42, 42, 42);

	const static auto TextEnabledBrush = Drawing::Color(Drawing::Color::White);
	const static auto TextDisabledBrush = Drawing::Color(Drawing::Color::White);

	const static auto ProgressGrad1 = Drawing::Color(3, 169, 244);
	const static auto ProgressGrad2 = Drawing::Color(0, 130, 220);

	const static auto HeaderGrad1 = Drawing::Color(50, 50, 50);
	const static auto HeaderGrad2 = Drawing::Color(42, 42, 42);
	const static auto HeaderSeparatorColor = Drawing::Color(64, 64, 64);
	const static auto HeaderBorderColour = Drawing::Color(32, 32, 32);

	const static auto HeaderBrush = Drawing::Color(54, 54, 54);

	// Constants for images
	static Drawing::Image* CheckBoxImage = nullptr;

	GreyhoundTheme::GreyhoundTheme()
		: UIX::UIXRenderer()
	{
		CheckBoxImage = Drawing::ImageFromTgaData(CheckBoxImage_Src, sizeof(CheckBoxImage_Src));

		// Change the message box colors to represent our theme
		Forms::MessageBox::SetMessageBoxColors(Drawing::Color::White, Drawing::Color(30, 30, 30), Drawing::Color(50, 50, 50));
	}

	GreyhoundTheme::~GreyhoundTheme()
	{
		delete CheckBoxImage;
	}

	void GreyhoundTheme::RenderControlBorder(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State) const
	{
		Drawing::Rectangle Rect(Ctrl->ClientRectangle());

		//
		// Override for textbox rendering due to a bug in the layout rect
		//

		if (Ctrl->GetType() == Forms::ControlTypes::TextBox)
		{
			Rect.Width = Ctrl->Size().Width;
			Rect.Height = Ctrl->Size().Height;
		}

		Rect.Width--;
		Rect.Height--;

		auto Brush = std::make_unique<Drawing::SolidBrush>((State == UIX::UIXRenderState::Disabled) ? DisabledBorderBrush : BorderBrush);

		//if (Ctrl->GetType() == Forms::ControlTypes::ListView)
		//{
		//	Brush = std::make_unique<Drawing::SolidBrush>(ListItemColorBase);
		//}

		Drawing::Pen Pen(Brush.get());

		EventArgs->Graphics->DrawRectangle(&Pen, Rect);
	}

	void GreyhoundTheme::RenderControlBackground(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State) const
	{
		Drawing::Rectangle Rect(Ctrl->ClientRectangle());

		auto brush = Drawing::SolidBrush(BackgroundBrush);

		EventArgs->Graphics->FillRectangle(&brush, Rect);
	}

	void GreyhoundTheme::RenderControlButtonBackground(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State) const
	{
		Drawing::Rectangle Rect(Ctrl->ClientRectangle());

		std::unique_ptr<Drawing::Brush> DrawBrush = nullptr;

		switch (State)
		{
		case UIX::UIXRenderState::Default:
			DrawBrush = std::make_unique<Drawing::LinearGradientBrush>(Rect, BackgroundGrad1, BackgroundGrad2, 90.f);
			break;
		case UIX::UIXRenderState::Disabled:
			DrawBrush = std::make_unique<Drawing::LinearGradientBrush>(Rect, BackgroundGrad1, BackgroundGrad2, 90.f);
			break;
		case UIX::UIXRenderState::MouseOver:
			DrawBrush = std::make_unique<Drawing::LinearGradientBrush>(Rect, BackgroundOverGrad1, BackgroundOverGrad2, 90.f);
			break;
		case UIX::UIXRenderState::MouseDown:
			DrawBrush = std::make_unique<Drawing::LinearGradientBrush>(Rect, BackgroundGrad1, BackgroundGrad2, 270.f);
			break;
		}

		EventArgs->Graphics->FillRectangle(DrawBrush.get(), Rect);
	}

	void GreyhoundTheme::RenderControlText(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State, Drawing::Rectangle LayoutRect, Drawing::ContentAlignment Alignment) const
	{
		// Fetch color from foreground
		Drawing::SolidBrush TextBrush((State == UIX::UIXRenderState::Disabled) ? TextDisabledBrush : TextEnabledBrush);

		// Fetch control text and font handle
		auto Text = Ctrl->Text().ToWString();
		auto Font = Ctrl->GetFont()->GetFont();

		// Setup string formatting
		Gdiplus::StringFormat StrFmt;

		// Handle horizontal alignment
		if (((int)Alignment & (int)Drawing::AnyLeftAlign) != 0)
			StrFmt.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		else if (((int)Alignment & (int)Drawing::AnyRightAlign) != 0)
			StrFmt.SetAlignment(Gdiplus::StringAlignment::StringAlignmentFar);
		else
			StrFmt.SetAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);

		// Handle vertical alignment
		if (((int)Alignment & (int)Drawing::AnyTopAlign) != 0)
			StrFmt.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		else if (((int)Alignment & (int)Drawing::AnyBottomAlign) != 0)
			StrFmt.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentFar);
		else
			StrFmt.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);

		// Build text layout rect
		Gdiplus::RectF TextLayoutRect((float)LayoutRect.X, (float)LayoutRect.Y, (float)LayoutRect.Width, (float)LayoutRect.Height);

		// Render text to the surface
		EventArgs->Graphics->DrawString((wchar_t*)Text, Text.Length(), Font.get(), TextLayoutRect, &StrFmt, &TextBrush);
	}

	void GreyhoundTheme::RenderControlProgressFill(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State, uint32_t Progress) const
	{
		// Bring client rect to stack
		Drawing::Rectangle Rect(Ctrl->ClientRectangle());

		// Generate the proper gradient if enabled/disabled
		Drawing::LinearGradientBrush FillBrush(Rect, ProgressGrad1, ProgressGrad2, 90.f);

		// Render the fill to surface
		EventArgs->Graphics->FillRectangle(&FillBrush, Gdiplus::RectF(2, 2, (Rect.Width - 4.f) * (Progress / 100.0f), Rect.Height - 4.f));
	}

	void GreyhoundTheme::RenderControlGlyph(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State) const
	{
		// Bring client rect to stack
		Drawing::Rectangle Rect(Ctrl->ClientRectangle());

		// Create Brush
		Drawing::SolidBrush FillBrush((State == UIX::UIXRenderState::Disabled) ? DisabledBorderBrush : BorderBrush);

		// It's already setup this way
		Rect.Width--;
		Rect.Height--;

		// Ensure smooth glyph
		EventArgs->Graphics->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);

		Gdiplus::GraphicsPath Path;
		Drawing::Rectangle RectCheck(Rect.Width - 18, 0, 18, Rect.Height - 1);

		// Rotate the glyph
		EventArgs->Graphics->TranslateTransform(RectCheck.X + RectCheck.Width / 2.f, RectCheck.Y + RectCheck.Height / 2.f);

		// Draw the triangle
		Path.AddLine(Drawing::PointF(-6 / 2.0f, -3 / 2.0f), Drawing::PointF(6 / 2.0f, -3 / 2.0f));
		Path.AddLine(Drawing::PointF(6 / 2.0f, -3 / 2.0f), Drawing::PointF(0, 6 / 2.0f));
		Path.CloseFigure();

		// Reset rotation
		EventArgs->Graphics->RotateTransform(0);

		// Render the glyph to surface
		EventArgs->Graphics->FillPath(&FillBrush, &Path);
	}

	void GreyhoundTheme::RenderControlCheckBoxBorder(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State) const
	{
		Drawing::Rectangle Rect(Ctrl->ClientRectangle());

		// Create the border brush
		auto brush = Drawing::SolidBrush((State == UIX::UIXRenderState::Disabled) ? DisabledBorderBrush : BorderBrush);
		Drawing::Pen Pen(&brush);

		// Calculate box sizing
		Drawing::Rectangle BoxRect(0, 0, Rect.Height - 1, Rect.Height - 1);
		Drawing::Rectangle FillRect(1, 1, Rect.Height - 2, Rect.Height - 2);

		// Create the fill brush
		std::unique_ptr<Drawing::Brush> FillBrush;

		// Change based on state
		switch (State)
		{
		case UIX::UIXRenderState::Disabled:
			FillBrush = std::make_unique<Drawing::LinearGradientBrush>(FillRect, DisabledBorderBrush, DisabledBorderBrush, 90.f);
			break;
		case UIX::UIXRenderState::Default:
			FillBrush = std::make_unique<Drawing::LinearGradientBrush>(FillRect, BackgroundGrad1, BackgroundGrad2, 90.f);
			break;
		case UIX::UIXRenderState::MouseOver:
			FillBrush = std::make_unique<Drawing::LinearGradientBrush>(FillRect, BackgroundOverGrad1, BackgroundOverGrad2, 90.f);
			break;
		case UIX::UIXRenderState::Selected:
			FillBrush = std::make_unique<Drawing::LinearGradientBrush>(FillRect, ProgressGrad1, ProgressGrad2, 90.f);
			break;
		}

		// Render the boxes to surface
		EventArgs->Graphics->FillRectangle(FillBrush.get(), FillRect);
		EventArgs->Graphics->DrawRectangle(&Pen, BoxRect);
	}

	void GreyhoundTheme::RenderControlCheckBoxCheck(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State) const
	{
		Drawing::Rectangle Rect(Ctrl->ClientRectangle());
		Drawing::Rectangle CheckRect(((Rect.Height - 12) / 2) + 1, (Rect.Height - 12) / 2, 12, 12);

		// Render the image to the surface
		if (State == UIX::UIXRenderState::Selected)
			EventArgs->Graphics->DrawImage(CheckBoxImage, CheckRect);
	}

	void GreyhoundTheme::RenderControlRadioBorder(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State) const
	{
		Drawing::Rectangle Rect(Ctrl->ClientRectangle());

		// Determine which fill to use
		auto brush = Drawing::SolidBrush((State == UIX::UIXRenderState::Disabled) ? DisabledBorderBrush : BorderBrush);
		Drawing::Pen Pen(&brush);

		// Adjust for overhang
		Rect.Width--;
		Rect.Height--;

		// Render the circle to surface
		auto SmMode = EventArgs->Graphics->GetSmoothingMode();
		{
			EventArgs->Graphics->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
			EventArgs->Graphics->DrawEllipse(&Pen, Drawing::Rectangle(0, 0, Rect.Height, Rect.Height));
		}
		EventArgs->Graphics->SetSmoothingMode(SmMode);
	}

	void GreyhoundTheme::RenderControlRadioCheck(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State) const
	{
		Drawing::Rectangle Rect(Ctrl->ClientRectangle());
		Drawing::Rectangle FillRect(1, 1, Rect.Height - 3, Rect.Height - 3);

		// Create the fill brush
		std::unique_ptr<Drawing::Brush> FillBrush;

		// Change based on state
		switch (State)
		{
		case UIX::UIXRenderState::Disabled:
			FillBrush = std::make_unique<Drawing::LinearGradientBrush>(FillRect, DisabledBorderBrush, DisabledBorderBrush, 90.f);
			FillRect = { 3, 3, (INT)Rect.Height - 7, (INT)Rect.Height - 7 };
			break;
		case UIX::UIXRenderState::Default:
			FillBrush = std::make_unique<Drawing::LinearGradientBrush>(FillRect, BackgroundGrad1, BackgroundGrad2, 90.f);
			break;
		case UIX::UIXRenderState::MouseOver:
			FillBrush = std::make_unique<Drawing::LinearGradientBrush>(FillRect, BackgroundOverGrad1, BackgroundOverGrad2, 90.f);
			break;
		case UIX::UIXRenderState::Selected:
			FillBrush = std::make_unique<Drawing::LinearGradientBrush>(FillRect, ProgressGrad1, ProgressGrad2, 90.f);
			FillRect = { 3, 3, (INT)Rect.Height - 7, (INT)Rect.Height - 7 };
			break;
		}

		// Render the state
		auto SmMode = EventArgs->Graphics->GetSmoothingMode();
		{
			EventArgs->Graphics->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
			EventArgs->Graphics->FillEllipse(FillBrush.get(), FillRect);
		}
		EventArgs->Graphics->SetSmoothingMode(SmMode);
	}

	void GreyhoundTheme::RenderControlGroupBox(const std::unique_ptr<Forms::PaintEventArgs>& EventArgs, Forms::Control* Ctrl, UIX::UIXRenderState State) const
	{
		Drawing::Rectangle Rect(Ctrl->ClientRectangle());

		Drawing::SolidBrush BackBrush(BackgroundBrush);
		Drawing::SolidBrush TextBrush((State == UIX::UIXRenderState::Disabled) ? TextDisabledBrush : TextEnabledBrush);

		EventArgs->Graphics->FillRectangle(&BackBrush, Rect);

		auto Text = Ctrl->Text().ToWString();
		auto Font = Ctrl->GetFont()->GetFont();

		Gdiplus::RectF TextLayoutRect(0.0f, 0.0f, (float)Rect.Width, (float)Rect.Height);
		Gdiplus::RectF TextSize;

		EventArgs->Graphics->MeasureString((wchar_t*)Text, Text.Length(), Font.get(), TextLayoutRect, &TextSize);
		EventArgs->Graphics->DrawString((wchar_t*)Text, Text.Length(), Font.get(), Gdiplus::PointF(12, 0), &TextBrush);

		Drawing::PointF Lines[] =
		{
			// Top-left
			{0, TextSize.Height / 2.f},
			{12, TextSize.Height / 2.f},
			// Left
			{0, TextSize.Height / 2.f},
			{0, Rect.Height - 1.f},
			// Bottom
			{0, Rect.Height - 1.f},
			{Rect.Width - 1.f, Rect.Height - 1.f},
			// Right
			{Rect.Width - 1.f, Rect.Height - 1.f},
			{Rect.Width - 1.f, TextSize.Height / 2.f},
			// Top-right
			{TextSize.Width + 11.f, TextSize.Height / 2.f},
			{Rect.Width - 1.f, TextSize.Height / 2.f}
		};

		auto pen = Drawing::Pen(&TextBrush);
		EventArgs->Graphics->DrawLines(&pen, &Lines[0], _countof(Lines));
	}

	void GreyhoundTheme::RenderControlListColumnHeader(const std::unique_ptr<Forms::DrawListViewColumnHeaderEventArgs>& EventArgs, Forms::Control* Ctrl) const
	{
		// Fetch color from foreground
		Drawing::SolidBrush TextBrush(TextEnabledBrush);
		Drawing::LinearGradientBrush BackBrush(EventArgs->Bounds, HeaderGrad1, HeaderGrad2, 90.f);

		// Make the separator brush
		Drawing::SolidBrush sepBrush(HeaderSeparatorColor);
		// The separator pen
		Drawing::Pen sepPen(&sepBrush);

		// Fetch control text and font handle
		auto Text = EventArgs->Header->Text().ToWString();
		auto Font = Ctrl->GetFont()->GetFont();

		// Setup string formatting
		Gdiplus::StringFormat StrFmt;
		StrFmt.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		StrFmt.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
		StrFmt.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);

		// Build text layout rect
		Gdiplus::RectF TextLayoutRect((float)EventArgs->Bounds.X + 3, (float)EventArgs->Bounds.Y, (float)EventArgs->Bounds.Width - 6, (float)EventArgs->Bounds.Height);
		Gdiplus::RectF TextSize;

		EventArgs->Graphics->MeasureString((wchar_t*)Text, Text.Length(), Font.get(), Drawing::PointF(0, 0), &TextSize);

		TextLayoutRect.Height = TextSize.Height - 1;
		TextLayoutRect.Y = (TextLayoutRect.Y) + (TextSize.Height / 2.f);
		TextLayoutRect.Y--;	// There is always one overlap pixel at bottom/right

		// Render to the surface
		EventArgs->Graphics->FillRectangle(&BackBrush, EventArgs->Bounds);

		auto AdjustWidth = (EventArgs->ColumnIndex == (4 - 1)) ? 1 : 2;
		// Render the divider
		if(EventArgs->ColumnIndex != 0)
			EventArgs->Graphics->DrawLine(&sepPen, Drawing::Point(EventArgs->Bounds.X, 0), Gdiplus::Point(EventArgs->Bounds.X, EventArgs->Bounds.Height - 2));

		EventArgs->Graphics->DrawString((wchar_t*)Text, Text.Length(), Font.get(), TextLayoutRect, &StrFmt, &TextBrush);
		auto brush = Drawing::SolidBrush(HeaderBorderColour);
		auto pen = Drawing::Pen(&brush);
		EventArgs->Graphics->DrawLine(&pen, Drawing::Point(EventArgs->Bounds.X, EventArgs->Bounds.Height - 1), Drawing::Point(EventArgs->Bounds.X + EventArgs->Bounds.Width, EventArgs->Bounds.Height - 1));
	}

	void GreyhoundTheme::RenderControlListHeader(const std::unique_ptr<Drawing::BufferedGraphics>& EventArgs) const
	{
		Drawing::LinearGradientBrush BackBrush(EventArgs->Region(), HeaderGrad1, HeaderGrad2, 90.f);
		auto brush = Drawing::SolidBrush(BackgroundLightBrush);
		EventArgs->Graphics->FillRectangle(&brush, EventArgs->Region());
		//Drawing::SolidBrush BackBrush(HeaderBrush);
		Drawing::Rectangle NewBounds(EventArgs->Region());

		// Make the separator brush
		Drawing::SolidBrush sepBrush(HeaderSeparatorColor);
		// The separator pen
		Drawing::Pen sepPen(&sepBrush);

		EventArgs->Graphics->FillRectangle(&BackBrush, NewBounds);

		EventArgs->Graphics->DrawLine(&sepPen, Drawing::Point(NewBounds.X, 0), Gdiplus::Point(NewBounds.X, NewBounds.Height - 2));

		auto borderBrush = Drawing::SolidBrush(HeaderBorderColour);
		auto pen = Drawing::Pen(&borderBrush);
		EventArgs->Graphics->DrawLine(&pen, Drawing::Point(NewBounds.X, NewBounds.Height -1), Drawing::Point(NewBounds.X + NewBounds.Width, NewBounds.Height -1));

		EventArgs->Render();
	}

	void GreyhoundTheme::RenderControlListItem(const std::unique_ptr<Forms::DrawListViewItemEventArgs>& EventArgs, Forms::Control* Ctrl, Drawing::Rectangle SubItemBounds) const
	{
		// Fetch the state because we are owner draw
		auto State = SendMessageA(Ctrl->GetHandle(), LVM_GETITEMSTATE, (WPARAM)EventArgs->ItemIndex, (LPARAM)LVIS_SELECTED);

		// Fetch color from style
		Drawing::SolidBrush TextBrush((State == LVIS_SELECTED) ? Drawing::Color::White : EventArgs->Style.ForeColor);
		Drawing::SolidBrush BackBrush((State == LVIS_SELECTED) ? BorderBrush : EventArgs->Style.BackColor);

		// Fetch control text and font handle
		auto Text = EventArgs->Text.ToWString();
		auto Font = Ctrl->GetFont()->GetFont();

		// Setup string formatting
		Gdiplus::StringFormat StrFmt;
		StrFmt.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		StrFmt.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
		StrFmt.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
		StrFmt.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

		// Build text layout rect
		Gdiplus::RectF TextLayoutRect((float)SubItemBounds.X, (float)SubItemBounds.Y, (float)SubItemBounds.Width, (float)SubItemBounds.Height);
		Gdiplus::RectF TextSize;

		EventArgs->Graphics->MeasureString((wchar_t*)Text, Text.Length(), Font.get(), Drawing::PointF(0, 0), &TextSize);

		TextLayoutRect.Height = TextSize.Height;
		TextLayoutRect.Y = (TextLayoutRect.Y) + ((SubItemBounds.Height / 2.f) - (TextSize.Height / 2.f));

		EventArgs->Graphics->FillRectangle(&BackBrush, SubItemBounds);
		EventArgs->Graphics->DrawString((wchar_t*)Text, Text.Length(), Font.get(), TextLayoutRect, &StrFmt, &TextBrush);
	}

	void GreyhoundTheme::RenderControlListSubItem(const std::unique_ptr<Forms::DrawListViewSubItemEventArgs>& EventArgs, Forms::Control* Ctrl) const
	{
		// Use stock bounds, subitems are valid
		Drawing::Rectangle SubItemBounds(EventArgs->Bounds);

		// Fetch the state because we are owner draw
		auto State = SendMessageA(Ctrl->GetHandle(), LVM_GETITEMSTATE, (WPARAM)EventArgs->ItemIndex, (LPARAM)LVIS_SELECTED);

		// Fetch color from style
		Drawing::SolidBrush TextBrush((State == LVIS_SELECTED) ? Drawing::Color::Black : EventArgs->Style.ForeColor);
		Drawing::SolidBrush BrushDefault((State == LVIS_SELECTED) ? DefaultForeground : ListItemColorBase);
		Drawing::SolidBrush BrushAlt((State == LVIS_SELECTED) ? DefaultForeground : ListItemColorAlt);

		// Fetch control text and font handle
		auto Text = EventArgs->Text.ToWString();
		auto Font = Ctrl->GetFont()->GetFont();

		// Setup string formatting
		Gdiplus::StringFormat StrFmt;
		StrFmt.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		StrFmt.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
		StrFmt.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
		StrFmt.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

		// Build text layout rect
		Gdiplus::RectF TextLayoutRect((float)SubItemBounds.X, (float)SubItemBounds.Y, (float)SubItemBounds.Width, (float)SubItemBounds.Height);
		Gdiplus::RectF TextSize;

		EventArgs->Graphics->MeasureString((wchar_t*)Text, Text.Length(), Font.get(), Drawing::PointF(0, 0), &TextSize);

		TextLayoutRect.Height = TextSize.Height;
		TextLayoutRect.Y = (TextLayoutRect.Y) + ((SubItemBounds.Height / 2.f) - (TextSize.Height / 2.f));

		EventArgs->Graphics->FillRectangle(EventArgs->ItemIndex % 2 ? &BrushAlt : &BrushDefault, SubItemBounds);
		EventArgs->Graphics->DrawString((wchar_t*)Text, Text.Length(), Font.get(), TextLayoutRect, &StrFmt, &TextBrush);
	}

	void GreyhoundTheme::RenderControlToolTip(const std::unique_ptr<Forms::DrawToolTipEventArgs>& EventArgs, Forms::Control* Ctrl) const
	{
		Drawing::SolidBrush Brush(BackgroundBrush);

		// Render to surface
		EventArgs->Graphics->FillRectangle(&Brush, EventArgs->Bounds);

		Drawing::Rectangle BoundsOne(EventArgs->Bounds);
		BoundsOne.Inflate(-1, -1);


		auto brush = Drawing::SolidBrush(BorderBrush);
		auto pen = Drawing::Pen(&brush);

		EventArgs->Graphics->DrawRectangle(&pen, BoundsOne);

		Drawing::SolidBrush TextBrush(TextEnabledBrush);
		Gdiplus::RectF BoundsF((float)EventArgs->Bounds.X, (float)EventArgs->Bounds.Y, (float)EventArgs->Bounds.Width, (float)EventArgs->Bounds.Height);

		Gdiplus::StringFormat Fmt;
		Fmt.SetAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
		Fmt.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);

		auto WideString = Ctrl->Text().ToWString();
		auto Fnt = Ctrl->GetFont()->GetFont();

		// Render text
		EventArgs->Graphics->DrawString((wchar_t*)WideString, WideString.Length(), Fnt.get(), BoundsF, &Fmt, &TextBrush);
	}


	Drawing::Color GreyhoundTheme::GetRenderColor(UIX::UIXRenderColor Color) const
	{
		switch (Color)
		{
		case UIX::UIXRenderColor::TextDefault:
			return TextEnabledBrush;
		case UIX::UIXRenderColor::BackgroundDefault:
			return BackgroundBrush;
		case UIX::UIXRenderColor::BackgroundLight:
			return BackgroundBrush;
		default:
			return Drawing::Color();
		}
	}
}
