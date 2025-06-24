#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include "LibMain.h"


// Initialize MCU screen and any essential variables
void LibMain::InitializeMCU()
{
    int x;

    Surface.Initialize();

    Surface.FirstShownSong = 0;

    // clear display
    CleanMCU();
    if (Surface.P1MType) {
        InitializeSoftbuttons();
        SendSoftbuttons(1, 80);
        SendSoftbuttonCodes(1, 80);
    }
}

void LibMain::ClearMCUDisplay(uint8_t row)
{
    // scriptLog("MCU clear display", 1);
    if (row < 2) {
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(MCU_CLEAR_BOT)));
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(MCU_CLEAR_TOP)));
    }
    else {
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(P1M_CLEAR_BOT)));
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(P1M_CLEAR_TOP)));
    }
}

void LibMain::CleanMCU()
{
    int x;

    // clear display
    ClearMCUDisplay(0);
    ClearMCUDisplay(2);

    sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(P1M_COLORBAR_CLEAR)));

    // shut off all leds
    for (x = 0; x <= 0x76; x++) {
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeNoteOnMessage(x, 0, 0));
    }

    // clear the knobs
    for (x = 0; x <= 7; x++) {
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeControlChangeMessage(0x30 + x, 0, 0 ));
    }

    // clear the VU meters on real MCUs
    for (x = 0; x <= 7; x++) {
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(gigperformer::sdk::GPUtils::hex2binaryString(MCU_CLEAR_VU + (std::string)"0" +
        std::to_string(x) + "00 F7")));
    }

}

// Transmits sysex midi data to display a message on the P1-M text display
void LibMain::DisplayText(uint8_t column, uint8_t row, std::string text, uint8_t maxlength)
{
    std::string hexmessage, subtext, binmessage;

    if (row < 2) // row zero and 1 are for knob display, show label row 0, value row 1
    {
        if (column < 8) {
            // Could  handle this better...  Adding blanks to the text to display so we're guaranteed to clear whatever's there, then just use front 'maxlength' chars
            subtext = cleanSysex(text);
            subtext = subtext.substr(0, maxlength) + "                                                                ";
            hexmessage = MCU_TEXT_HDR + gigperformer::sdk::GPUtils::intToHex(row * 0x38 + column * 7) + textToHexString(subtext.substr(0, (maxlength % 7 == 0) ? maxlength : maxlength + 7 - maxlength % 7)) + (std::string)" f7";
            binmessage = gigperformer::sdk::GPUtils::hex2binaryString(hexmessage);
            sendMidiMessage(binmessage);
        }
    }
    else if (row < 4)
    {
        // DisplayP1MText(column, row, text, maxlength); // No longer need this post fw 1.08 for P1-M
        if (column < 8) {
            // Could  handle this better...  Adding blanks to the text to display so we're guaranteed to clear whatever's there, then just use front 'maxlength' chars
            subtext = cleanSysex(text);
            subtext = subtext.substr(0, maxlength) + "                                                                ";
            hexmessage = P1M_TEXT_HDR + gigperformer::sdk::GPUtils::intToHex((row-2) * 0x38 + column * 7) + textToHexString(subtext.substr(0, (maxlength % 7 == 0) ? maxlength : maxlength + 7 - maxlength % 7)) + (std::string)" f7";
            binmessage = gigperformer::sdk::GPUtils::hex2binaryString(hexmessage);
            sendMidiMessage(binmessage);
        }
    }
}

// Displays a string on the upper right of the MCU text display
void LibMain::DisplayBankInfo(std::string text)
{
    std::string hexmessage, subtext, binmessage;

    if (!Surface.P1MType)
    {
        subtext = "                                                                " + cleanSysex(text);
        hexmessage = MCU_TEXT_HDR + (std::string)"1C" + textToHexString(subtext.substr(subtext.length() - 28, subtext.length())) + (std::string)" f7";
        binmessage = gigperformer::sdk::GPUtils::hex2binaryString(hexmessage);
        sendMidiMessage(binmessage);
    }
}

