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

// basic code to format the two-line softbutton display
// could improve it by deciding where to line break if > 8 characters
P1Softbutton LibMain::formatSoftbuttonText(std::string label)
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

// Send the softbutton labels to the P1-M
// currently sends the entire five pages of buttons, although would like to narrow it down to range first-last
std::string LibMain::SendSoftbuttons(uint8_t first, uint8_t last)
{
    std::string sysex, hexsysex;
    uint8_t lines, loop, position;
    bool touched, linetouched;

    touched = false;
    for (lines = 0; lines < (80 / P1M_NAMES_PER_PAGE); lines++)
    {
        // sysex = gigperformer::sdk::GPUtils::hex2binaryString(P1M_NAME_START);
        // sysex[P1M_NAME_PAGE] = lines - 1;

        hexsysex = P1M_NAME_START;
        hexsysex = hexsysex.replace(21, 2, std::format("{:02x}", lines + 1));
        linetouched = false;

        for (loop = 0; loop < P1M_NAMES_PER_PAGE; loop++)
        {
            position = loop + lines * P1M_NAMES_PER_PAGE;
            if (Surface.P1SoftbuttonArray[position].Label != Surface.LastSoftbuttonArray[position].Label)
            {
                linetouched = true;
                touched = true;
                Surface.LastSoftbuttonArray[position].Label = Surface.P1SoftbuttonArray[position].Label;
                // sysex += Surface.P1SoftbuttonArray[position].Format;
                // sysex += Surface.P1SoftbuttonArray[position].Label;
            }

            hexsysex += std::format(" {:02x} ", Surface.P1SoftbuttonArray[position].Format) + textToHexString(Surface.P1SoftbuttonArray[position].Label);
        }

        // sysex += (uint8_t)0xf7;
        hexsysex += " f7";

        // only send the sysex for the line if it was touched, or some line was touched and it's the final line
        if (linetouched || (lines > 6 && touched)) {
            scriptLog(hexsysex, 0);
            sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex));
        }
        else scriptLog("P1M: line " + std::to_string(lines) + " skipped", 0);

    }
    return "xxx";
}

// Sends the P1-M port 4 sysex to define the softbutton control codes
std::string LibMain::SendSoftbuttonCodes(uint8_t first, uint8_t last)
{
    std::string sysex, hexsysex;
    uint8_t page, loop, notenum=54, channel=0, totalcount=0;

    // the softbutton key codes are sent to the P1-M on the first three pages of keydefs
    // then we also have to send the final page (9) or the P1-M ignores everything we sent
    for (page = 0; page < 3; page++)
    {
        // sysex = gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_START);
        // sysex[P1M_KEYDEF_PAGE] = lines - 1;

        hexsysex = P1M_KEYDEF_START;
        hexsysex = hexsysex.replace(21, 2, std::format("{:02x}", page + 1)); // replace page we're on

        for (loop = 0; loop < P1M_KEYDEFS_PER_PAGE; loop++)
        {
            hexsysex += std::format(" 09 09 {:02x} {:02x} 7f 00 00 00", channel, notenum);
            notenum++;
            if (notenum == 70)
            {
                if (page == 2 && channel == 4) break; // channel 5 marks end of softkeys
                notenum = 54;
                channel++;
            }
        }

        // we need the last four keydefs to be the chan left-right and bank left-right MCU definitions
        if (page == 2)
            hexsysex += " 09 09 00 30 7f 00 00 00 09 09 00 31 7f 00 00 00 09 09 00 2e 7f 00 00 00 09 09 00 2f 7f 00 00 00";
        // sysex += (uint8_t)0xf7;
        hexsysex += " f7";
        // scriptLog(hexsysex, 0);
        // sendPort4Message(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex)));
        sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex));
    }
    // send page 9 of keydefs or the P1-M ignores everything above
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE4));
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE5));
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE6));
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE7));
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE8));
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE9));
    return "xxx";
}


// set up basic softbutton printing routine


void LibMain::DisplayP1MText(uint8_t column, uint8_t row, std::string text, uint8_t maxlength)
{
    std::string hexmessage, subtext, binmessage;

    // Could probably handle this better...  Adding blanks to the text to display so we're guaranteed to clear whatever's there, then just use front 'maxlength' chars
    if (column < 8 && row < 4) {

        // If the bottom two lines of text worked as they should the below would work. But that butchers the spacing of the text
        /* subtext = cleanSysex(text);
        subtext = subtext.substr(0, maxlength) + "                                                                ";
        hexmessage = P1M_TEXT_HDR + gigperformer::sdk::GPUtils::intToHex(row * 0x38 + column * 7) + textToHexString(subtext.substr(0, (maxlength % 7 == 0) ? maxlength : maxlength + 7 - maxlength % 7)) + (std::string)" f7";
        binmessage = gigperformer::sdk::GPUtils::hex2binaryString(hexmessage);
        sendMidiMessage(binmessage); */

        // we need to write an entire line at once, so we store the two lines in a global variable and
        // splice in our changes then write the whole thing to the P1-M
        subtext = cleanSysex(text);
        subtext = subtext.substr(0, maxlength) + "                                                                                                               ";
        int insertpoint = (row-2) * 0x38 + 7 * column;


        Surface.P1MText.replace(insertpoint, maxlength, subtext.substr(0, (maxlength % 7 == 0) ? maxlength : maxlength + 7 - maxlength % 7));

        binmessage = gigperformer::sdk::GPUtils::hex2binaryString((std::string) P1M_TEXT_HDR + " 38");
        // binmessage += gigperformer::sdk::GPUtils::hex2binaryString("38"); // have to start writing at second line due to but in P1-M firmware
        binmessage += Surface.P1MText;
        binmessage += gigperformer::sdk::GPUtils::hex2binaryString("f7");
        sendMidiMessage(binmessage);

        // scriptLog("P1M: col: " + std::to_string(column) + " row: " + std::to_string(row) + " max: " + std::to_string(maxlength) + text, 0);
        // scriptLog("P1M: display " + Surface.P1MText, 0);
        
    }
}

// P1M color bars - we have to send them all at once in one sysex
void LibMain::DisplayP1MColorbars()
{
    std::string hexcolorstring = P1M_COLORBAR_PREFIX;

    for (uint8_t loop = 0; loop < 8; loop++)
    {
        hexcolorstring += GPColorToSLColorHex(Surface.P1MColorbars[loop]);
    }
    hexcolorstring += "F7";
    // sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(hexcolorstring));
    sendMidiMessage(gigperformer::sdk::GPUtils::hex2binaryString(hexcolorstring));
}
