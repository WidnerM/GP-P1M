#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include "LibMain.h"



//  This displays the Song List along the bottom button area if we're in SONG_MODE
//  Will also display the songs using both rows of the LCD display
void LibMain::DisplaySongs(SurfaceRow Row, bool forcetocurrent)
{
    int x, songindex, current, songcount;
    std::string TopLine = "", BottomLine = "";
    std::string songname, hexmessage, binmessage, oscwidget, oscwidget2;
    double selected = 0;

    // GP sends current song over OSC, which we can display with OSC{/CurrentSongName, -} in the label field

    current = getCurrentSongIndex();
    songcount = getSongCount();

    // Set FirstShownSong correctly for what we're going to show at leftmost position
	Controller.Instance[1].setFirstShownSong(current, songcount, forcetocurrent);

    // scriptLog("MC: In DisplaySongs, current song = " + std::to_string(current) + ", songcount = " + std::to_string(songcount)
    //    + ", firstShownSong = " + std::to_string(Controller.Instance[1].FirstShownSong), 1);

    songindex = Controller.Instance[1].FirstShownSong;

    for (x = 0; x < Controller.Instance[1].ShowSongCount; x++)  // cycle through display positions
    {
        if (songindex >= songcount)  // clear the text if there's no song this high
        {
            TopLine = ("");
            BottomLine = ("");
            songname = "-";   // Better to send something
        }
        else
        {   
            songname = getSongName(songindex);
            if (songname == "") { songname = "un-named"; }  // Better to send something

        }
        if (songindex == current)
        {
            selected = 1;
        }
        else
        {
            selected = 0;
        }

        // light or turn off the buttons on the button row
        if (selected == 1) { DisplayWidgetValue(Row, x, BUTTON_LIT); }
        else { DisplayWidgetValue(Row, x, BUTTON_OFF); }

        Controller.Instance[1].SoftbuttonArray.setLabel(x, songname);

        songindex++;
    }

    if (Controller.Instance[1].ShowSongpartCount) DisplaySongParts(Row, -1);
}


void LibMain::DisplaySongParts(SurfaceRow Row, int current)
{
    int x, songpartcount;
    std::string songpartname, oscwidget;

    current = getCurrentSongpartIndex();
    songpartcount = getSongpartCount(getCurrentSongIndex());

    for (x = 0; x < Controller.Instance[1].ShowSongpartCount; x++)  // cycle through display positions
    {
        if (x >= songpartcount)  // clear the text if there's no song this high
        {
            songpartname = "-";   // Better to send something than leave it blank
        }
        else
        {
            songpartname = getSongpartName(getCurrentSongIndex(), x);
            if (songpartname == "") { songpartname = "null"; } // this should never happen
        }

        DisplayWidgetValue(Row, x + Controller.Instance[1].ShowSongCount, x == current ? BUTTON_LIT : BUTTON_OFF);
        Controller.Instance[1].SoftbuttonArray.setLabel(x+Controller.Instance[1].ShowSongCount, songpartname);

    }
}

void LibMain::DisplayVariations(SurfaceRow Row, int current)
{
    int x, variationcount;
    std::string variationname, oscwidget;


    if (current < 0) { current = getCurrentVariationIndex(); }
    variationcount = getVariationCount(getCurrentRackspaceIndex());

    for (x = 0; x < Controller.Instance[1].ShowVariationCount; x++)  // cycle through display positions
    {
        if (x >= variationcount)  // clear the text if there's no variation this high
        {
            variationname = "-";   // Better to send something to OSC so widgets don't pull up default labels
        }
        else
        {
            variationname = getVariationName(getCurrentRackspaceIndex(), x);
            if (variationname == "") { variationname = "[blank]"; }
        }

        DisplayWidgetValue(Row, x+Controller.Instance[1].ShowRackCount, x==current ? BUTTON_LIT : BUTTON_OFF);

        Controller.Instance[1].SoftbuttonArray.setLabel(x+Controller.Instance[1].ShowRackCount, variationname);

    }
}

//  This displays the Racks on the softbutton row
void LibMain::DisplayRacks(SurfaceRow Row, bool forcetocurrent)
{
    int x, rackindex, current, rackcount;
    std::string TopLine = "", BottomLine = "";
    std::string rackname, hexmessage, binmessage, oscwidget, oscwidget2;
    double selected = 0;

    // GP sends current song over OSC we can display with OSC{/CurrentSongName, -} in the label field

    current = getCurrentRackspaceIndex();
    rackcount = getRackspaceCount();

	Controller.Instance[1].setFirstShownRack(current, rackcount, forcetocurrent);

    rackindex = Controller.Instance[1].FirstShownRack;

    for (x = 0; x < Controller.Instance[1].ShowRackCount; x++)  // cycle through display positions
    {
        if (rackindex >= rackcount)  // clear the text if there's no song this high
        {
            TopLine = ("");
            BottomLine = ("");
            rackname = "-";   // Better to send something to OSC so widgets don't pull up default labels
        }
        else
        {
            rackname = getRackspaceName(rackindex);
            if (rackname == "") { rackname = "-"; }  // Better to send something to OSC so widgets don't pull up default labels

        }
        if (rackindex == current)
        {
            selected = 1;
        }
        else
        {
            selected = 0;
        }

        Controller.Instance[1].SoftbuttonArray.setLabel(x,rackname);
        // refreshTimer.softbuttonarray.set(x, formatSoftbuttonText(rackname));

        if (selected == 1) { DisplayWidgetValue(Row, x, BUTTON_LIT); }
        else { DisplayWidgetValue(Row, x, BUTTON_OFF); }

        // Show the song name on the the OSC display and MCU display if appropriate
        oscwidget = THIS_PREFIX + (std::string) "_" + Row.WidgetID + "_active_" + std::to_string(x);
        if (widgetExists(oscwidget))
        {
            setWidgetCaption(oscwidget, rackname);
            if (selected == getWidgetValue(oscwidget)) { selected > 0.9 ? setWidgetValue(oscwidget, 0.95) : setWidgetValue(oscwidget, 0.05); }
            setWidgetValue(oscwidget, selected);  // There appears to be a race condition in GP where this may be missed if we don't stall it with a redundant write of the caption
        }
        rackindex++;
    }
    
    if (Controller.Instance[1].ShowVariationCount) DisplayVariations(Row, -1);
}

void LibMain::DisplayRow(SurfaceRow Row)
{
    DisplayRow(Row, false);
}

void LibMain::DisplayRow(SurfaceRow Row, bool forcetocurrent)
{
    if (Row.Type == SOFTBUTTON_TYPE) {
        if (Controller.Instance[1].ShowRacksSongs)
        {
            if (Row.Showing == SHOW_SONGS) { DisplaySongs(Row, forcetocurrent); }
            else { DisplayRacks(Row, forcetocurrent); }
        }
        else { DisplaySoftbuttons(Row); }
        DisplayButton(SID_FADERBANK_FLIP, Controller.Instance[1].ShowRacksSongs ? 127 : 0);
        QueueMidi(Controller.Instance[1].Softsend()); // send the required row names  MRWedit
    }
    // else if (Row.Showing == SHOW_SONGPARTS) { DisplaySongParts(Row, forcetocurrent); }
    // else if (Row.Showing == SHOW_VARIATIONS) { DisplayVariations(Row, -1); }
    else if (Row.Type == FADER_TYPE || Row.Type == KNOB_TYPE) { DisplayFaders(Row); }
    else { DisplayButtonRow(Row, 0, 8); }
}