// as of Feb 9, 2025 the P1-M/Nano firmware doesn't do anything with this,
// but we still do it to adhere to MCU protocol
uint8_t LibMain::KnobDotValue(uint8_t column)
{
    uint8_t dotvalue = 0;
    if (Surface.Row[KNOB_BUTTON_ROW].BankValid())
    {
        std::string widgetname = Surface.Row[KNOB_BUTTON_ROW].WidgetPrefix + "_" + Surface.Row[KNOB_BUTTON_ROW].ActiveBankID() +
            "_" + std::to_string(column);
//        scriptLog("Knob Dot Value: " + widgetname + " " + std::to_string(getWidgetValue(widgetname)), true);
        if (widgetExists(widgetname)) dotvalue = (getWidgetValue(widgetname) > 0 ? 64 : 0);
    }
    return dotvalue;
}

// MCU protocol only allows for 12 positions for knobring display
// this converts a widget value into a 1-12 value
uint8_t LibMain::KnobRingValue(uint8_t column)
{
    uint8_t ringvalue = 0;
    if (Surface.Row[KNOB_ROW].BankValid())
    {
        std::string widgetname = Surface.Row[KNOB_ROW].WidgetPrefix + "_" + Surface.Row[KNOB_ROW].ActiveBankID() +
            "_" + std::to_string(column);
        //        scriptLog("Knob Dot Value: " + widgetname + " " + std::to_string(getWidgetValue(widgetname)), true);
        if (widgetExists(widgetname)) ringvalue = (uint8_t) (getWidgetValue(widgetname) * 10.9999 + 1);
    }
    return ringvalue;
}

// Show value of a widget on it's linked control surface physical item
// e.g., moves a fader, lights buttons, shows knob position
// this does not do anything with the text displays
void LibMain::DisplayWidgetValue(SurfaceRow Row, uint8_t column, double value)
{
    uint8_t MidiMessage[3];
    int newvalue;

    MidiMessage[0] = Row.MidiCommand;

    if (Row.Type == BUTTON_TYPE || Row.Type == SOFTBUTTON_TYPE)
    {
        MidiMessage[1] = Row.FirstID + column;
        MidiMessage[2] = (value != 0) ? BUTTON_LIT : BUTTON_OFF;
    }
    else if (Row.Type == FADER_TYPE)
    {
        newvalue = (int)(value * 16383); // convert to 14 bit value
        MidiMessage[0] += (column & 0x0F);  // MCU faders run on "pitch bend" 0xE[channel], with channel being fader 0-7 and master on 8.
        MidiMessage[1] = (uint8_t)(newvalue & 0x70);  // these are always 10 bit as far as I know
        MidiMessage[2] = (uint8_t)(newvalue >> 7 & 0x7f);
    }
    else if (Row.Type == KNOB_TYPE)
    {
        MidiMessage[1] = KNOBRING_0 + (column & 0x0F);
        MidiMessage[2] = (uint8_t)(value * 10.9999) + 1 + KnobDotValue(column);
    }
    else if (Row.Type == KNOB_BUTTON_TYPE)
    {
        MidiMessage[1] = KNOBRING_0 + (column & 0x0F);
        MidiMessage[2] = KnobRingValue(column) + KnobDotValue(column);
    }
    sendMidiMessage(MidiMessage, sizeof(MidiMessage));
}

// light up or turn off a button on the control surface
// this is currently only used for indicating what "control mode" we're in for different MCU clone types
void LibMain::DisplayButton(uint8_t button, uint8_t value)
{
    sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeNoteOnMessage(button, (value == 0) ? BUTTON_OFF : BUTTON_LIT, 0));
}

