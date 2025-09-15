#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include "LibMain.h"

void LibMain::ProcessSoftButton(uint8_t channel, uint8_t button, uint8_t value)  // processes a softbutton
{
    uint8_t songnumber;

    // we're putting Rackspaces/Songs on the U1 page and Variations/Songparts on the U2 page if in ShowRacksSongs mode (flip button)
    if (Controller.Instance[1].ShowRacksSongs)
    {
        songnumber = (button - Controller.Instance[1].Row[SOFTBUTTON_ROW].FirstID);

        if (channel == 1) // Songs and Racks on page (channel) one
        {
            if (inSetlistMode() == 1) {
                if ((songnumber + Controller.Instance[1].FirstShownSong) < getSongCount())
                {
                    Controller.Instance[1].reportWidgetChanges = false;
                    switchToSong(songnumber + Controller.Instance[1].FirstShownSong, 0);
                    Controller.Instance[1].reportWidgetChanges = Controller.Instance[1].reportWidgetMode;
                }
                return;
            }
            else
            {
                if ((songnumber + Controller.Instance[1].FirstShownRack) < getRackspaceCount())
                {
                    Controller.Instance[1].reportWidgetChanges = false;
                    switchToRackspaceName(getRackspaceName(songnumber + Controller.Instance[1].FirstShownRack));
                    Controller.Instance[1].reportWidgetChanges = Controller.Instance[1].reportWidgetMode;
                }
                return;
            }
        }
        else if (channel == 2) // Variations and Songparts on page (channel) two
        {
            if (inSetlistMode() == 1) {
                if (songnumber < getSongpartCount(getCurrentSongIndex()))
                {
                    Controller.Instance[1].reportWidgetChanges = false;
                    switchToSongPart(songnumber);
                    Controller.Instance[1].reportWidgetChanges = Controller.Instance[1].reportWidgetMode;
                }
                return;
            }
            else
            {
                if (songnumber < getVariationCount(getCurrentRackspaceIndex()))
                {
                    Controller.Instance[1].reportWidgetChanges = false;
                    switchToVariation(songnumber);
                    Controller.Instance[1].reportWidgetChanges = Controller.Instance[1].reportWidgetMode;
                }
                return;
            }
        }
    }
    
    // if we get here we just toggle the button
    // our button column number is channel * 16 + (button - firstID)
    ToggleButton(Controller.Instance[1].Row[SOFTBUTTON_ROW],
        (button - Controller.Instance[1].Row[SOFTBUTTON_ROW].FirstID) + (channel-1) * 16);
}

