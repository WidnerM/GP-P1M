#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <format>
#include "LibMain.h"


std::string centerText(std::string text, int maxAmount = 8)
{
    return std::format("{:^8}", text);
}

P1Softbutton formatSoftbuttonText(std::string label)
{
    P1Softbutton Softbutton;
    std::string newlabel = cleanSysex(label);

    if (newlabel.length() < 8)
    {
        Softbutton.Format = newlabel.length() % 2 ? 1 : 3; // even or odd number of characters, centered in display
        Softbutton.Label = centerText(newlabel) + centerText("  ");
    }
    else if (newlabel.length() == 8)
    {
        Softbutton.Format = 2; // full 8 character line, centered in display
        Softbutton.Label = centerText(newlabel) + centerText("  ");
    }
    else
    {
        Softbutton.Format = 7;
        Softbutton.Label = centerText(newlabel.substr(0, 8)) + centerText(newlabel.substr(8, 8));
    }
    return Softbutton;
}

// InitializeSoftbuttons() just initializes the data structure that holds formatted Softbutton data in the Surface class
void LibMain::InitializeSoftbuttons()
{
    std::string label;

    for (int x = 0; x < 80; x++)
    {
        label = "Soft " + std::to_string(x + 1);

        Surface.P1SoftbuttonArray[x] = formatSoftbuttonText(label);
    }
}

std::string LibMain::SendSoftbuttons(uint8_t first, uint8_t last)
{
    std::string sysex, hexsysex;
    uint8_t lines, loop, position;

    for (lines = 0; lines < (80 / P1M_NAMES_PER_PAGE); lines++)
    {
        sysex = gigperformer::sdk::GPUtils::hex2binaryString(P1M_NAME_START);
        sysex[P1M_NAME_PAGE] = lines - 1;

        hexsysex = P1M_NAME_START;
        hexsysex = hexsysex.replace(21, 2, std::format("{:02x}", lines + 1));

        for (loop = 0; loop < P1M_NAMES_PER_PAGE; loop++)
        {
            position = loop + lines * P1M_NAMES_PER_PAGE;
            sysex += Surface.P1SoftbuttonArray[position].Format;
            sysex += Surface.P1SoftbuttonArray[position].Label;

            hexsysex += std::format(" {:02x} ", Surface.P1SoftbuttonArray[position].Format) + textToHexString(Surface.P1SoftbuttonArray[position].Label);
        }

        sysex += (uint8_t)0xf7;
        hexsysex += " f7";
        scriptLog(hexsysex, 0);
        // sendPort4Message(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex)));
        sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex));
    }
    return "xxx";
}

void LibMain::InitializeP1M()
{
    int x;

    // Surface.Initialize();

    // Surface.FirstShownSong = 0;

    // clear display
    CleanP1M();
}

void LibMain::ClearP1MDisplay()
{
    // scriptLog("MCU clear display", 1);
    sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(P1M_CLEAR_BOT)));
    sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(P1M_CLEAR_TOP)));
    sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(MCU_CLEAR_BOT)));
    sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(MCU_CLEAR_TOP)));
}

void LibMain::CleanP1M()
{
    int x;

    // clear secondary display
    ClearP1MDisplay();

    // shut off all leds
    for (x = 0; x <= 0x76; x++) {
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeNoteOnMessage(x, 0, 0));
    }

    // clear the knobs
    for (x = 0; x <= 7; x++) {
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeControlChangeMessage(0x30 + x, 0, 0));
    }

    // clear the VU meters on real MCUs
    for (x = 0; x <= 7; x++) {
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(MCU_CLEAR_VU + (std::string)"0" +
            std::to_string(x) + "00 F7")));
    }

}


// put knob text on top for P1M, fader text on top - if Surface.P1MType is true always do it this way
// set up basic softbutton printing routine
// set up label rewriting routine - integrate into DisplayRow, WidgetValueChanged??
// implement color bars

void LibMain::DisplayP1MText(uint8_t column, uint8_t row, std::string text, uint8_t maxlength)
{
    std::string hexmessage, subtext, binmessage;

    // Could probably handle this better...  Adding blanks to the text to display so we're guaranteed to clear whatever's there, then just use front 'maxlength' chars
    if (column < 8) {

        subtext = cleanSysex(text);
        subtext = subtext.substr(0, maxlength) + "                                                                                                               ";
        int insertpoint = row * 0x38 + 7 * column;

        Surface.P1MText.replace(insertpoint, 7, subtext.substr(0,7) );

        binmessage = gigperformer::sdk::GPUtils::hex2binaryString(P1M_TEXT_HDR);
        binmessage += Surface.P1MText;
        binmessage += gigperformer::sdk::GPUtils::hex2binaryString("f7");
        sendMidiMessage(binmessage);
    }
}
