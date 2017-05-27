#ifndef CSettingsDialog_H
#define CSettingsDialog_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/validate.h>
#include <wx/valnum.h>

#include "CSettingsModel.h"
#include "Definitions.h"

/**
 * Class representing a modal window for setting the detector.
 */
class CSettingsDialog : public wxDialog
{
	// variables
public:
	/* none */
private:
	wxStaticText * bandLowLabel;
	wxStaticText * bandHighLabel;
	wxStaticText * k1Label;
	wxStaticText * k2Label;
	wxStaticText * k3Label;
	wxStaticText * winsizeLabel;
	wxStaticText * noverlapLabel;
	wxStaticText * bufferingLabel;
	wxStaticText * mhfLabel;
	wxStaticText * decLabel;
	wxStaticText * dtLabel;
	wxStaticText * putLabel;
	
	wxTextCtrl * bandLowEntry;
	wxTextCtrl * bandHighEntry;
	wxTextCtrl * k1Entry;
	wxTextCtrl * k2Entry;
	wxTextCtrl * k3Entry;
	wxTextCtrl * winsizeEntry;
	wxTextCtrl * noverlapEntry;
	wxTextCtrl * bufferingEntry;
	wxTextCtrl * mhfEntry;
	wxTextCtrl * decEntry;
	wxTextCtrl * dtEntry;
	wxTextCtrl * putEntry;
	
	wxButton * buttonLoadDefault;
	wxButton * buttonLoadSaved;
	wxButton * buttonUse;
	wxButton * buttonSave;
	
	wxStaticText * messageLabel;
	
	CSettingsModel 	  * m_model;
	bool			    m_isInput;
	DETECTOR_SETTINGS * m_settings;
	
	// methods
public:
	/**
	 * A constructor.
	 * @param parent an pointer to the parent frame or another dialog box.
	 * @param title the title of the dialog.
	 */
	CSettingsDialog(wxWindow * parent, const wxString& title);
	
	/**
	 * A virtual destructor.
	 */
	virtual ~CSettingsDialog();
	
	/**
	 * Return pointer to structure DETECTOR_SETTINGS.
	 *	@return pointer to structure DETECTOR_SETTINGS
	 */
	inline DETECTOR_SETTINGS * GetSettings() const
	{ return m_settings; }
	
	/**
	 * Set sample rate.
	 */
	void SetFS(const int& fs);

	/**
	 * Verify if the values in DETECTOR_SETTINGS are correct.
	 */
	bool ValidateValues();
private:
	/**
	 *	Is called on quit the settings modal dialog.
	 */
	void OnQuit(wxCommandEvent& WXUNUSED(event));

	/**
	 * Set the changet values.
	 * @return true if all settings are valid, else return false
	 */
	void OnUse(wxCommandEvent& WXUNUSED(event));

	/**
	 * Close settings modal dialog.
	 */
	void OnClose(wxCloseEvent& event);

	/**
	 * Save changed values of settings to the XML.
	 */
	void OnSave(wxCommandEvent& event);

	/**
	 * Load defautl settings from XML.
	 */
	void OnLoadDefault(wxCommandEvent& WXUNUSED(event));
	
	/**
	 * Load saved settings from XML.
	 */
	void OnLoadSaved(wxCommandEvent& WXUNUSED(event));
	
	/**
	 * Verify if the inputs from entries are correct.
	 * If aren't marks the bad entries and show warning.
	 */
	bool ValidateInput();

	/**
	 * Refresch controls.
	 */
	void RefreshControls();
	
	wxDECLARE_EVENT_TABLE();
};

#endif
