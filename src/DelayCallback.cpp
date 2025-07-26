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
    
    // none of this is used currently. Updating softbuttons on an asynchronous basis is not worth the tradeoffs
    // the initial goal was so that we could update softbutton labels ad-hoc as widgets & captions change, but
    // the P1-M protocol requires so much sysex overhead for one softbutton change that we end up either laggy
	// or with corrupted softbutton displays because we overflow the P1-M buffers
    if (countdown > 0) countdown--;
    else
    {
        countdown = 10; // reset the countdown
        lib->Controller.Instance[1].SoftbuttonArray.Dirty[0] = true; // mark the first line as dirty so we send it out
        lib->Controller.Instance[1].SoftbuttonArray.Dirty[1] = true; // mark the second line as dirty so we send it out
	}
	std::string midimessage = LibMain::Controller.Instance[1].Softsend();
    if (midimessage.length() > 2) { lib->QueueMidi(midimessage); }
	//lib->SendQueuedMidi(); // send any queued sysex messages
}


// This could be used for a delayed action, for example to display or clear a message after a certain time
void LibMain::lambdaDemo(std::string text)
{
    std::string device;
    gigperformer::sdk::GPMidiMessage midimessage;

    juce::Timer::callAfterDelay(2000,
        // Lambda starts here
        [this, device, midimessage, text] // List of variables that the body has access to
        {                 // Body of the lambda
            scriptLog(text, 1);
        } // End of the body
            ); // This belongs to `juce::Timer:callAfterDelay(...)`
}

