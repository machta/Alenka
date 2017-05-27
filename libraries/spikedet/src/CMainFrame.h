#ifndef CMainFrame_H
#define	CMainFrame_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/progdlg.h>
#include <wx/busyinfo.h>

#include <wx/grid.h>
#include <wx/headerctrl.h>
#include <wx/generic/gridctrl.h>
#include <wx/generic/grideditors.h>
#include <wx/stopwatch.h>

#include <cwchar>

#include "CInputModel.h"
#include "CException.h"
#include "CSettingsDialog.h"
#include "CSpikeDetector.h"
#include "CResultsModel.h"
#include "Definitions.h"

/**
 * Class representing GUI Main frame - window of application.
 * Implementation of the application controls and view the results.
 */
class CMainFrame: public wxFrame
{
	// methods
public:
	/**
	 * A constructor.
	 * @param title the caption to be displayed on the frame's title bar.
	 * @param pos the window position. The value wxDefaultPosition indicates a default position, chosen by either the windowing system or wxWidgets, depending on platform.
	 * @param size the window size. The value wxDefaultSize indicates a default size, chosen by either the windowing system or wxWidgets, depending on platform.
	 * @param model the class the class allows access to input data.
	 */
	CMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, CInputModel * model);

private:
	/**
	 * Runs analysis of input data from EDF file. After analysis are shown results if isn't any error in analysis.
	 * This method is called on click to Run.
	 */
	void OnRun(wxCommandEvent& WXUNUSED(event));

	/**
	 * Show open file dialog for selection of input EDF file.
	 * After selecting the file are loaded basic information and it is possible to run the analysis.
	 */
	void OnOpen(wxCommandEvent& WXUNUSED(event));

	/**
	 * Show save file dialog to save results to file.
	 */
	void OnSaveResults(wxCommandEvent& WXUNUSED(event));

	/**
	 * Show open file dialog to load results from file.
	 */
	void OnLoadResults(wxCommandEvent& WXUNUSED(event));

	/**
	 * Define operations when clicking to exit application. Invokes wxCloseEvent.
	 */
	void OnExit(wxCommandEvent& event);

	/**
	 * Define operations when closing application.
	 */
	void OnClose(wxCloseEvent& event);

	/**
	 * Show information about application.
	 */
	void OnAbout(wxCommandEvent& WXUNUSED(event));

	/**
	 * Show settings dialog. @see #CSettingsDialog
	 */
	void OnOptions(wxCommandEvent& WXUNUSED(event));
	
	/**
	 * Captures events from CSpikeDetector. Shows progress analysis and reacts to end of the analysis.
	 */
	void OnDetectorEvent(wxThreadEvent& event);

	/**
	 * Plot detector outputs to data grids.
	 */
	void plotOutputs();

	/**
	 * Clear results, close file, etc.
	 */
	void clear();
	
	wxDECLARE_EVENT_TABLE();

	// variables
public:
	/* none */
	
private:
	/// toolbar in window
	wxToolBar* 		  m_mainToolbar;
	/// File menu in menu bar
	wxMenu* 		  m_menuFile;
	/// Detector menu in menu bar
	wxMenu*			  m_menuDetector;
	/// wxGrid for show out
	wxGrid* 		  m_gridOut;
	/// wxFrid for show discharges
	wxGrid*           m_gridDisch;
	
	/// settings dialog
	CSettingsDialog*  m_settingsDialog;
	/// object for loading input data
	CInputModel* 	  m_model;

	/// detector object for analysis
	CSpikeDetector*   m_detector;
	
	// the progress dialog which we show while worker thread is running
	/// progress dialog of analysis
	wxProgressDialog* m_dlgProgress;
	
	// was the worker thread cancelled by user?
	bool 			  m_cancelled;
	wxCriticalSection m_csCancelled;        // protects m_cancelled

	// detector outputs
	/// results of analysis out
	CDetectorOutput*  m_out;
	/// eesults of analysis disch
	CDischarges* 	  m_disch;
	/// flag whether the results are saved, if are any
	bool			  m_isResultsSaved;

	/// stop watch for time measuring
	wxStopWatch 	  m_sw;
};

#endif
