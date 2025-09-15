#pragma once

#include "gigperformer/sdk/GPMidiMessages.h"
#include "gigperformer/sdk/GPUtils.h"
#include "gigperformer/sdk/GigPerformerAPI.h"
#include "gigperformer/sdk/types.h"
#include "LibMain.h"
#include "MCU_Classes.h"

// SurfaceWidget member functions

bool SurfaceWidget::IsFader()
{
	return (WidgetID == FADER_TAG);
}

bool SurfaceWidget::IsKnob()
{
	return (WidgetID == KNOB_TAG);
}

bool SurfaceWidget::IsSoftbutton()
{
	return (WidgetID == SOFTBUTTON_TAG);
}



// basic code to format the two-line softbutton display
// could improve it by deciding where to line break if > 8 characters
bool P1Softbutton::formatSoftbuttonText(std::string label)
{
	std::string newlabel = cleanSysex(label);

	if (newlabel.length() < 8)
	{
		Format = newlabel.length() % 2 ? 1 : 3; // even or odd number of characters, centered in display
		Label = std::format("{:^8}", trim(newlabel)) + std::format("{:^8}", trim("  ")); // we pad line two with spaces but they're ignored
	}
	else if (newlabel.length() == 8)
	{
		Format = 2; // full 8 character line, centered in display
		Label = std::format("{:^8}", trim(newlabel)) + std::format("{:^8}", trim("  "));
	}
	else if (newlabel.length() <= 16) // show everything if 16 or under
	{
		Format = 7;
		Label = std::format("{:^8}", newlabel.substr(0, 8)) + std::format("{:^8}", newlabel.substr(8, 8));
	}
	else // if more than 16 we strip vowels that aren't the first chracter of a word
	{
		newlabel = std::regex_replace(newlabel, std::regex(R"(\B[aeiouAEIOU])"), "");
		Format = 7;
		Label = std::format("{:^8}", newlabel.substr(0, 8)) + std::format("{:^8}", newlabel.substr(8, 8));
	}
	return true;
}


bool P1SoftbuttonArray::setLabel(uint8_t position, P1Softbutton button)
{
	if (position < 80) {
		Buttons[position] = button;
		Dirty[(uint8_t)(position / P1M_NAMES_PER_PAGE)] = true;
		return true;
	}
	else return false;
}

bool P1SoftbuttonArray::setLabel(uint8_t position, std::string text)
{
	if (position < 80) {
		Buttons[position].formatSoftbuttonText(text);
		Dirty[(uint8_t) (position / P1M_NAMES_PER_PAGE)] = true;
		return true;
	}
	else return false;
}


