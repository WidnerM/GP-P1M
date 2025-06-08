#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>
#include "DelayCallback.h"
#include "LibMain.h"

LibMain* lib = new LibMain(nullptr); // mrw - this is needed as a reference to LibMain

void RefreshTimer::timerCallback()  // what gets done on the periodic timer
{
    if (lib == nullptr)
        return;
    lib->ScheduledSoftsend();
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

