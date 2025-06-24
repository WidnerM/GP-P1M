#pragma once

#include <format>
#include "General_Utils.h"
#include "MCU_Constructs.h"
#include "MCU_Buttons.h"

// Define identifiers GP user must use to name their widgets
#define THIS_PREFIX "mc"
#define RECORD_PREFIX "mc_rec"
#define SOLO_PREFIX "mc_solo"
#define MUTE_PREFIX "mc_mute"
#define SELECT_PREFIX "mc_sel"
#define FADER_PREFIX "mc_f"
#define KNOB_PREFIX "mc_k"
#define KNOB_BUTTON_PREFIX "mc_push"
#define FUNCTION_PREFIX "mc_fn"
#define VIEW_PREFIX "mc_view"
#define SOFTBUTTON_PREFIX "mc_sb"
#define ROW_PREFIX_ARRAY {RECORD_PREFIX, SOLO_PREFIX, MUTE_PREFIX, SELECT_PREFIX, SOFTBUTTON_PREFIX, KNOB_BUTTON_PREFIX, FADER_PREFIX, KNOB_PREFIX}
// #define ROW_PREFIX_ARRAY {RECORD_PREFIX, SOLO_PREFIX, MUTE_PREFIX, SELECT_PREFIX, FUNCTION_PREFIX, VIEW_PREFIX, KNOB_BUTTON_PREFIX, FADER_PREFIX, KNOB_PREFIX}

#define RECORD_ROW 0
#define SOLO_ROW 1
#define MUTE_ROW 2
#define SELECT_ROW 3
#define FUNCTION_ROW 4
#define SOFTBUTTON_ROW 4
#define KNOB_BUTTON_ROW 5
#define FADER_ROW 6
#define KNOB_ROW 7

#define RECORD_TAG "rec"
#define SOLO_TAG "solo"
#define MUTE_TAG "mute"
#define SELECT_TAG "sel"
#define FADER_TAG "f"
#define KNOB_TAG "k"
#define KNOB_BUTTON_TAG "push"
#define FUNCTION_TAG "fn"
#define VIEW_TAG "view"
#define SOFTBUTTON_TAG "sb"

#define TAG_ARRAY {RECORD_TAG, SOLO_TAG, MUTE_TAG, SELECT_TAG, SOFTBUTTON_TAG, KNOB_BUTTON_TAG, FADER_TAG, KNOB_TAG}
#define ROW_LABEL_ARRAY { "Rec", "Solo", "Mute", "Sel", "Softbutton", "KPush" , "Fader", "Knob"}

#define BUTTON_TYPE "Button"
#define FADER_TYPE "Fader"
#define KNOB_TYPE "Knob"
#define SOFTBUTTON_TYPE "Softbutton"
#define KNOB_BUTTON_TYPE "KButton"
#define ROW_TYPE_ARRAY {BUTTON_TYPE, BUTTON_TYPE, BUTTON_TYPE, BUTTON_TYPE, SOFTBUTTON_TYPE, KNOB_BUTTON_TYPE, FADER_TYPE, KNOB_TYPE}


#define SHOW_SONGS 0
#define SHOW_SONGPARTS 1
#define SHOW_RACKSPACES 2
#define SHOW_VARIATIONS 3
#define SHOW_ALLVARIATIONS 4
#define SHOW_BUTTONS 5
#define SHOW_KNOBS 6
#define SHOW_FADERS 7
#define SHOW_KNOB_BUTTONS 8
#define SHOW_ASSIGNED 9
// #define SHOW_ARRAY { SHOW_BUTTONS, SHOW_BUTTONS, SHOW_SONGS, SHOW_BUTTONS, SHOW_BUTTONS, SHOW_KNOB_BUTTONS, SHOW_FADERS, SHOW_KNOBS}


