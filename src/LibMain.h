#pragma once

#include "gigperformer/sdk/GPMidiMessages.h"
#include "gigperformer/sdk/GPUtils.h"
#include "gigperformer/sdk/GigPerformerAPI.h"
#include "gigperformer/sdk/types.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <queue>
#include <math.h>
#include <regex>
#include "MCU_Classes.h"
#include "General_Utils.h"
#include "DelayCallback.h"



// define an XML string describing your product
const std::string XMLProductDescription =   
     // Replace with your information            
    "<Library>" 
    "<Product Name=\"iCon P1-M Extension\" Version=\"0.9\" BuildDate=\"9/1/2024\"></Product> "
    "<Description>Control integration for iCon P1-M protocol devices.</Description>"
    "<ImagePath>/Users/Downloads/nothing.jpg</ImagePath>"
    "</Library>";


// External code runs in the LibMain class, inherited from GigPerformerAPI

class LibMain : public gigperformer::sdk::GigPerformerAPI
{

protected:
    int GetPanelCount() override;
    std::string GetPanelName(int index) override;
    std::string GetPanelXML(int index) override;

    // These are for creating menu items in Gig Performer that can be used to trigger external functions provided by the extension developer
    int GetMenuCount() override;
    std::string GetMenuName(int index) override;
    void InvokeMenu(int itemIndex) override;

public:
    // These must be here but no need to do anything unless you want extra behavior
    LibMain(LibraryHandle handle) : GigPerformerAPI(handle)  {}
    virtual ~LibMain() {}

    //  Global declarations and initializations
    //  [Global to the LibMain class, that is]
    // static SrfcClass Surface;
    static SrfcArray Controller;
    RefreshTimer refreshTimer;

    std::queue<std::string> SysexQueue; // because the P1-M gets overloaded easily on port 4

    void lambdaDemo(std::string message);

    std::vector<std::string> MidiOut = { MCU_OUT };
    std::vector<std::string> MidiIn = { MCU_IN };
    std::string P1Port4Out = "MIDIOUT4 (iCON P1-M V1.10 )";
    std::string P1Port4In = "MIDIIN4 (iCON P1-M V1.10 )";


    // from LibMain.cpp.  We make these to simplify sending the same midi message to all destinations if we have multiple destinations
    void sendMidiMessage(std::string MidiMessage);
    void sendMidiMessage(gigperformer::sdk::GPMidiMessage MidiMessage);
    void sendMidiMessage(const uint8_t* MidiMessage, int length);
	void sendMidiMessage(uint8_t daw, std::string MidiMessage);
    void sendMidiMessage(uint8_t daw, gigperformer::sdk::GPMidiMessage MidiMessage);
    void sendMidiMessage(uint8_t daw, const uint8_t* MidiMessage, int length);
//    void sendHexMidiMessage(std::string MidiMessage);

    void SetSurfaceLayout(uint8_t config);
    void sendPort4Message(std::string MidiMessage);


    // from Display.cpp - functions for displaying things on the control surface (fader positions, LCD, button lights, etc.)
    void InitializeMCU();
    void ClearMCUDisplay(uint8_t row = 0);
    void CleanMCU();
    void Notify(std::string text);
    void DisplayText(uint8_t column, uint8_t row, std::string text, uint8_t maxlength);
    void DisplayBankInfo(std::string text);
    // void DisplayModeButtons();
    void DisplayButton(uint8_t button, uint8_t value);
    void SetRowAssignments();
    void SyncBankIDs(uint8_t syncrow);

    void DisplayWidgetValue(SurfaceRow Row, uint8_t column, double Value);
    // void DisplayTopLeft(uint8_t column, const std::string caption);
    void DisplayControlLabel(uint8_t column, const std::string caption);
    uint8_t KnobDotValue(uint8_t column);
    uint8_t KnobRingValue(uint8_t column);
    void ClearRow(SurfaceRow Row);
    std::string GPColorToSLColorHex(int color);

    SurfaceWidget PopulateWidget(std::string widgetname);
    