// process button inputs from the control surface
void LibMain::ProcessButton(uint8_t channel, uint8_t button, uint8_t value)  // processes a midi button press
{
    uint8_t x, songnumber;
    std::string labelwidget;

    if (value == 127)
    {
        if (channel > 0) ProcessSoftButton(channel, button, value); // softbuttons are on channels 1-5
        else
        {
            if (button == Controller.Instance[1].CommandButtons[FADERS_BANK_UP])  // next Fader bank
            {
                if (Controller.Instance[1].Row[FADER_ROW].IncrementBank())
                {
                    SyncBankIDs(FADER_ROW);
                }
                // DisplayFaders(Controller.Instance[1].Row[FADER_ROW]);
            }
            else if (button == Controller.Instance[1].CommandButtons[FADERS_BANK_DOWN])  // prior Fader bank
            {
                if (Controller.Instance[1].Row[FADER_ROW].DecrementBank())
                {
                    SyncBankIDs(FADER_ROW);
                }
            }
            else if (button == Controller.Instance[1].CommandButtons[KNOBS_BANK_UP])  // next Knob bank
            {
                if (Controller.Instance[1].Row[KNOB_ROW].IncrementBank())
                {
                    SyncBankIDs(KNOB_ROW);
                }
            }
            else if (button == Controller.Instance[1].CommandButtons[KNOBS_BANK_DOWN])  // prior Knob bank
            {
                if (Controller.Instance[1].Row[KNOB_ROW].DecrementBank())
                {
                    SyncBankIDs(KNOB_ROW);
                }
            }
            else if (button == Controller.Instance[1].CommandButtons[SONGS_BANK_UP])   // next song/rack/softbutton bank
            {
				if (Controller.Instance[1].ShowRacksSongs) // disable this for now - putting songs/rack on U1 always
                {
                    if (inSetlistMode() == 1) { Controller.Instance[1].FirstShownSong += Controller.Instance[1].ShowSongCount; }
                    else { Controller.Instance[1].FirstShownRack += Controller.Instance[1].ShowRackCount; }
                    DisplayRow(Controller.Instance[1].Row[Controller.Instance[1].RackRow], false);
                }
                else
                {
                    if (Controller.Instance[1].Row[SOFTBUTTON_ROW].IncrementBank())
                    {
                        SyncBankIDs(SOFTBUTTON_ROW);  // this will call DisplayRow() itself, so don't need to call again
                    }
                }
            }
            else if (button == Controller.Instance[1].CommandButtons[SONGS_BANK_DOWN])  // prior Song/rack/softbutton bank
            {
				if (Controller.Instance[1].ShowRacksSongs) // disable this for now - putting songs/rack on U1 always
                {
                    if (inSetlistMode() == 1) { Controller.Instance[1].FirstShownSong -= Controller.Instance[1].ShowSongCount; }
                    else { Controller.Instance[1].FirstShownRack -= Controller.Instance[1].ShowRackCount; }
                    DisplayRow(Controller.Instance[1].Row[Controller.Instance[1].RackRow], false);
                }
                else
                {
                    if (Controller.Instance[1].Row[SOFTBUTTON_ROW].DecrementBank())
                    {
                        SyncBankIDs(SOFTBUTTON_ROW);  // this will call DisplayRow() itself, so don't need to call again
                    }
                }
            }
            else if (button == Controller.Instance[1].CommandButtons[SETLIST_TOGGLE])  // Toggle between in and out of Setlist mode
            {
                Controller.Instance[1].reportWidgetChanges = false;
                inSetlistMode() ? switchToPanelView() : switchToSetlistView();
                Controller.Instance[1].reportWidgetChanges = Controller.Instance[1].reportWidgetMode;
            }
            else if (button == SID_TRANSPORT_PLAY) // 
            {
                setPlayheadState(true);
            }
            else if (button == SID_TRANSPORT_STOP) // 
            {
                setPlayheadState(false);
            }
            else if (button == SID_FADERBANK_FLIP) // toggle between songs/racks and softbuttons - disabling for now
            {
                Controller.Instance[1].ShowRacksSongs = !Controller.Instance[1].ShowRacksSongs;
                DisplayButton(SID_FADERBANK_FLIP, Controller.Instance[1].ShowRacksSongs ? 127 : 0);
                DisplayRow(Controller.Instance[1].Row[Controller.Instance[1].RackRow], true);
            }

            // it's not one of the special buttons, so cycle through our button rows until we find the one it's in
            // softbuttons are handled differently because they can be in song/rack select mode
            else for (x = 0; x < Controller.Instance[1].ButtonRows; x++)
            {
                if ((button >= Controller.Instance[1].Row[x].FirstID) && (button < Controller.Instance[1].Row[x].LastID) )
                {
                    if (Controller.Instance[1].Row[x].Type == SOFTBUTTON_TYPE)
                    {
                        ProcessSoftButton(channel, button, value);
                    }
                    else if (Controller.Instance[1].Row[x].BankValid()) // make sure ActiveBank is a valid bank to avoid exceptions
                    {
                        ToggleButton(Controller.Instance[1].Row[x], button - Controller.Instance[1].Row[x].FirstID);
                        // ShowButton(Controller.Instance[1].Row[x], button - Controller.Instance[1].Row[x].FirstID);
                    }
                }
            }
        }
    }
}


// button is position within the active bank on the given Row
void LibMain::ToggleButton(SurfaceRow Row, uint8_t button)
{
    std::string widgetname;

    widgetname = Row.WidgetPrefix + "_" + Row.BankIDs[Row.ActiveBank] + "_" + std::to_string(button);
    setWidgetValue(widgetname, (getWidgetValue(widgetname) == 0.0) ? (double)1.0 : (double)0.0);
}


