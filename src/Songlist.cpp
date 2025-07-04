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

    // scriptLog("MC: In DisplaySongs, current song = " + std::to_string(current), 1);

    // Set Surface.FirstShownSong correctly for what we're going to show at leftmost position
    if (forcetocurrent == true)
    {
        x = current % Surface.ShowSongCount; // we're going to show in banks of 8, this is the offset
        Surface.FirstShownSong = current - x;
    }
    else
    {
        if (Surface.FirstShownSong >= songcount)
        {
            Surface.FirstShownSong = 0; // firstshown is zero based, count can be 0 only if there are no songs
        }
        if (Surface.FirstShownSong < 0) { Surface.FirstShownSong = songcount - songcount % Surface.ShowSongCount; }
    }
    songindex = Surface.FirstShownSong;

    for (x = 0; x < Surface.ShowSongCount; x++)  // cycle through display positions
    {
        if (songindex >= songcount)  // clear the text if there's no song this high
        {
            TopLine = ("");
            BottomLine = ("");
            songname = "-";   // Better to send something to OSC so widgets don't pull up default labels
        }
        else
        {   
            songname = getSongName(songindex);
            if (songname == "") { songname = "-"; }  // Better to send something to OSC so widgets don't pull up default labels

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

        Surface.SoftbuttonArray.set(x, formatSoftbuttonText(songname));

        // show songs/racks on LCD display if appropriate, alternating top and bottom rows so we can fit 12 letters per song
        /* if (Surface.TextDisplay == SHOW_SONGS) {
            if (inSetlistMode() == 1) { DisplayText(x, x % 2, songname, (x==7) ? 6 : 12); }
        } */

        // Show the song name on the the OSC display
        oscwidget = THIS_PREFIX + (std::string) "_" + Row.WidgetID + "_active_" + std::to_string(x);
        if (widgetExists(oscwidget))
        {
            setWidgetCaption(oscwidget, songname);
            // we force a value change to force OSC to update the Caption.  GP doesn't send an OSC update if WidgetValue stays the same.
            if (selected == getWidgetValue(oscwidget)) { selected > 0.9 ? setWidgetValue(oscwidget, 0.95 ) : setWidgetValue(oscwidget, 0.05); }
            // setWidgetValue(oscwidget, 1.0 - selected);  
            setWidgetValue(oscwidget, selected);
        }
        songindex++;
    }

    if (Surface.ShowSongpartCount) DisplaySongParts(Row, -1);
}


void LibMain::DisplaySongParts(SurfaceRow Row, int current)
{
    int x, songpartcount;
    std::string songpartname, oscwidget;

    current = getCurrentSongpartIndex();
    songpartcount = getSongpartCount(getCurrentSongIndex());

    for (x = 0; x < Surface.ShowSongpartCount; x++)  // cycle through display positions
    {
        if (x >= songpartcount)  // clear the text if there's no song this high
        {
            songpartname = "-";   // Better to send something to OSC so widgets don't pull up default labels
            // SetButtonColor(MCU_LOWEST_BUTTON + x, SLMKIII_BLACK);
        }
        else
        {
            songpartname = getSongpartName(getCurrentSongIndex(), x);
            if (songpartname == "") { songpartname = ""; }
        }

        DisplayWidgetValue(Row, x + Surface.ShowSongCount, x == current ? BUTTON_LIT : BUTTON_OFF);
        Surface.SoftbuttonArray.set(x+Surface.ShowSongCount, formatSoftbuttonText(songpartname));

        // Show the song name on the the OSC display and MCU display if appropriate
        oscwidget = THIS_PREFIX + (std::string) "_" + Row.WidgetID + "_active_" + std::to_string(x);
        if (widgetExists(oscwidget))
        {
            setWidgetCaption(oscwidget, songpartname);
            setWidgetValue(oscwidget, x == current ? 0.0 : 1.0); // we force a toggle to force OSC to update the Caption.  GP doesn't send an OSC update if WidgetValue stays the same.
            setWidgetValue(oscwidget, x == current ? 1.0 : 0.0);
        }
    }
}