// light buttons to indicate what display mode we're in - showing faders, knobs, or songs/racks on the LCD display
/* void LibMain::DisplayModeButtons()
{
    sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeNoteOnMessage(Surface.CommandButtons[FADERS_SELECT], (Surface.TextDisplay == SHOW_FADERS) ? BUTTON_LIT : BUTTON_OFF, 0));
    sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeNoteOnMessage(Surface.CommandButtons[KNOBS_SELECT], (Surface.TextDisplay == SHOW_KNOBS) ? BUTTON_LIT : BUTTON_OFF, 0));
    sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeNoteOnMessage(Surface.CommandButtons[SONGSRACKS_SELECT], (Surface.TextDisplay == SHOW_SONGS) ? BUTTON_LIT : BUTTON_OFF, 0));
} */

// Adjust row assignments for where Racks/Songs/Variations/Songparts are shown
void LibMain::SetRowAssignments()
{
    std::string row_tags[] = TAG_ARRAY;
    std::string widgetval;
    uint8_t x;

    bool setlistmode = inSetlistMode();

    for (uint8_t x = 0; x < Surface.ButtonRows; x++)
    {
        Surface.Row[x].Showing = SHOW_ASSIGNED;
    }

    //if (widgetExists(RACKROW_WIDGETNAME)) {
    //    widgetval = getWidgetCaption(RACKROW_WIDGETNAME);
    //    for (x = 0; x < Surface.ButtonRows; x++)
    //    {
    //        if (row_tags[x] == widgetval)
    //        {
    //            Surface.RackRow = x;
    //            Surface.Row[Surface.RackRow].Showing = (setlistmode) ? SHOW_SONGS : SHOW_RACKSPACES;
    //            DisplayRow(Surface.Row[Surface.RackRow], true);
    //            break;
    //        }
    //    }
    //}

    //if (widgetExists(VARROW_WIDGETNAME)) {
    //    widgetval = getWidgetCaption(VARROW_WIDGETNAME);
    //    for (x = 0; x < Surface.ButtonRows; x++)
    //    {
    //        if (row_tags[x] == widgetval)
    //        {
    //            Surface.VarRow = x;
    //            Surface.Row[Surface.VarRow].Showing = (setlistmode) ? SHOW_SONGPARTS : SHOW_VARIATIONS;
    //            DisplayRow(Surface.Row[Surface.VarRow], true);
    //            break;
    //        }
    //    }
    //}
    // scriptLog("RackRow: " + std::to_string(Surface.RackRow), 1);
    // scriptLog("VarRow: " + std::to_string(Surface.VarRow), 1);
}

// For "bank switching" between different fader/knob/button banks, this is for keeping the
// knobs and faders on the same bank_name if the new fader/knob bank shares the same name
void LibMain::SyncBankIDs(uint8_t syncrow)
{
    if (Surface.Row[syncrow].BankValid()) {
        std::string rowname = Surface.Row[syncrow].ActiveBankID();
        for (int x = 0; x < std::size(Surface.Row); x++)
        {
            if (Surface.Row[x].makeActiveBank(rowname)) DisplayRow(Surface.Row[x]);
        }
    }
}

// Display a notification message - should have this notify on the MCU display, but we'd need some kind of timer to refresh the display
void LibMain::Notify(std::string text)
{
    consoleLog(text);
}

/// <summary>
/// Displays text on the top left portion of the MCU display using a MIDI sysex message.  Unsafe chars in the text will be deleted before sending.
/// </summary>
/// <param name="column">unused, deprecated</param>
/// <param name="label">text, automatically trimmed to 28 characters</param>
void LibMain::DisplayTopLeft(uint8_t column, const std::string label)
{
    if (column <= 8 && !Surface.P1MType) { DisplayText(0, 0, label, 28); }
}

void LibMain::DisplayControlLabel(uint8_t column, const std::string label)
{
    if (column <= 7) {
        DisplayText(column, 1, label, 7);
    }
}