#define SONGSRACKS_SELECT 0
#define KNOBS_SELECT 1
#define FADERS_SELECT 2
#define SONGS_BANK_UP 3
#define SONGS_BANK_DOWN 4
#define KNOBS_BANK_UP 5
#define KNOBS_BANK_DOWN 6
#define FADERS_BANK_UP 7
#define FADERS_BANK_DOWN 8
#define SETLIST_TOGGLE 9

// the button order here is { show_faders, show_knobs, show_racks/songs, fader_bank_down, fader_bank_up, knob bank up, knob bank down, song/rack list fwd, song/rack back, toggle setlist/rackspace }
#define DEFAULT_COMMAND_BUTTONS {SID_ASSIGNMENT_TRACK, SID_ASSIGNMENT_PAN, SID_ASSIGNMENT_EQ, SID_FADERBANK_GLOBAL, SID_FADERBANK_FLIP, SID_FADERBANK_NEXT_BANK, SID_FADERBANK_PREV_BANK, SID_FADERBANK_NEXT_CH, SID_FADERBANK_PREV_CH, SID_TRANSPORT_RECORD }
#define ICON_MPLUS_COMMAND_BUTTONS {SID_AUTOMATION_WRITE, SID_AUTOMATION_READ, SID_MARKER, SID_TRANSPORT_FAST_FORWARD, SID_TRANSPORT_REWIND, SID_FADERBANK_NEXT_BANK, SID_FADERBANK_PREV_BANK, SID_FADERBANK_NEXT_CH, SID_FADERBANK_PREV_CH, SID_TRANSPORT_RECORD }
#define ICON_P1M_COMMAND_BUTTONS {0xff, 0xff, 0xff,                                        SID_FADERBANK_NEXT_BANK, SID_FADERBANK_PREV_BANK, SID_FADERBANK_NEXT_CH, SID_FADERBANK_PREV_CH, SID_TRANSPORT_FAST_FORWARD, SID_TRANSPORT_REWIND, SID_TRANSPORT_RECORD }
#define XTOUCH_COMMAND_BUTTONS {SID_ASSIGNMENT_TRACK, SID_ASSIGNMENT_PAN, SID_ASSIGNMENT_EQ, SID_TRANSPORT_FAST_FORWARD, SID_TRANSPORT_REWIND, SID_FADERBANK_NEXT_BANK, SID_FADERBANK_PREV_BANK, SID_FADERBANK_NEXT_CH, SID_FADERBANK_PREV_CH, SID_TRANSPORT_RECORD }

// The SurfaceWidget class is the conduit used for translating GP widget information and changes to control surface displayscontroller_widgettype_bankname_position
// These are always temporary.  We do not store the state of widgets or control surface items in the extension.
class SurfaceWidget
{
public:
	std::string SurfacePrefix;  // typical widget label structure as "controller_widgettype_bankname_position" eg. sl_k_pan_3
	std::string WidgetID;
	std::string BankID;
	uint8_t Column = 255;

	bool Validated = false;  // user may create widgets not on the surface or in banks that don't yet exist - this simplifies detection and crash avoidance
	bool IsSurfaceItemWidget = false;  // indicates the widget maps to a physical surface control
	bool IsRowParameterWidget = false;  // things like names or resolutions on GP widgets that don't correspond to physical surface controls

	int RowNumber = -1;  // If this widget is associated with a Row, used primarily to determine if it's the active bank or not

	double Value = 0.0;
	std::string TextValue = "";
	std::string Caption = "";
	int RgbLitColor = 0, RgbDimColor = 0;

	int resolution = 1000;

	bool IsFader();

	bool IsKnob();

};

class SurfaceRow
{
public:
	std::vector<std::string> BankIDs;
	std::string WidgetPrefix; // the hardware identifier (e.g. "mc") + the WidgetID (e.g. "f") together to simplify a bunch of widget checks (e.g., mc_f)
	std::string WidgetID;  // the widget ID that signifies this row, e.g., f, k, b, etc.
	std::string RowLabel = "";  // a friendly name for what the row is, e.g., "Faders", "Knobs", etc.  Used to indicate on the display what bank of controls the displayed labels are for