void LibMain::DisplayVariations(SurfaceRow Row, int current)
{
    int x, variationcount;
    std::string variationname, oscwidget;


    if (current < 0) { current = getCurrentVariationIndex(); }
    variationcount = getVariationCount(getCurrentRackspaceIndex());

    for (x = 0; x < Surface.ShowVariationCount; x++)  // cycle through display positions
    {
        if (x >= variationcount)  // clear the text if there's no variation this high
        {
            variationname = "-";   // Better to send something to OSC so widgets don't pull up default labels
            // SetButtonColor(MCU_LOWEST_BUTTON + x, SLMKIII_BLACK);
        }
        else
        {
            variationname = getVariationName(getCurrentRackspaceIndex(), x);
            if (variationname == "") { variationname = "[blank]"; }
        }

        DisplayWidgetValue(Row, x+Surface.ShowRackCount, x==current ? BUTTON_LIT : BUTTON_OFF);

        Surface.SoftbuttonArray.set(x+Surface.ShowRackCount, formatSoftbuttonText(variationname));
        // refreshTimer.softbuttonarray.set(x + 8, formatSoftbuttonText(variationname));

        // refreshTimer.softbuttonarray.set(x+8, formatSoftbuttonText(variationname));

        // Show the variation name on the the OSC display and MCU display if appropriate
        oscwidget = THIS_PREFIX + (std::string) "_" + Row.WidgetID + "_active_" + std::to_string(x);
        if (widgetExists(oscwidget))
        {
            setWidgetCaption(oscwidget, variationname);
            setWidgetValue(oscwidget, x == current ? 0.0 : 1.0);  // we force a toggle to force OSC to update the Caption.  GP doesn't send an OSC update if WidgetValue stays the same.
            setWidgetValue(oscwidget, x == current ? 1.0 : 0.0);
        }
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

    // Get Surface.FirstShownRack set correctly for what we're going to show at leftmost position
    if (forcetocurrent == true)
    {
        x = current % Surface.ShowRackCount; // we're going to show in banks of 8, this is the offset
        Surface.FirstShownRack = current - x;
    }
    else
    {
        if (Surface.FirstShownRack >= rackcount)
        {
            Surface.FirstShownRack = 0;  // firstshown is zero based, count can be 0 only if there are no racks
        }
        if (Surface.FirstShownRack < 0) { Surface.FirstShownRack = rackcount - rackcount % 8;
        }
    }
    rackindex = Surface.FirstShownRack;

    /* if (Surface.TextDisplay == SHOW_SONGS)
    {
        if (inSetlistMode() == 0) { DisplayText(0, 1, "R" + std::to_string(Surface.FirstShownRack + 1) + "-" + std::to_string(Surface.FirstShownRack + 8), 7); }
    } */

    for (x = 0; x < Surface.ShowRackCount; x++)  // cycle through display positions
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

        Surface.SoftbuttonArray.set(x,formatSoftbuttonText(rackname));
        // refreshTimer.softbuttonarray.set(x, formatSoftbuttonText(rackname));

        if (selected == 1) { DisplayWidgetValue(Row, x, BUTTON_LIT); }
        else { DisplayWidgetValue(Row, x, BUTTON_OFF); }

        // show songs/racks on LCD display if appropriate, alternating top and bottom rows so we can fit 12 letters per song
        /* if (Surface.TextDisplay == SHOW_SONGS) {
            if (inSetlistMode() == 0) { DisplayText(x, x % 2, rackname, (x == 7) ? 6 : 12); }
        } */

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
    
    if (Surface.ShowVariationCount) DisplayVariations(Row, -1);
}

void LibMain::DisplayRow(SurfaceRow Row)
{
    DisplayRow(Row, false);
}

void LibMain::DisplayRow(SurfaceRow Row, bool forcetocurrent)
{
    if (Row.Type == SOFTBUTTON_TYPE) {
        if (Surface.ShowRacksSongs)
        {
            if (Row.Showing == SHOW_SONGS) { DisplaySongs(Row, forcetocurrent); }
            else { DisplayRacks(Row, forcetocurrent); }
        }
        else { DisplaySoftbuttons(Row); }
        DisplayButton(SID_FADERBANK_FLIP, Surface.ShowRacksSongs ? 127 : 0);
    }
    // else if (Row.Showing == SHOW_SONGPARTS) { DisplaySongParts(Row, forcetocurrent); }
    // else if (Row.Showing == SHOW_VARIATIONS) { DisplayVariations(Row, -1); }
    else if (Row.Type == FADER_TYPE || Row.Type == KNOB_TYPE) { DisplayFaders(Row); }
    else { DisplayButtonRow(Row, 0, 8); }
}