// Displays Faders or Knobs on the LCD display
void LibMain::DisplayFaders(SurfaceRow Row)
{
    std::string widgetname, oscwidget, oscname;
    std::string Caption, Label, Extras, TextValue;
    int Color;
    SurfaceWidget widget;
    double Value = 0, oldvalue;
    std::string hexmessage, subtext, binmessage;
    int x;


    if (! Row.BankValid()) // if there are no FaderBanks or ActiveFaderBank is out of range we clear the display
    {
        // if ((Surface.TextDisplay == SHOW_FADERS && Row.WidgetID == "f") || (Surface.TextDisplay == SHOW_KNOBS && Row.WidgetID == "k")) { ClearMCUDisplay(); }
        if (Row.Type == KNOB_TYPE) { ClearMCUDisplay(); }
        else if (Row.Type == FADER_TYPE) { ClearMCUDisplay(2); }
        else for (x = 0; x < Row.Columns; x++)
        {
            DisplayWidgetValue(Row, (uint8_t)x, 0);
            //Surface.SoftbuttonArray.set(x, formatSoftbuttonText("-"));
            Surface.SoftbuttonArray.set(x, "-");
        }
    }
    else
    { // Set the bank indicator widgets, if they exist
        for (x = 0; x < Row.BankIDs.size(); x++)
        {
            widgetname = Row.WidgetPrefix + (std::string)"_" + Row.BankIDs[x] + "_i";
            // scriptLog("DisplayFaders setting bank indicator " + widgetname, 1);
            if (widgetExists(widgetname)) {
                setWidgetValue(widgetname, (x == Row.ActiveBank) ? 1.0 : 0.3); // light up panel for active bank, otherwise turn it off
            }
        }
    }

    // Set the positions and displays for each of the faders using the active bank
    for (x = Row.Columns; x >= 0; x--)
    {
        if (!Row.BankIDs.empty())
        {
            widgetname = Row.WidgetPrefix + (std::string)"_" + Row.BankIDs[Row.ActiveBank] + "_" + std::to_string(x);
            widget = PopulateWidget(widgetname);
            if (widget.IsSurfaceItemWidget)
            {
                Value = widget.Value;
                Label = widget.Caption;
                TextValue = widget.TextValue;
                Color = widget.RgbLitColor;
                if (Row.Type == FADER_TYPE) Surface.P1MColorbars[x] = Color;
            }
            else  // we end up here if the widget doesn't exist, so then we set the whole thing blank
            {
                Label = " ";
                Caption = " ";
                Value = 0;
                // Show = false;
            }

            if (Surface.P1MType) {
                DisplayWidgetValue(Row, (uint8_t)x, Value); // move fader or show knob position
                if (Row.Type == FADER_TYPE || Row.Type == KNOB_TYPE) {
                    DisplayText(x, Row.Type == FADER_TYPE ? 3 : 0, Label, 7);
                    DisplayText(x, Row.Type == FADER_TYPE ? 2 : 1, TextValue, 7);
                }
                else if (Row.Type == SOFTBUTTON_TYPE)
                {
                    // Surface.SoftbuttonArray.set(x, formatSoftbuttonText(Label));
                    Surface.SoftbuttonArray.set(x, Label);
                }
            }

        }
        else {
            Label = " ";
        }
        // if the OSC interface widget exists, set or clear it appropriately
        oscwidget = Row.WidgetPrefix + (std::string)"_active_" + std::to_string(x);

        // scriptLog("MC: Set " + oscwidget + " to " + Label,1);
        if (widgetExists(oscwidget) == true)
        {
            setWidgetCaption(oscwidget, Label);
            // OSC won't send update for Caption only, we need to make sure Value changes.
            if (Value == getWidgetValue(oscwidget)) { Value > 0.9 ? setWidgetValue(oscwidget, Value - 0.05) : setWidgetValue(oscwidget, Value + 0.05); }
            setWidgetValue(oscwidget, Value);
        }
    }
    if (Row.Type == FADER_TYPE) DisplayP1MColorbars();
}

// Displays Faders or Knobs on the LCD display
void LibMain::DisplaySoftbuttons(SurfaceRow Row)
{
    scriptLog("Showing softbuttons", 0);
    DisplayFaders(Row);
}

