// implements juce delay calls

#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_events/juce_events.h>
#include <regex>
#include "MCU_Classes.h"

class RefreshTimer : public juce::Timer
{
public:
    // RefreshTimer();
    // ~RefreshTimer() override;

    void timerCallback() override;
};

