#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>
#include "DelayCallback.h"
#include "LibMain.h"

LibMain* lib = new LibMain(nullptr); // mrw - this is needed as a reference to LibMain

void RefreshTimer::timerCallback()
{
    if (lib == nullptr)
        return;
    lib->ScheduledSoftsend();
}

/* void RefreshTimer::setDirty(bool value)
{
    surfacelink.SoftbuttonArray.Dirty = value;
}

RefreshTimer::RefreshTimer()
{

}

RefreshTimer::~RefreshTimer()
{

}
*/

class SoftbuttonTimer : public juce::Timer
{
public:
	virtual void timerCallback() override;

    P1SoftbuttonArray ButtonArray;

    void SetButtons(P1Softbutton buttons[80])
    {
        for (auto x=0; x<80; x++)
            ButtonArray.Buttons[x] = buttons[x];
    }

    void SetDirty()
    {
        ButtonArray.Dirty = true;
    }

    void SetClean()
    {
        ButtonArray.Dirty = false;
    }

};

void SoftbuttonTimer::timerCallback()
{
    // LibMain::scriptLog("Callback", 0);
    // this->stopTimer();
}

void LibMain::lambdaDemo(std::string text)
{
    std::string device;
    gigperformer::sdk::GPMidiMessage midimessage;

    juce::Timer::callAfterDelay(2000,
        // Lambda starts here
        [this, device, midimessage, text] // List of variables that the body has access to
        {                 // Body of the lambda
            scriptLog(text, 1);
            // LibMain::SendSoftbuttons(1,80);
        } // End of the body
            ); // This belongs to `juce::Timer:callAfterDelay(...)`
}