void LibMain::DisplayButtonRow(SurfaceRow Row, uint8_t firstbutton, uint8_t number)
{
    std::string widgetname, oscwidget;
    std::string Caption, Label, Extras;
    SurfaceWidget widget;
    double Value = 0;
    int x;
    uint8_t upcolor = 0;
    uint8_t downcolor = 0;

    for (x = 0; x < std::size(Row.BankIDs); x++)  // cycle through banks to turn on/off the panel widget indicators for active bank
    {
        widgetname = THIS_PREFIX + (std::string)"_" + Row.WidgetID + "_" + Row.BankIDs[x] + "_i";
        if (widgetExists(widgetname)) {
            setWidgetValue(widgetname, (x == Row.ActiveBank) ? 1 : 0.3); // light up panel for active bank, otherwise turn it off
            if (x == (Row.ActiveBank - 1)) {    // light up the button for "next bank" of one exists
                upcolor = BUTTON_LIT;
            }
            if (x == (Row.ActiveBank + 1))
            {
                downcolor = BUTTON_LIT;
            }
        }
    }

    if (Row.BankValid())
    {
        for (x = firstbutton + number - 1; x >= firstbutton; x--)
        {
            widgetname = Row.WidgetPrefix + "_" + Row.BankIDs[Row.ActiveBank] + "_" + std::to_string(x);
            widget = PopulateWidget(widgetname);

            // if (widget.IsSurfaceItemWidget)
            {
                oscwidget = Row.WidgetPrefix + "_active_" + std::to_string(x);  // same widget name with the bank # set to "active"
                setWidgetCaption(oscwidget, widget.Caption);
                setWidgetValue(oscwidget, 1.0 - widget.Value);  // ensure a widget toggle so caption gets propagated
                setWidgetValue(oscwidget, widget.Value);

                DisplayWidgetValue(Row, x, widget.Value);
            }
        }
    }
    else
    {
        ClearRow(Row);
    }
}

void LibMain::ClearRow(SurfaceRow Row)
{
    std::string oscwidget;

    for (uint8_t x = 0; x < Row.Columns; x++)
    {
        oscwidget = Row.WidgetPrefix + "_active_" + std::to_string(x);  // same widget name with the bank # set to "active"
        setWidgetCaption(oscwidget, "-");
        setWidgetValue(oscwidget, 0.1);  // ensure a widget toggle so caption gets propagated
        setWidgetValue(oscwidget, 0);

        DisplayWidgetValue(Row, x, 0);
    }
}