	std::string Type = BUTTON_TYPE;

	int ActiveBank = -1;
	uint8_t Showing = SHOW_ASSIGNED;
	uint8_t Columns = 8;
	uint8_t MidiCommand = 0x90;
	uint8_t FirstID = 0;

	bool BankValid()
	{
		return (ActiveBank >= 0 && ActiveBank < BankIDs.size());
	}

	std::string ActiveBankID()
	{
		if (BankValid()) return BankIDs[ActiveBank];
		else return "";
	}

	bool addBank(std::string bank) {
		int index, result;
		bool found = false;

		for (index = 0; index < BankIDs.size(); index++)
		{
			result = BankIDs[index].compare(bank);
			if (result == 0)
			{
				found = true;
				break;
			}
			else if (result > 0) { break; }
		}
		if (!found) {
			BankIDs.insert(BankIDs.begin() + index, bank);
		}
		return (! found);
	}

	int NextBank()
	{
		if (ActiveBank < 0)
		{
			return -1;
		}
		else if (ActiveBank < BankIDs.size() - 1)
		{
			return ActiveBank + 1;
		}
		else
		{
			return 0;
		}
	}

	int PreviousBank()
	{
		if (ActiveBank > 0) { return ActiveBank - 1; }
		else { return BankIDs.size() - 1; }
	}

	bool IncrementBank()
	{
		if (NextBank() >= 0)
		{
			ActiveBank = NextBank();
			return true;
		}
		else return false;
	}

	bool DecrementBank()
	{
		if (PreviousBank() >= 0)
		{
			ActiveBank = PreviousBank();
			return true;
		}
		else return false;
	}

	bool makeActiveBank(std::string bank) {
		int index, result;
		bool found = false;

		for (index = 0; index < BankIDs.size(); index++)
		{
			result = BankIDs[index].compare(bank);
			if (result == 0)
			{
				found = true;
				ActiveBank = index;
				break;
			}
			else if (result > 0) { break; }
		}
		return found;
	}
};

// iCon P1-M/Nano/V1-M softbutton structure - string format byte, 16 character text
class P1Softbutton
{
public:
	uint8_t Format = 7;
	std::string Label = "                ";

	// basic code to format the two-line softbutton display
	// could improve it by deciding where to line break if > 8 characters
	bool formatSoftbuttonText(std::string label);

};

class P1SoftButtonEvent
{
public:
	uint8_t event;
	uint8_t repeat;
	uint8_t channel;
	uint8_t note;
	uint8_t pushval;
	uint8_t releaseval;
	uint8_t unused1;
	uint8_t unused2;
};

class P1SoftbuttonArray
{
public:
	P1Softbutton Buttons[80];
	P1Softbutton LastButtons[80];
	bool Dirty = true;

	// set an array element text and forat it appropriately
	bool set(uint8_t position, std::string text);
	bool set(uint8_t position, P1Softbutton button);
};