    // from P1Routines.cpp
    void ScheduledSoftsend();
    std::string SendSoftbuttonCodes(uint8_t first, uint8_t last);
	bool QueueMidi(std::string sysex); // Queues a sysex message to be sent later, to avoid overloading the P1-M port 4
	bool SendQueuedMidi(); // pops one message and sends it to the P1-M port 4

    // void InitializeSoftbuttons();
    // void SendSoftbuttons(uint8_t first, uint8_t last);
    // void DisplayP1MText(uint8_t column, uint8_t row, std::string text, uint8_t maxlength);
    // void DisplayP1MColorbars();
    // P1Softbutton formatSoftbuttonText(std::string label);


    // from Inputs.cpp
    void ProcessButton(uint8_t button, uint8_t value);
    void ToggleButton(SurfaceRow Row, uint8_t button);


    void ProcessKnob(uint8_t column, uint8_t value);
    void ProcessWheel(uint8_t value);
    void ProcessFader(uint8_t data0, uint8_t data1, uint8_t data2);

    bool IsKnob(const uint8_t* data, int length);
    bool IsWheel(const uint8_t* data, int length);
    bool IsFader(const uint8_t* data, int length);
    bool IsButton(const uint8_t* data, int length);


    // from Songlist.cpp file, functions having to do with song management
    void DisplaySongs(SurfaceRow Row, bool forcetocurrent); // Shows the song list along the bottom bar, allowing song selection with bottom buttons in SONG_MODE
    void DisplaySongParts(SurfaceRow Row, int newIndex);  // Shows the song parts on specified Row
    void DisplayRacks(SurfaceRow Row, bool forcetocurrent);
    void DisplayVariations(SurfaceRow Row, int current);
    void DisplayRow(SurfaceRow Row, bool forcetocurrent);
    void DisplayRow(SurfaceRow Row);
    void DisplayFaders(SurfaceRow Row);  // Shows the active knob bank (as stored in Controller.Instance[1].ActiveKnobBank)
    void DisplayButtonRow(SurfaceRow Row, uint8_t firstbutton, uint8_t number);
    void DisplaySoftbuttons(SurfaceRow Row);

       
    // General routines we define before the varoius callbacks
    bool SetMidiInOutDevices() {
        bool foundin = false, foundout = false;
        std::string name;
        std::vector <std::string> validInPorts = {};
        std::vector <std::string> validOutPorts = {};
        std::regex rgx;

        for (int i = 0; i < getMidiInDeviceCount(); i++)
        {
            name = getMidiInDeviceName(i);
            for (int j = 0; j < MidiIn.size(); j++) {
                if (std::regex_match(name, std::regex(MidiIn[j]))) {
                    listenForMidi(getMidiInDeviceName(i), 1);
                    foundin = true;
                    validInPorts.push_back(name);
                    scriptLog("P1M:  Using midi in " + name, 0);
                }
            }
            if (regex_match(name, std::regex("MIDIIN4.*P1-M.*")))
            {
                P1Port4In = name;
                Controller.Instance[1].PortFourIn = name;
                listenForMidi(Controller.Instance[1].PortFourIn, 1);
                scriptLog("P1M:  Using port 4 " + name, 0);
            }
        }
        
        for (int i = 0; i < getMidiOutDeviceCount(); i++)
        {
            name = getMidiOutDeviceName(i);
            // scriptLog("Evaluating midi out " + name, 1);
            for (int j = 0; j < MidiOut.size(); j++) {
                if (regex_match(name, std::regex(MidiOut[j])) ) { 
                    foundout = true; 
                    validOutPorts.push_back(name);
                    if (regex_match(name, std::regex(P1M_REGEX)))
                    {
                        Controller.Instance[1].P1MType = true;
						Controller.Instance[1].OutPort = name;
                    }
                    scriptLog("P1M:  Using midi out " + name, 0);
                }
            }
            if (regex_match(name, std::regex("MIDIOUT4.*P1-M.*")))
            {
                P1Port4Out = name;
                Controller.Instance[1].PortFourOut = name;
                scriptLog("P1M:  Using port 4 " + name, 0);
            }
        }
        MidiOut = validOutPorts;
        // scriptLog(foundout ? EXTENSION_IDENTIFIER + (std::string)" using midi out " + MidiOut : EXTENSION_IDENTIFIER + (std::string)"COULD NOT FIND midi out " + MidiOut, 1);
        return (foundin && foundout);
    }


