#include "LibMain.h"


// List of panels
std::vector<std::string> panelNames = { "MCU to OSC" };
std::vector<std::string> relativePanelLocations = { "MCUtoOSC.gppanel" };

// make global variable Surface static so it can be referened in asychronous RefreshTimer calls
// SrfcClass staticSurface;
// SrfcClass LibMain::Surface = staticSurface;
SrfcArray staticController;
SrfcArray LibMain::Controller = staticController;

std::string pathToMe; // This needs to be initialized from the initialization secttion of the LibMain class so it can be used in the standalone functions directly below

int LibMain::GetPanelCount()
{
    return panelNames.size();
}

std::string  LibMain::GetPanelName(int index)
{
    std::string text;
    if (index >= 0 && index < panelNames.size())
        text = panelNames[index];

    return text;
}

// Return panel layout in XML format
std::string  LibMain::GetPanelXML(int index)
{
    std::string text;
    if (index >= 0 && index < panelNames.size())
    {
        // We assume the panels are in the same folder as the library
        // scriptLog("MC: Path to panels: " + getPathToMe() + relativePanelLocations[index], 0);
        gigperformer::sdk::GPUtils::loadTextFile(getPathToMe() + relativePanelLocations[index], text);
    }
    return text;
}



// List of menu items
std::vector<std::string> menuNames = { "Re-initialize extention", "lambdaDemo"};


int LibMain::GetMenuCount()
{
    return menuNames.size();
}

std::string  LibMain::GetMenuName(int index)
{
    std::string text;
    if (index >= 0 && index < menuNames.size())
        text = menuNames[index];

    return text;
}


void LibMain::InvokeMenu(int index)
{
    std::vector <std::string> widgetlist;
    std::string widgetname;

    if (index >= 0 && index < menuNames.size())
    {
        switch (index)
        {
        //case 0:
        //    if (widgetExists(LAYOUT_WIDGETNAME)) { setWidgetValue(LAYOUT_WIDGETNAME, 1.0); }  // Layout # is stored in a widget as 1/x because value is constrained to 0-1.  ie, 0.25 = layout 4
        //    else SetSurfaceLayout(0);
        //    break;
        //case 1:
        //    if (widgetExists(LAYOUT_WIDGETNAME)) { setWidgetValue(LAYOUT_WIDGETNAME, 0.5); }
        //    else SetSurfaceLayout(1);
        //    break;
        //case 2:
        //    if (widgetExists(LAYOUT_WIDGETNAME)) { setWidgetValue(LAYOUT_WIDGETNAME, 0.33333); }
        //    else SetSurfaceLayout(2);
        //    break;
        //case 3:
        //    if (widgetExists(LAYOUT_WIDGETNAME)) { setWidgetValue(LAYOUT_WIDGETNAME, 0.25); }
        //    else SetSurfaceLayout(3);
        //    break;

        case 0:
            // OnStatusChanged(GPStatus_GigFinishedLoading);
            // setWidgetBounds("BoundsWidget", 10, 10, 100, 100);
            Controller.Instance[1].SoftbuttonArray.Initialize();
            // SendSoftbuttons(1, 80);
            SendSoftbuttonCodes(1, 80);
            break;
        case 1:
            scriptLog("calling LambdaDemo", 0);
            lambdaDemo("testmessage");
            scriptLog("Returned after LambdaDemo", 0);
            break;

        default:
            break;
        }
    }
}

void LibMain::sendMidiMessage(std::string MidiMessage) {
    for (int i = 0; i < MidiOut.size(); i++) {
        sendMidiMessageToMidiOutDevice(MidiOut[i], MidiMessage);
    }
}

void LibMain::sendMidiMessage(gigperformer::sdk::GPMidiMessage MidiMessage) {
    for (int i = 0; i < MidiOut.size(); i++) {
        sendMidiMessageToMidiOutDevice(MidiOut[i], MidiMessage);
    }
}

void LibMain::sendMidiMessage(const uint8_t* MidiMessage, int length) {
    for (int i = 0; i < MidiOut.size(); i++) {
        sendMidiMessageToMidiOutDevice(MidiOut[i], MidiMessage, length);
    }
}

// Send a midi message to the out port for the specified daw instance (1-3)
void LibMain::sendMidiMessage(uint8_t daw, std::string MidiMessage) {
        sendMidiMessageToMidiOutDevice(Controller.Instance[daw].OutPort, MidiMessage);
}

void LibMain::sendMidiMessage(uint8_t daw, gigperformer::sdk::GPMidiMessage MidiMessage) {
        sendMidiMessageToMidiOutDevice(Controller.Instance[daw].OutPort, MidiMessage);
}

void LibMain::sendMidiMessage(uint8_t daw, const uint8_t* MidiMessage, int length) {
        sendMidiMessageToMidiOutDevice(Controller.Instance[daw].OutPort, MidiMessage, length);
}

// Takes hex string (e.g., "F0 2A b4"), converts, and sends
/* void LibMain::sendHexMidiMessage(std::string MidiMessage) {
    sendMidiMessage(gigperformer::sdk::GPMidiMessage::makeSysexMessage(MidiMessage);
} */

void LibMain::sendPort4Message(std::string MidiMessage) {
    // sendMidiMessageToMidiOutDevice(P1Port4Out, MidiMessage);
    sendMidiMessageToMidiOutDevice(Controller.Instance[1].PortFourOut, MidiMessage);
}


// MCU compatible surfaces come in different configurations.  We have a configuration variable in the Surface structure so we can define different action keys for different functions
void LibMain::SetSurfaceLayout(uint8_t config) {
    const uint8_t std_commandarray[] = DEFAULT_COMMAND_BUTTONS;
    const uint8_t config_array[4][sizeof(std_commandarray)] = { DEFAULT_COMMAND_BUTTONS, ICON_MPLUS_COMMAND_BUTTONS, XTOUCH_COMMAND_BUTTONS, ICON_P1M_COMMAND_BUTTONS };

    // for now we're always setting to default P1-M layout now. |< >| cycle knob banks, << >> for fader banks, <<8 8>> for softbuttons
    config = 3;

    if (config >= 0 && config <= 3)
    {
        for (auto x = 0; x < sizeof(Controller.Instance[1].CommandButtons); ++x)
        {
            DisplayButton(Controller.Instance[1].CommandButtons[x], BUTTON_OFF);  // turn off all the prior mode indicators
            Controller.Instance[1].CommandButtons[x] = config_array[config][x];
        }
        Controller.Instance[1].ButtonLayout = config;
        // DisplayModeButtons();
    }
}


gigperformer::sdk::GigPerformerAPI* gigperformer::sdk::CreateGPExtension(LibraryHandle handle)
{
    return new LibMain(handle);
}