class SrfcClass
{
public:
	SurfaceRow Row[8];
	uint8_t ButtonRows = 6;
	std::string IgnoreWidget = "";
	bool reportWidgetChanges = true; // for MCUs indicates whether we dedicate a display line to showing value of any touched widget
	bool reportWidgetMode = false; // for P1-M we're turning that entirely off but keeping the MCU code generally intact
	bool P1MType = true;
	P1SoftbuttonArray SoftbuttonArray;
	std::string InPort = "";
	std::string OutPort = "";
	std::string PortFourOut = "";
	std::string PortFourIn = "";

	
	int P1MColorbars[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	// uint8_t TextDisplay = SHOW_FADERS; // eliminated this for P1-M
	int FirstShownSong = 0;
	int FirstShownRack = 0;
	uint8_t CommandButtons[10] = ICON_P1M_COMMAND_BUTTONS;
	uint8_t ButtonLayout = 0;
	uint8_t RackRow = SOFTBUTTON_ROW; // always putting this on the softbutton row for P1
	bool ShowRacksSongs = true; // whether to show racks/songs on softbuttons page 1
	// uint8_t VarRow = 255;
	uint8_t ShowSongCount = 8;
	uint8_t ShowSongpartCount = 8;
	uint8_t ShowRackCount = 8;
	uint8_t ShowVariationCount = 8;


	bool Initialize()
	{
		// defines up the structure of the control surface
		// controls exist in "rows", such as knobs, faders, record button, solo button, etc.
		// we define 9 rows of these, which map to the widget name structure (e.g., mc_solo_bank_position)
		int x;
		std::string row_prefixes[] = ROW_PREFIX_ARRAY;
		std::string row_tags[] = TAG_ARRAY;
		std::string row_types[] = ROW_TYPE_ARRAY;
		std::string row_labels[] = ROW_LABEL_ARRAY;
		// uint8_t show_array[] = SHOW_ARRAY;
		uint8_t midi_commands[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0xB0, 0xE0, 0xB0 };
		int row_columns[] = { 8, 8, 8, 8, 16, 8, 9, 8 };

		uint8_t first_midi[] = { SID_RECORD_ARM_BASE, SID_SOLO_BASE, SID_MUTE_BASE, SID_SELECT_BASE, SID_FUNCTION_BASE, SID_VPOD_PUSH_BASE, FADER_0, KNOB_0};

		// basic Surface structure initializations
		for (x = 0; x < std::size(Row); x++)
		{
			Row[x].WidgetPrefix = row_prefixes[x];
			Row[x].WidgetID = row_tags[x];
			Row[x].RowLabel = row_labels[x];
			Row[x].Type = row_types[x];
			Row[x].Columns = row_columns[x];
			Row[x].FirstID = first_midi[x];
			Row[x].MidiCommand = midi_commands[x];
			Row[x].Showing = SHOW_ASSIGNED; // SHOW_ASSIGNED means show normal row.  Can alternately be set to show racks, songs, etc.
		}
		// Row[3].Showing = SHOW_SONGS;
		// Row[2].Showing = SHOW_SONGPARTS;

		RackRow = 4; // for P1-M the racks are always on the softbutton row

		// P1-M Softbutton array initialization - label can be up to 14 characters (2 lines of 7)
		for (x = 0; x < 80; x++)
		{
			SoftbuttonArray.Buttons[x].Label = "Soft " + std::to_string(x + 1);
			SoftbuttonArray.Buttons[x].Format = 1;
		}

		return true;
	}

	bool addSurfaceBank(std::string type, std::string bank) {
		int index;

		for (index = 0; index < std::size(Row); index++) {
			if (type == Row[index].WidgetID)
			{
				return Row[index].addBank(bank);
			}
		}
		return false;  // if we get here, we didn't find an appropriate Row in the array
	}

	void SwapRows(int a, int b) {
		SurfaceRow temp;

		temp = Row[a];
		Row[a].WidgetID = Row[b].WidgetID;
		Row[a].WidgetPrefix = Row[b].WidgetPrefix;
		Row[b].WidgetID = temp.WidgetID;
		Row[b].WidgetPrefix = temp.WidgetPrefix;

	}

	int IdentifySurfaceRow(std::string rowidentifier) {
		int x;

		for (x = 0; x < std::size(Row); x++)
		{
			if (Row[x].WidgetID == rowidentifier)
				return x;
		}
		return -1;
	}

	bool RowValid(int rownum)
	{
		return (rownum >= 0 && rownum < std::size(Row));
	}

};

class SrfcArray
{
public:
	SrfcClass Instance[3];

	void Initialize()
	{
		Instance[0].Initialize();
		Instance[1].Initialize();
		Instance[2].Initialize();
	}
};