    // Here's where all the callbacks are.
    // For organization, most of the real routines are split out into other files.
    void OnStatusChanged(GPStatusType status) override
    {
        uint8_t row, column, i;
        std::string widgetname, setting;

        if (status == GPStatus_GigFinishedLoading)
        {
            // scriptLog("MC: Gig loaded.", 1);
            registerCallback("OnSongChanged");
            registerCallback("OnSongPartChanged");
            registerCallback("OnModeChanged");
            registerCallback("OnWidgetValueChanged");
            registerCallback("OnGlobalPlayStateChanged");
            registerCallback("OnRackspaceActivated");
            registerCallback("OnVariationChanged");
            registerCallback("OnMidiIn");
            registerCallback("OnWidgetStateChanged");
            registerCallback("OnTempoChanged");

            // Look for names for Midi in and out, preferably in global rack
            if (widgetExists(MIDI_IN_WIDGETNAME)) { MidiIn = ParseWidgetName(getWidgetCaption(MIDI_IN_WIDGETNAME), ','); }
            if (widgetExists(MIDI_OUT_WIDGETNAME)) { MidiOut = ParseWidgetName(getWidgetCaption(MIDI_OUT_WIDGETNAME), ','); }

            SetMidiInOutDevices();

            InitializeMCU();

            if (widgetExists(LAYOUT_WIDGETNAME)) {
                column = (uint8_t)round(1 / std::max(getWidgetValue(LAYOUT_WIDGETNAME), 1.0 / 127)) - 1;
                SetSurfaceLayout(column);
            }

            OnRackspaceActivated();  // will move faders, light buttons, set display.
            OnModeChanged(inSetlistMode());  // will set indicators and buttons for whether we're in setlist or rackspace mode

            // LightButtonsForRow(Controller.Instance[1].Row[0], 0, 8);
            // DisplayModeButtons();
        }
    }

    // On closing GP, clear the MCU display and button LEDs
    void OnClose() override
    {
        int row, column;
        std::string widgetname;

        // scriptLog("MC: Shutting down.", 1);
        InitializeMCU();  // clear buttons, display, and VCUs

        // Clear all the OSC displays so we don't have stray junk left behind or on next Gig load
        for (row = 0; row < std::size(Controller.Instance[1].Row); row++)
        {
            for (column = 0; column < Controller.Instance[1].Row[row].Columns; column++)
            {
                widgetname = Controller.Instance[1].Row[row].WidgetPrefix + "_active_" + std::to_string(column);
                if (widgetExists(widgetname))
                {
                    setWidgetCaption(widgetname, "");
                    if (getWidgetValue(widgetname) == 0) { setWidgetValue(widgetname, 0.1); }
                    setWidgetValue(widgetname, 0);
                }
            }
        }
    }

