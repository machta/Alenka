#ifndef CDefinitions_H
#define CDefinitions_H

/// Definition type of the input signal: float or double
typedef float SIGNALTYPE;

/// Definition bandwidth - upper and lower limits of filtering.
typedef struct bandwidth
{
public:
	/**
	 * A constructor
	 * @param bl lower limit filtering
	 * @param bh upper limit filtering
	 */
	bandwidth(const int& bl, const int& bh)
		: m_bandLow(bl), m_bandHigh(bh)
	{
		/* empty */
	}

	/// Lower limit of filtering.
	int m_bandLow;
	
	/// Upper limit filtering.
	int m_bandHigh;
} BANDWIDTH;


/// Definition IDs of wxWidgets elements
enum
{
	// CMainFrame
	MENU_FILE_Open       = 101,
	MENU_FILE_Load		 = 102,
	MENU_FILE_Save 		 = 103,
	MENU_DETECTOR_Option = 104,
	MENU_DETECTOR_Run    = 105,
	TOOLBAR_Run          = 106,
	TOOLBAR_Open         = 107,
	TOOLBAR_Save         = 108,
	TOOLBAR_Load	     = 109,
	TOOLBAR_Sett         = 110,
	
	// CSettingsDialog
	BUTTON_LoadDefault = 201,
	BUTTON_LoadSaved   = 202,
	BUTTON_Use         = 203,
	BUTTON_Save	       = 204,
	
	// CSpikeDetector
	DETECTOR_EVENT    	  = 301,
	THREAD_START_DETECTOR = 302,

	// CMainFrame
	GRID_Output 	   = 401,
	GRID_Discharges	   = 401,

};

/// Definition types of exceptions
enum EXCEPTION_RELEVANCES
{
	EXC_WARNING  = 0,
	EXC_CRITICAL = 1
};

#endif
