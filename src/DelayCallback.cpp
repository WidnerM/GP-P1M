#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>
#include "LibMain.h"


using namespace juce;


class myHUDTimer : public Timer
{
public:
	virtual void timerCallback() override;
};
