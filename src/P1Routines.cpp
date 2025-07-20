#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <format>
#include "LibMain.h"
#include "General_Utils.h"


// routine that is called periodically to send softbutton text to the P1-M
// if we send data too often it appears to overflow the P1-M input buffer resulting in gibberish displayed
// we store all of the softbutton data in an array, then periodically send it out
void LibMain::ScheduledSoftsend()
{
    std::string hexsysex, sysex;
    uint8_t lines, loop, position;
    bool touched, linetouched;  // to reduce data volume we only send data if the sysex string it belongs to changed

    // if nothing has changed we don't send anything
    if (Controller.Instance[1].SoftbuttonArray.Dirty == true)
    {
        Controller.Instance[1].SoftbuttonArray.Dirty = false;
        touched = false;
        for (lines = 0; lines < (80 / P1M_NAMES_PER_PAGE); lines++)
        {
            hexsysex = P1M_NAME_START;
            hexsysex = hexsysex.replace(P1M_NAME_PAGE*3, 2, std::format("{:02x}", lines + 1)); // which chunk we're on
            linetouched = false;

            for (loop = 0; loop < P1M_NAMES_PER_PAGE; loop++)
            {
                position = loop + lines * P1M_NAMES_PER_PAGE;
                if (Controller.Instance[1].SoftbuttonArray.Buttons[position].Label != Controller.Instance[1].SoftbuttonArray.LastButtons[position].Label)
                {
                    linetouched = true;
                    touched = true;
                    // Controller.Instance[1].SoftbuttonArray.LastButtons[position].Label = Controller.Instance[1].SoftbuttonArray.Buttons[position].Label;
                }
                hexsysex += std::format(" {:02x} ", Controller.Instance[1].SoftbuttonArray.Buttons[position].Format) + textToHexString(Controller.Instance[1].SoftbuttonArray.Buttons[position].Label);
            }
            hexsysex += " f7";


            // only send the sysex for the line if it was touched, or some line was touched and it's the final line
            if (linetouched || (lines > 6 && touched)) {
                // scriptLog(hexsysex, 0);
                // sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex));
                sysex = gigperformer::sdk::GPUtils::hex2binaryString(hexsysex);
				scriptLog("P1M: Sending line " + std::to_string(lines) + ": " + hexsysex, 0);
                sendMidiMessageToMidiOutDevice(Controller.Instance[1].PortFourOut, sysex);
            }
            // else scriptLog("P1M: line " + std::to_string(lines) + " skipped", 0);
        }
        // hexsysex = Controller.Instance[1].Softsend();
		// QueueMidi(hexsysex); // queue the sysex message to be sent later, to avoid overloading the P1-M port 4
        // sendMidiMessageToMidiOutDevice(Controller.Instance[1].PortFourOut, hexsysex); // overflows and garbles the display
        // scriptLog("P1M - new line: " + std::to_string(hexsysex.length()) + textToHexString(hexsysex), 0);
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
    uint8_t page, loop, notenum = 54, channel = 0, totalcount = 0;

    // the softbutton key codes are sent to the P1-M on the first three pages of keydefs
    // then we also have to send the final page (9) or the P1-M ignores everything we sent
    for (page = 0; page < 3; page++)
    {
        hexsysex = P1M_KEYDEF_START;
        hexsysex = hexsysex.replace(3 * P1M_NAME_PAGE, 2, std::format("{:02x}", page + 1)); // replace page we're on

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

// the idea of this was to build a queue of sysex to avoid overflowing the P1-M, but it doesn't seem necessary
// so I am just sending the sysex immediately
bool LibMain::QueueMidi(std::string midiMessage)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = midiMessage.find((uint8_t)0xf7);

    while (end != std::string::npos) {
        // SysexQueue.push(midiMessage.substr(start, end - start + 1));
        sendPort4Message(midiMessage.substr(start, end - start + 1)); // send each sysex message immediately
        start = end + 1;
        end = midiMessage.find((uint8_t)0xf7, start);
    }
    // SysexQueue.push(midiMessage.substr(start)); // Add any non-sysex remnants left at the end
    return true;
}

// this was intended to pop and send the first sysex message from a queue, and do so on a timer, but
// right now it doesn't seem necessary.  I was going to wait for ACKs from the P1-M on port 4, but that just slows things down
bool LibMain::SendQueuedMidi()
{
    if (!SysexQueue.empty())
    {
        std::string sysex = SysexQueue.front();
        SysexQueue.pop();
        sendPort4Message(sysex);
        // scriptLog("P1-M: Sending queued sysex: " + textToHexString(sysex), 0);
        return true;
    }
	// scriptLog("P1-M: No queued sysex to send", 0);
    return false;
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
//void LibMain::SendSoftbuttons(uint8_t first, uint8_t last)
//{
//    std::string hexsysex;
//    uint8_t lines, loop, position;
//    bool touched, linetouched;
//
//    if (false)
//    {
//        Controller.Instance[1].SoftbuttonArray.Dirty = true;
//    }
//    else
//    {
//        touched = false;
//        for (lines = 0; lines < (80 / P1M_NAMES_PER_PAGE); lines++)
//        {
//            hexsysex = P1M_NAME_START;
//            hexsysex = hexsysex.replace(21, 2, std::format("{:02x}", lines + 1)); // which chunk we're on
//            linetouched = false;
//
//            for (loop = 0; loop < P1M_NAMES_PER_PAGE; loop++)
//            {
//                position = loop + lines * P1M_NAMES_PER_PAGE;
//                if (Controller.Instance[1].SoftbuttonArray.Buttons[position].Label != Controller.Instance[1].SoftbuttonArray.LastButtons[position].Label)
//                {
//                    linetouched = true;
//                    touched = true;
//                    Controller.Instance[1].SoftbuttonArray.LastButtons[position].Label = Controller.Instance[1].SoftbuttonArray.Buttons[position].Label;
//                }
//
//                hexsysex += std::format(" {:02x} ", Controller.Instance[1].SoftbuttonArray.Buttons[position].Format) + textToHexString(Controller.Instance[1].SoftbuttonArray.Buttons[position].Label);
//            }
//            hexsysex += " f7";
//
//            // only send the sysex for the line if it was touched, or some line was touched and it's the final line
//            if (linetouched || (lines > 6 && touched)) {
//                // scriptLog(hexsysex, 0);
//                sendPort4Message(gigperformer::sdk::GPUtils::hex2binaryString(hexsysex));
//            }
//            // else scriptLog("P1M: line " + std::to_string(lines) + " skipped", 0);
//        }
//    }
//    // lambdaDemo("nothing");
//}


// places text at center of 8 character field, padding with correct number of spaces
/* std::string centerText(std::string text, int maxAmount = 8)
{
    return std::format("{:^8}", trim(text)); // std::regex_replace(text, std::regex("^ +| +$|( ) +"), "$1"));
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
        Softbutton.Label = centerText(newlabel) + centerText("  "); // we pad line two with spaces but they're ignored
    }
    else if (newlabel.length() == 8)
    {
        Softbutton.Format = 2; // full 8 character line, centered in display
        Softbutton.Label = centerText(newlabel) + centerText("  ");
    }
    else if (newlabel.length() <= 16) // show everything if 16 or under
    {
        Softbutton.Format = 7;
        Softbutton.Label = centerText(newlabel.substr(0, 8)) + centerText(newlabel.substr(8, 8));
    }
    else // if more than 16 we strip vowels that aren't the first chracter of a word
    {
        newlabel = std::regex_replace(newlabel, std::regex(R"(\B[aeiouAEIOU])"), "");
        Softbutton.Format = 7;
        Softbutton.Label = centerText(newlabel.substr(0, 8)) + centerText(newlabel.substr(8, 8));
    }
    return Softbutton;
} */

// InitializeSoftbuttons() initializes the data structure that holds formatted Softbutton data in the Surface class
//void LibMain::InitializeSoftbuttons()
//{
//    std::string label;
//
//    for (int x = 0; x < 80; x++)
//    {
//        label = "Soft " + std::to_string(x + 1);
//
//        Controller.Instance[1].SoftbuttonArray.set(x, label);
//    }
//}




// P1M color bars - we have to send them all at once in one sysex
//void LibMain::DisplayP1MColorbars()
//{
//    std::string hexcolorstring = P1M_COLORBAR_PREFIX;
//
//    for (uint8_t loop = 0; loop < 8; loop++)
//    {
//        hexcolorstring += GPColorToSLColorHex(Controller.Instance[1].P1MColorbars[loop]);
//    }
//    hexcolorstring += "F7";
//    // sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(hexcolorstring));
//    sendMidiMessage(gigperformer::sdk::GPUtils::hex2binaryString(hexcolorstring));
//}