    // Light or unlight the play state button
    void OnGlobalPlayStateChanged(double playing) override
    {
        // consoleLog(std::string("playing = ") + (playing ? " true" : "false"));
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeNoteOnMessage(SID_TRANSPORT_PLAY, (playing == 1) ? BUTTON_LIT : BUTTON_OFF, 0));
    }

    void OnWidgetStateChanged(const std::string& widgetname, int newstate) override
    {
        if (newstate == 0) {
            // scriptLog("Widget state " + widgetname + std::to_string(newstate), 1);
            // add code here to clear controls or zero faders when the widget it was attached to is deleted
        }
    }

    // if the widget caption changes and it's the active bank then update the bank "active" widgets so it's sent out over OSC
    void OnWidgetCaptionChanged(const std::string& widgetName, const std::string& newCaption) override
    {
        uint8_t column = 255;
        uint8_t row;
        double Value;

        std::string widget_prefix, control_type, control_bank, control_number, control_color, setwidget;

        // scriptLog("MC: Callback for Caption " + widgetName, 1);
        std::vector< std::string> name_segments = ParseWidgetName(widgetName, '_');

        if (name_segments.size() >= 4)
        {
            widget_prefix = name_segments[0];
            control_type = name_segments[1];
            control_bank = name_segments[2];
            control_number = name_segments[3];
            if (name_segments.size() > 4) {
                control_color = name_segments[4];
            }

            column = std::stoi("0" + control_number);
            if (std::to_string(column) != control_number) { column = 255; } // flag it as 255 if it's an invalid id

            for (row = 0; row < std::size(Controller.Instance[1].Row); row++)  // cycle through each Row of control
            {
                if (control_type == Controller.Instance[1].Row[row].WidgetID && Controller.Instance[1].Row[row].Showing == SHOW_ASSIGNED && column <= Controller.Instance[1].Row[row].Columns && Controller.Instance[1].Row[row].BankValid())
                {
                    //scriptLog("MC: Callback for " + widgetName, 1);
                    if (control_bank.compare(Controller.Instance[1].Row[row].BankIDs[Controller.Instance[1].Row[row].ActiveBank]) == 0)  // if it's the active bank we update the OSC label
                    {

                        Value = getWidgetValue(widgetName);
                        setWidgetCaption(Controller.Instance[1].Row[row].WidgetPrefix + "_active_" + control_number, newCaption);
                        // Gig Performer doesn't send OSC updates for just the caption.  Need to move the value to make it send an OSC update.
                        Value > 0.9 ? setWidgetValue(widgetName, Value - 0.05) : setWidgetValue(widgetName, Value + 0.05);
                        setWidgetValue(widgetName, Value);
                    }
                }
            }
        }
        else if (widgetName == SHOWRACKS_WIDGETNAME)
        {
            Controller.Instance[1].ShowRackCount = std::min(std::stoi("0" + getWidgetCaption(widgetName)), 16);
            Controller.Instance[1].ShowVariationCount = 16 - Controller.Instance[1].ShowRackCount;
        }
        else if (widgetName == SHOWSONGS_WIDGETNAME)
        {
            Controller.Instance[1].ShowSongCount = std::min(std::stoi("0" + getWidgetCaption(widgetName)), 16);
            Controller.Instance[1].ShowSongpartCount = 16 - Controller.Instance[1].ShowSongCount;
        }

    }

    void OnTempoChanged(double newValue) override
    {
        if (Controller.Instance[1].reportWidgetChanges == true )  // show changes on upper left display if not showing racks/songs
        {
            // DisplayTopLeft(0, "BPM: " + std::to_string(newValue));
            // maybe display on the LED time code display
        }
    }

    void OnWidgetValueChanged(const std::string & widgetName, double newValue) override
    {
        // scriptLog("MC: Widget changed: " + widgetName, 1);
        if (Controller.Instance[1].IgnoreWidget != "") {
            scriptLog("MC: Ignoring widget: " + widgetName, 0);
            Controller.Instance[1].IgnoreWidget = "";
        }
        else
        {
            if (widgetName == LAYOUT_WIDGETNAME) {  // if it's a widget that changes the control layout
                uint8_t column = (uint8_t) round(1 / std::max(newValue, 1.0 / 127)) - 1;
                if (column != Controller.Instance[1].ButtonLayout)
                {
                    SetSurfaceLayout(column);
                }
            }
            else if (widgetName == RACKROW_WIDGETNAME || widgetName == VARROW_WIDGETNAME)
            {
                // SetRowAssignments(); // for P1-M we're put racks and variations / songs and songparts on Fn & View buttons right now
            }
            else
            {
                SurfaceWidget widget = PopulateWidget(widgetName);

                if (widget.IsSurfaceItemWidget)
                {
                    if (widget.BankID == Controller.Instance[1].Row[widget.RowNumber].ActiveBankID() && 
                        (Controller.Instance[1].Row[widget.RowNumber].Type != SOFTBUTTON_TYPE || !Controller.Instance[1].ShowRacksSongs) )
                    {
//                        setWidgetValue(Controller.Instance[1].Row[widget.RowNumber].WidgetPrefix + "_active_" + widget.Column, newValue);
                        setWidgetValue( widget.SurfacePrefix + "_" + widget.WidgetID + "_active_" + std::to_string(widget.Column), newValue);
                        DisplayWidgetValue(Controller.Instance[1].Row[widget.RowNumber], widget.Column, newValue);
                        if (widget.WidgetID == FADER_TAG) { DisplayText(widget.Column, 2, widget.TextValue, 7); }
                        if (widget.WidgetID == KNOB_TAG) { DisplayText(widget.Column, 1, widget.TextValue, 7); }
                        //if (Controller.Instance[1].reportWidgetChanges == true && Controller.Instance[1].TextDisplay != SHOW_SONGS && !Controller.Instance[1].P1MType)  // show changes on upper left display if not showing racks/songs
                        //{
                        //    DisplayTopLeft(widget.Column, widget.Caption + " : " + widget.TextValue);
                        //}
                    }
                }
            }
        } // IgnoreWidget check
    } 

    // A midi device was added or removed
    void OnMidiDeviceListChanged(std::vector< std::string> & inputs, std::vector< std::string> & outputs) override
    {
        SetMidiInOutDevices();
    }    


    // process data coming from the MCU (or a virtual MCU)
    bool OnMidiIn(const std::string & deviceName, const uint8_t* data, int length) override
    {
        const uint8_t ACK[] = { 0x9a, 0x26, 0x01 }; // note on ch 10 38 vel 1
        char str[1024];
        char extra[5];
        int x;

        if (deviceName == Controller.Instance[1].PortFourIn)
        {
            sprintf(str, "Port4 In: ");
            for (x = 0; x < length; x++)
            {
                sprintf(extra, " %0x", data[x]);
                strcat(str, extra);
            }
            scriptLog(str, 0);
            //if (memcmp(data, ACK, sizeof(data)) == 0)
            //{ 
            //    SendQueuedMidi(); // if we get the ACK from the P1-M it's ok to send the next block
            //}
        }
        else
        {
            // scriptLog("MC: In " + deviceName, 1);
            if (IsKnob(data, length)) {
                ProcessKnob(data[1] - KNOB_0, data[2]);  // pass the knob position (0-7) and the value (relative pos is usually 1 or 127)
            }
            else if (IsButton(data, length)) {
                ProcessButton(data[1], data[2]);  // it's a button press
            }
            else if (IsFader(data, length)) {
                ProcessFader(data[0], data[1], data[2]);
            }
            else if (IsWheel(data, length)) {
                ProcessWheel(data[2]);
            }
            else
            {
                sprintf(str, "MCU:  Unexpected midi event: ");
                for (x = 0; x < length; x++)
                {
                    sprintf(extra, " %0x", data[x]);
                    strcat(str, extra);
                }
                scriptLog(str, 0);
            }
        }
        return(false);
    }


    // Called when a new gig file has been loaded and when song is changed
    void OnSongChanged(int oldIndex, int newIndex) override
    {
        uint8_t row;
        
        // scriptLog("MC: Song changed to number " + std::to_string(newIndex), 1);
        // scriptLog("MC: getCurrentSongIndex = " + std::to_string(getCurrentSongIndex()), 1);
        
        for (row = 0; row < Controller.Instance[1].ButtonRows; row++)
        {
            if ((Controller.Instance[1].Row[row].Showing == SHOW_SONGS) && (getCurrentSongIndex() >= 0))
            {
                DisplaySongs(Controller.Instance[1].Row[row], true);
            }
            //if ((Controller.Instance[1].Row[row].Showing == SHOW_SONGPARTS) && (getCurrentSongIndex() >= 0))
            //{
            //    DisplaySongParts(Controller.Instance[1].Row[row], getCurrentSongpartIndex());
            //}
        }
    } 

    // Called when a song part is changed
    void OnSongPartChanged(int oldIndex, int newIndex) override
    {
        uint8_t row;

        // scriptLog("MC: Song part changed to number " + std::to_string(newIndex), 1);
        // scriptLog("MC: getCurrentSongIndex = " + std::to_string(getCurrentSongIndex()), 1);

        /* for (row = 0; row < Controller.Instance[1].ButtonRows; row++)
        {
            if ((Controller.Instance[1].Row[row].Showing == SHOW_SONGPARTS) && (getCurrentSongIndex() >= 0))
            {
                DisplaySongParts(Controller.Instance[1].Row[row], newIndex);
            }
        } */
        DisplayRow(Controller.Instance[1].Row[Controller.Instance[1].RackRow]);
    }

    // Called when entering song mode
    void OnModeChanged(int mode) override
    {
        uint8_t row;

        // scriptLog("MC: Mode changed to " + std::to_string(mode), 1);
        // scriptLog("MC: getCurrentSongIndex = " + std::to_string(getCurrentSongIndex()), 1);

        // DisplayTopLeft(0, "");
        
        if (Controller.Instance[1].RackRow < Controller.Instance[1].ButtonRows)
        {
            Controller.Instance[1].Row[Controller.Instance[1].RackRow].Showing = (mode == 1) ? SHOW_SONGS : SHOW_RACKSPACES;
            DisplayRow(Controller.Instance[1].Row[Controller.Instance[1].RackRow], true);
        }

        //if (Controller.Instance[1].VarRow < Controller.Instance[1].ButtonRows)
        //{
        //    Controller.Instance[1].Row[Controller.Instance[1].VarRow].Showing = (mode == 1) ? SHOW_SONGPARTS : SHOW_VARIATIONS;
        //    DisplayRow(Controller.Instance[1].Row[Controller.Instance[1].VarRow], true);
        //}

        // Light or turn off the SID_MIXER button to indicate if we're in Setlist mode (mode==1) or rackspace mode
        sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeNoteOnMessage(Controller.Instance[1].CommandButtons[SETLIST_TOGGLE], (mode == 1) ? BUTTON_LIT : BUTTON_OFF, 0));
    }

    // examine a vector of widgets and structure the Surface structure to reflect widgets in the rackspace or global rackspace
    void buildSurfaceModel(std::vector <std::string> widgetlist)
    {
        std::string widgetname, prefix, type, bank, column;

        for (auto index = widgetlist.begin(); index != widgetlist.end(); ++index)
        {
            widgetname = *index;
            widgetname = widgetname.substr(widgetname.find(":") + 1);

            std::vector< std::string> name_segments = ParseWidgetName(widgetname, '_');

            // scriptLog("bS sees widget " + widgetname, 1);

            // SHOWRACKS_WIDGETNAME caption sets how many Rackspaces to show on softbuttons (rest will show variations)
            if (widgetname == SHOWRACKS_WIDGETNAME)
            {
                Controller.Instance[1].ShowRackCount = std::min(std::stoi("0" + getWidgetCaption(widgetname)),16);
                Controller.Instance[1].ShowVariationCount = 16 - Controller.Instance[1].ShowRackCount;
            }
            // SHOWSONGS_WIDGETNAME caption sets how many Songs to show on softbuttons (rest will show Songparts)
            else if (widgetname == SHOWSONGS_WIDGETNAME)
            {
                Controller.Instance[1].ShowSongCount = std::min(std::stoi("0" + getWidgetCaption(widgetname)), 16);
                Controller.Instance[1].ShowSongpartCount = 16 - Controller.Instance[1].ShowSongCount;
            }
            else if (widgetname == LAYOUT_WIDGETNAME)
            { 
                listenForWidget(LAYOUT_WIDGETNAME, true);
            }
            else if (widgetname == RACKROW_WIDGETNAME)
            {
                listenForWidget(RACKROW_WIDGETNAME, true);
            }
            else if (widgetname == VARROW_WIDGETNAME)
            {
                listenForWidget(VARROW_WIDGETNAME, true);
            }
            else if (name_segments.size() >= 4)
            {
                prefix = name_segments[0];
                type = name_segments[1];
                bank = name_segments[2];
                column = name_segments[3];

                // loop this three times for prefic mc1 mc2 mc3 (or just mc in place of mc1)
                // and instead of Controller.Instance[1].addSurfaceBank use SurfaceArray[x].addSurfaceBank
                // if it's a widget we're interested in, add a bank for it if it doesn't already exist, and listen for it
                if (prefix == THIS_PREFIX && bank != "active" )  // we don't listen for "active" widgets, which are generally just for linking to an OSC display
                {
                    if (Controller.Instance[1].addSurfaceBank(type, bank) == true) {  /* scriptLog("bS added bank for " + widgetname, 1); */ }
                    if (column != "b") { listenForWidget(widgetname, true); }
                }
            }
        }
    }

    /// examines a Row of widgets and determines which bank should be active and sets things accordingly
    /// this is required because after Rackspace or Variation switching we can end up in states where multiple banks or no banks are flagged as "active" by the 'b' widgets
    void setActiveBank(int row)
    {
        std::string widgetname;
        int index;

        Controller.Instance[1].Row[row].ActiveBank = -1;

        if (Controller.Instance[1].Row[row].BankIDs.empty())
        {
            Controller.Instance[1].Row[row].ActiveBank = -1;
        }
        else {
            for (index = 0; index < Controller.Instance[1].Row[row].BankIDs.size(); ++index)
            {
                widgetname = Controller.Instance[1].Row[row].WidgetPrefix + (std::string)"_" + Controller.Instance[1].Row[row].BankIDs[index] + (std::string)"_i";
                // scriptLog("sAFB sees " + widgetname + (std::string)" as " + std::to_string(getWidgetValue(widgetname)), 1);
                if (widgetExists(widgetname))
                {
                    if ((getWidgetValue(widgetname) > 0.99) && (Controller.Instance[1].Row[row].ActiveBank == -1)) {
                        Controller.Instance[1].Row[row].ActiveBank = index;
                        // scriptLog("setActiveBank set active to " + std::to_string(index) + " " + Controller.Instance[1].Row[row].BankIDs[index], 1);
                    }
                    else if (index != Controller.Instance[1].Row[row].ActiveBank) { setWidgetValue(widgetname, 0.3); }
                }
            }
            if (Controller.Instance[1].Row[row].ActiveBank == -1) { Controller.Instance[1].Row[row].ActiveBank = 0; } // if there are no bank indicator widgets, set ActiveBank to first bank
        }
    }

    // Called when rackspace changed
    void OnRackspaceActivated() override
    {
        uint8_t row, column, bank;
        std::string widgetname, setting;
        std::vector<std::string> widgetlist, globalwidgetlist;

        // scriptLog("MC: Rackspace Changed to " + getRackspaceName(getCurrentRackspaceIndex()),1);
        //Controller.Instance[1].reportWidgetChanges = false;
        //if (Controller.Instance[1].TextDisplay != SHOW_SONGS) {   
        //    ClearMCUDisplay();
        //    ClearMCUDisplay(2);
        //}

        // Clear the BankIDs and active bank data from the prior rackspace's widget set
        for (row = 0; row < std::size(Controller.Instance[1].Row); row++)
        {
            Controller.Instance[1].Row[row].ActiveBank = -1;
            Controller.Instance[1].Row[row].BankIDs.clear();
        }

        // get local and global widget list, combine them, analyze them to build the surface model they define
        getWidgetList(globalwidgetlist, true);
        getWidgetList(widgetlist, false);
        widgetlist.insert(widgetlist.end(), globalwidgetlist.begin(), globalwidgetlist.end());
        buildSurfaceModel(widgetlist);

        // scriptLog("Done buildSurface", 1);

        // Button row initializations
        for (row = 0; row < Controller.Instance[1].ButtonRows; row++) {
            setActiveBank(row);
            // scriptLog("Set active bank for row " + std::to_string(row) + " to " + std::to_string(Controller.Instance[1].Row[row].ActiveBank), 1);
            if (row == Controller.Instance[1].RackRow) // if we're showing Racks/Songs on this row, just do that
            {
                DisplayRow(Controller.Instance[1].Row[row], true);
            }
            else
            {
                for (column = 0; column < Controller.Instance[1].Row[row].Columns; column++)
                {
                    // if there's no valid bank, or if there is a valid bank but no valid widget for this column, then un-light the button 
                    if (Controller.Instance[1].Row[row].BankValid())
                    {
                        widgetname = THIS_PREFIX + (std::string)"_" + Controller.Instance[1].Row[row].WidgetID + "_" + Controller.Instance[1].Row[row].BankIDs[Controller.Instance[1].Row[row].ActiveBank] + "_" + std::to_string(column);
                        if (widgetExists(widgetname) != true && Controller.Instance[1].Row[row].Showing == SHOW_ASSIGNED) { DisplayWidgetValue(Controller.Instance[1].Row[row], column, BUTTON_OFF); }
                    }
                    else { DisplayWidgetValue(Controller.Instance[1].Row[row], column, BUTTON_OFF); }
                }
                DisplayRow(Controller.Instance[1].Row[row], true);
            }
        }


        // scriptLog("Done button rows", 1);
        // we need to decisively set the active bank because it's possible that upon rackspace or variation change more than one, or zero, are set
        setActiveBank(KNOB_ROW);
        DisplayFaders(Controller.Instance[1].Row[KNOB_ROW]);
        if (!Controller.Instance[1].Row[KNOB_ROW].BankValid()) { ClearMCUDisplay(); }

        // scriptLog("Done Knob row", 1);
        setActiveBank(FADER_ROW);
        DisplayFaders(Controller.Instance[1].Row[FADER_ROW]);
        if (!Controller.Instance[1].Row[FADER_ROW].BankValid()) { ClearMCUDisplay(2); }

        // DisplayModeButtons();
        
        // scriptLog("Finishing OnRackspaceChanged.", 1);

        Controller.Instance[1].reportWidgetChanges = Controller.Instance[1].reportWidgetMode;
    }
    

    // Called when variation changed
    void OnVariationChanged(int oldIndex, int newIndex) override
    {
        int row;

        // scriptLog("MC: Variation Changed to " + std::to_string(newIndex) + getVariationName(getCurrentRackspaceIndex(), getCurrentVariationIndex()), 1);
        Controller.Instance[1].reportWidgetChanges = false;

        // we need to decisively set the active bank because it's possible that upon rackspace or variation change more than one, or zero, are set
        setActiveBank(KNOB_ROW);
        DisplayFaders(Controller.Instance[1].Row[KNOB_ROW]);

        setActiveBank(FADER_ROW);
        DisplayFaders(Controller.Instance[1].Row[FADER_ROW]);
        // scriptLog("Finishing OnVariationChanged.", 1);
        Controller.Instance[1].reportWidgetChanges = Controller.Instance[1].reportWidgetMode;

        // DisplayModeButtons();

        DisplayRow(Controller.Instance[1].Row[Controller.Instance[1].RackRow]);
    }

    void OnOpen() override
    {
        // ExtensionWindow::initialize();
        juce::MessageManager::getInstance()->callAsync([]() { });  // this starts the juce library
        refreshTimer.startTimer(50);
    }

    void Initialization() override
    {
        // Do any basic initializations

        // Register callbacks.
        // We register for a minimal list, then add the rest in OnStatusChanged()

        registerCallback("OnOpen");
        registerCallback("OnStatusChanged");
        registerCallback("OnMidiDeviceListChanged");
        registerCallback("OnClose");

    }


    // Generally don't touch this - simply define the constant 'XMLProductDescription' at the top of this file with
    // an XML description of your product
    std::string GetProductDescription() override // This MUST be defined in your class
    {
        return XMLProductDescription;
    }
    
};

