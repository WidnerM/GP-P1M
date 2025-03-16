#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <format>
#include "LibMain.h"


void LibMain::ScheduledSoftsend()
{
    std::string hexsysex, sysex;
    uint8_t lines, loop, position;
    bool touched, linetouched;

    if (Surface.SoftbuttonArray.Dirty == true)
    {
        Surface.SoftbuttonArray.Dirty = false;
        touched = false;
        for (lines = 0; lines < (80 / P1M_NAMES_PER_PAGE); lines++)
        {
            hexsysex = P1M_NAME_START;
            hexsysex = hexsysex.replace(21, 2, std::format("{:02x}", lines + 1)); // which chunk we're on
            linetouched = false;

            for (loop = 0; loop < P1M_NAMES_PER_PAGE; loop++)
            {
                position = loop + lines * P1M_NAMES_PER_PAGE;
                if (Surface.SoftbuttonArray.Buttons[position].Label != Surface.SoftbuttonArray.LastButtons[position].Label)
                {
                    linetouched = true;
                    touched = true;
                    Surface.SoftbuttonArray.LastButtons[position].Label = Surface.SoftbuttonArray.Buttons[position].Label;
                }
                hexsysex += std::format(" {:02x} ", Surface.SoftbuttonArray.Buttons[position].Format) + textToHexString(Surface.SoftbuttonArray.Buttons[position].Label);
            }
            hexsysex += " f7";

            // scriptLog(hexsysex, 0);
            // scriptLog("Surface 8: " + surfacelink.SoftbuttonArray.Buttons[8].Label + " - refreshTimer 8: " + refreshTimer.softbuttonarray.Buttons[8].Label, 0);

            // only send the sysex for the line if it was touched, or some line was touched and it's the final line
            if (linetouched || (lines > 6 && touched)) {
                // scriptLog(hexsysex, 0);
                // sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex));
                sysex = gigperformer::sdk::GPUtils::hex2binaryString(hexsysex);
                sendMidiMessageToMidiOutDevice(Surface.PortFour, sysex);
            }
            // else scriptLog("P1M: line " + std::to_string(lines) + " skipped", 0);
        }
    }
}

// Send the softbutton labels to the P1-M
// the P1-M protocol requires sending this in 180 byte chunks (containing 10 softbutton labels each)
// where the last chunk (of 8 total) is mandatory but all preceding chunks (i.e., 1-7) are optional.
// The first-last parameters are not presently used because it hasn't
// proven helpful in working around the "challenge" that there seems to be a timing window in the P1-M
// where successive writes must be spaced out in time.  Specifically, the first-last parameters are
// ignored here because we compare every chunk to the last sent version of that chunk and only send updated ones.
// That has not been sufficient to address the issue, so my next effort will be to try putting this routine
// on a timer.
void LibMain::SendSoftbuttons(uint8_t first, uint8_t last)
{
    std::string hexsysex;
    uint8_t lines, loop, position;
    bool touched, linetouched;

    if (false)
    {
        Surface.SoftbuttonArray.Dirty = true;
    }
    else
    {
        touched = false;
        for (lines = 0; lines < (80 / P1M_NAMES_PER_PAGE); lines++)
        {
            hexsysex = P1M_NAME_START;
            hexsysex = hexsysex.replace(21, 2, std::format("{:02x}", lines + 1)); // which chunk we're on
            linetouched = false;

            for (loop = 0; loop < P1M_NAMES_PER_PAGE; loop++)
            {
                position = loop + lines * P1M_NAMES_PER_PAGE;
                if (Surface.SoftbuttonArray.Buttons[position].Label != Surface.SoftbuttonArray.LastButtons[position].Label)
                {
                    linetouched = true;
                    touched = true;
                    Surface.SoftbuttonArray.LastButtons[position].Label = Surface.SoftbuttonArray.Buttons[position].Label;
                }

                hexsysex += std::format(" {:02x} ", Surface.SoftbuttonArray.Buttons[position].Format) + textToHexString(Surface.SoftbuttonArray.Buttons[position].Label);
            }
            hexsysex += " f7";

            // only send the sysex for the line if it was touched, or some line was touched and it's the final line
            if (linetouched || (lines > 6 && touched)) {
                // scriptLog(hexsysex, 0);
                sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex));
            }
            // else scriptLog("P1M: line " + std::to_string(lines) + " skipped", 0);
        }
    }
    // lambdaDemo("nothing");
}

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

// InitializeSoftbuttons() initializes the data structure that holds formatted Softbutton data in the Surface class
void LibMain::InitializeSoftbuttons()
{
    std::string label;

    for (int x = 0; x < 80; x++)
    {
        label = "Soft " + std::to_string(x + 1);

        Surface.SoftbuttonArray.set(x,formatSoftbuttonText(label));
        // Surface.SoftbuttonArray.Buttons[x] = formatSoftbuttonText(label);
        // refreshTimer.softbuttonarray.set(x, formatSoftbuttonText(label));
    }
}

// Sends the P1-M port 4 sysex to define the softbutton control codes
// the structure of the P1-M protocol appears to be designed with the idea that we could write any
// individual keydef in isolation. As implemented we cannot.  We have to send groups of keydefs in
// specifically sized sysex strings which happen to be different between the P1-M, P1-Nano, and probably
// every other device in the P1 family.
// For now we are setting up the softbutton area to send note-on events 54-69 (16 button grid) on
// each of the five softbutton pages. We put each page on its own channel number, with Blue being channel 1.
// In the current P1-M firmware these are the only 16 contiguous buttons that can be lit-unlit that are not
// already taken by the rest of the control surface. Unfortunately the P1-M cannot light-unlight anything
// but channel 1.  Hopefully they will enhance this at some point.
std::string LibMain::SendSoftbuttonCodes(uint8_t first, uint8_t last)
{
    std::string sysex, hexsysex;
    uint8_t page, loop, notenum=54, channel=0, totalcount=0;

    // the softbutton key codes are sent to the P1-M on the first three pages of keydefs
    // then we also have to send the final page (9) or the P1-M ignores everything we sent
    for (page = 0; page < 3; page++)
    {
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
        hexsysex += " f7";
        // scriptLog(hexsysex, 0);
        // sendPort4Message(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex)));
        sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex));
    }
    // send page 9 of keydefs or the P1-M ignores everything above
    // for now I am sending everything because it seems the stuff on pages 4-8 gets corrupted now
    // and then, most likely as a result of periodically overflowing the buffers for the softbutton
    // display once in a while.  Once these are corrupted even a power cycle doesn't fix them.
    // These need to be set properly anyway for the extension to work, so might as well write them
    // all during the initialization routine.
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE4));
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE5));
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE6));
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE7));
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE8));
    sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(P1M_KEYDEF_PAGE9));
    return "xxx";
}



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