void LibMain::ProcessKnob(uint8_t column, uint8_t value)  // processes a midi message for a knob turn (0-7)
{
    std::string widgetname, pwidgetname, caption;
    int resolution = 200;
    double newValue = 0;

    if (Controller.Instance[1].Row[KNOB_ROW].BankValid()) {

        widgetname = KNOB_PREFIX + (std::string)"_" + Controller.Instance[1].Row[KNOB_ROW].BankIDs[Controller.Instance[1].Row[KNOB_ROW].ActiveBank] + "_" + std::to_string(column);
        if (widgetExists(widgetname) == true)  // if the widget doesn't exist we ignore the input
        {
            if (column < 8) // if it's < 8 it's a real knob
            {
                pwidgetname = KNOB_PREFIX + (std::string)"p_" + Controller.Instance[1].Row[KNOB_ROW].BankIDs[Controller.Instance[1].Row[KNOB_ROW].ActiveBank] + "_" + std::to_string(column);
                if (widgetExists(pwidgetname))  // if there's a mc_kp_1_5 type caption, process first field as resolution (integer)
                {
                    caption = getWidgetCaption(pwidgetname);
                    std::vector< std::string> name_segments = ParseWidgetName(caption, '_');
                    (name_segments.size() > 1) ? resolution = (int)std::stoi("0" + name_segments[1]) : resolution = 200;  // default to 200
                }
                newValue = getWidgetValue(widgetname);
                if (value < 0x10) {  // small numbers are turns in the clockwise direction
                    // newValue = newValue + 0.005 * value;
                    newValue = newValue + static_cast<double>(value) / static_cast<double>(resolution);
                    if (newValue > 1) { newValue = 1; }
                }
                else if (value > 0x40) {  // above 0x40 is a counter-clockwise move
                    // newValue = newValue - 0.005 * (value - 0x40);
                    newValue = newValue - static_cast<double>(value - 0x40) / static_cast<double>(resolution);
                    if (newValue < 0) { newValue = 0; }
                }
                setWidgetValue(widgetname, newValue);  // Move the widget, and the OnWidgetChange callback will move the display
            }
            else
            {
                // if it's above column 7 then something weird is going on and we ignore it
            }
        }
    }
}

void LibMain::ProcessWheel(uint8_t value)  // processes a midi message for jog wheel
{
    std::string widgetname, caption;
    int resolution = 2;

    auto newValue = getBPM();
    if (value < 0x10) {  // small numbers are turns in the clockwise direction
        newValue = newValue + static_cast<double>(value) / static_cast<double>(resolution);
        if (newValue > 640) { newValue = 640; }
    }
    else if (value > 0x40) {  // above 0x40 is a counter-clockwise move
        newValue = newValue - static_cast<double>(value - 0x40) / static_cast<double>(resolution);
        if (newValue < 20) { newValue = 20; }
    }
    setBPM(newValue);
}

// processes a midi message for a fader
void LibMain::ProcessFader(uint8_t channel, uint8_t data1, uint8_t data2)  
{
    std::string widgetname, caption;
    double newValue = 0;

    channel = channel & 0x0F;
//    widgetname = FADER_PREFIX + (std::string) "_" + std::to_string(Controller.Instance[1].ActiveFaderBank) + "_" + std::to_string(channel);
    if (Controller.Instance[1].Row[FADER_ROW].BankValid()) {
        widgetname = FADER_PREFIX + (std::string)"_" + Controller.Instance[1].Row[FADER_ROW].BankIDs[Controller.Instance[1].Row[FADER_ROW].ActiveBank] + "_" + std::to_string(channel);

        // scriptLog(widgetname, 1);
        if (channel < 9) // if it's < 9 it's a real fader
        {
            newValue = ((double)data2 * 128 + (double)data1) / 16383;
            setWidgetValue(widgetname, newValue);  // Move the widget, and the OnWidgetChange callback will move the MCU
            widgetname = FADER_PREFIX + (std::string)"_" + std::to_string(channel);
            if (widgetExists(widgetname)) { setWidgetValue(widgetname, newValue); }
        }
        else
        {
            // if it's above channel 9 then something weird is going on and we ignore it
        }
    }
}


bool LibMain::IsKnob(const uint8_t* data, int length)  // Is midi event from a knob turn?
{
    return(data[0] == 0xB0 && data[1] >= KNOB_0 && data[1] <= KNOB_7);
}

bool LibMain::IsWheel(const uint8_t* data, int length)  // Is midi event from a jow wheel turn?
{
    return(data[0] == 0xB0 && data[1] == JOGWHEEL);
}


bool LibMain::IsFader(const uint8_t* data, int length)  // Is midi event from a fader?
{
    return((data[0] & 0xE0) == 0xE0);
}

bool LibMain::IsButton(const uint8_t* data, int length)   // Is midi event from a button?
{
    return( ((data[0] & 0xf0) == 0x90) || ((data[0] & 0xf0) == 0x80) ); // any note on/off, any channel
}
