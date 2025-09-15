#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <format>
#include "LibMain.h"
#include "General_Utils.h"


// Sends the P1-M port 4 sysex to define the softbutton control codes
// the structure of the P1-M protocol appears to be designed with the idea that we could write any
// individual keydef in isolation. As implemented we cannot.  We have to send groups of keydefs in
// specifically sized sysex strings which happen to be different between the P1-M, P1-Nano, and V1-M.
// For now we are setting up the softbutton area to send note-on events 54-69 (16 button grid) on
// each of the five softbutton pages. We put each page on its own channel number, with Blue being channel 1.
// In the current P1-M firmware these are the only 16 contiguous buttons that can be lit-unlit that are not
// already taken by the rest of the control surface. Unfortunately the P1-M cannot light-unlight anything
// but channel 1.  Hopefully they will enhance this at some point.
std::string LibMain::SendSoftbuttonCodes(uint8_t first, uint8_t last)
{
    std::string sysex, hexsysex;
    uint8_t page, loop, notenum=0, channel = 1, totalcount = 0;

    // the softbutton key codes are sent to the P1-M on the first three pages of keydefs
    // then we also have to send the final page (9) or the P1-M ignores everything we sent
    for (page = 0; page < 3; page++)
    {
        hexsysex = P1M_KEYDEF_START;
        hexsysex = hexsysex.replace(3 * P1M_NAME_PAGE, 2, std::format("{:02x}", page + 1)); // replace page we're on

        for (loop = 0; loop < P1M_KEYDEFS_PER_PAGE; loop++)
        {
            hexsysex += std::format(" 09 09 {:02x} {:02x} 7f 00 00 00", channel, notenum + Controller.Instance[1].Row[SOFTBUTTON_ROW].FirstID);
            notenum++;
            if (notenum == 16)
            {
                if (page == 2 && channel == 5) break; // channel 5 marks end of softkeys
                notenum = 0;
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
// it is built to manage sysex strings, not individual midi messages, but I should fix it to work with both
bool LibMain::QueueMidi(std::string midiMessage)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = midiMessage.find((uint8_t)0xf7);

    while (end != std::string::npos) {
        // SysexQueue.push(midiMessage.substr(start, end - start + 1));
		// scriptLog("sending port 4 msg " + textToHexString(midiMessage.substr(start, end-start + 1)), 0);
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