// This extension expects widget names generally in the format "DevicePrefix_WidgetType_Bank_Column" where the "_" character is used as a delimiter.
// An example widget would be "mc_k_pan_0" referring to MCU knob bank 'pan' column 0.
// Extra parameters for widgets are looked for on a "mc_kp_pan_0" widget in the Caption.
SurfaceWidget LibMain::PopulateWidget(std::string widgetname)
{
    SurfaceWidget widget;
    std::string control_number, pcaption, pwidgetname;

    if (widgetExists(widgetname))
    {
        widget.Value = getWidgetValue(widgetname);

        std::vector<std::string> name_segments = ParseWidgetName(widgetname, '_');

        if (name_segments.size() >= 4)
        {
            widget.SurfacePrefix = name_segments[0];
            widget.WidgetID = name_segments[1];
            widget.BankID = name_segments[2];
            control_number = name_segments[3];

            if (widget.SurfacePrefix == THIS_PREFIX)
            {

                widget.RowNumber = Surface.IdentifySurfaceRow(widget.WidgetID);

                // is it a valid row identifier for this Surface?
                if (widget.RowNumber >= 0)
                {
                    try
                    {
                        widget.IsRowParameterWidget = false;
                        widget.Column = std::stoi(control_number);
                        if (widget.RowNumber >= 0 && widget.RowNumber < std::size(Surface.Row))
                        {
                            if (widget.Column < Surface.Row[widget.RowNumber].Columns)
                            {
                                widget.IsSurfaceItemWidget = true;
                                widget.TextValue = getWidgetTextValue(widgetname);
                                widget.Caption = getWidgetCaption(widgetname);

                                // we've checked for valid Surface prefix, row type, and valid column number for the
                                // row, so it's a valid widget we don't check that the BankID is valid because by
                                // definition a bank is valid if there's a valid widget for it
                                widget.Validated = true;
                                // pwidgetname = widgetname + "_p";
                            }
                        }
                    }
                    catch (...) // catch blow ups, such as stoi() if the widget name doesn't have a number where we
                        // need a number
                    {
                        widget.Column = -1;
                        widget.IsSurfaceItemWidget = false;
                        widget.Validated = false;
                    }

                    // look for extra parameters on a parameter widget if it's a valid surface item widget
                    if (widget.Validated)
                    {

                        // if there is a column speciic _kp_ widget that takes first priority, e.g. sl_kp_bank_0
                        pwidgetname =
                            widget.SurfacePrefix + "_" + widget.WidgetID + "p_" + widget.BankID + "_" + control_number;
                        if (widgetExists(pwidgetname))
                        {
                            pcaption = getWidgetCaption(pwidgetname);
                            if (!pcaption.empty())
                            {
                                std::vector< std::string> name_segments = ParseWidgetName(pcaption, '_');
                                (name_segments.size() >= 1) ? widget.Caption = name_segments[0] : widget.Caption = "";

                                if ((widget.WidgetID == FUNCTION_TAG || widget.WidgetID == VIEW_TAG) && name_segments.size() >= 3)
                                {
                                    if (widget.Value > 0)
                                        widget.TextValue = name_segments[2];
                                    else
                                        widget.TextValue = name_segments[1];
                                }

                                // size_t pos = pcaption.find("_");
                                // widget.Caption = pcaption.substr(0, pos);
                            }
                            widget.RgbLitColor = getWidgetFillColor(pwidgetname);  // for knobs LitColor is the knob color, DimColor is top bar color
                            widget.RgbDimColor = getWidgetOutlineColor(pwidgetname);
                        }
                        else
                        {
                            // in the absense of a widget specific property widget we'll try the bank p widget, eg sl_kp_bank
                            pwidgetname = widget.SurfacePrefix + "_" + widget.WidgetID + "p_" + widget.BankID;
                            if (widgetExists(pwidgetname))
                            {
                                // widget.Caption = getWidgetCaption(pwidgetname);
                                widget.RgbLitColor = getWidgetFillColor(pwidgetname);
                                widget.RgbDimColor = getWidgetOutlineColor(pwidgetname);
                            }
                            else
                            {
                                // if no individual or bank property widget exists we'll try the bank indicator widget
                                pwidgetname = widget.SurfacePrefix + "_" + widget.WidgetID + "_" + widget.BankID + "_i";
                                if (widgetExists(pwidgetname))
                                {
                                    // widget.Caption = getWidgetCaption(pwidgetname);
                                    widget.RgbLitColor = getWidgetFillColor(pwidgetname);
                                    widget.RgbDimColor = getWidgetOutlineColor(pwidgetname);
                                }
                                else
                                {
                                    // if none of them exist we'll use a green color
                                    widget.RgbLitColor = 0x00A000;
                                    widget.RgbDimColor = 0x003000;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        widget.Validated = false;
    }
    return widget;
}

// converts a GP widget color integer to a hex string for SL midi
std::string LibMain::GPColorToSLColorHex(int color)
{
    std::string hexstring;

    hexstring = gigperformer::sdk::GPUtils::intToHex((uint8_t)(color >> 17 & 0x7f)) +
        gigperformer::sdk::GPUtils::intToHex((uint8_t)(color >> 9 & 0x7f)) +
        gigperformer::sdk::GPUtils::intToHex((uint8_t)(color >> 1 & 0x7f));

    return hexstring;
}
