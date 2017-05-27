#include "CSettingsDialog.h"

#include <iostream>

wxBEGIN_EVENT_TABLE(CSettingsDialog, wxDialog)
EVT_BUTTON(wxID_EXIT, CSettingsDialog::OnQuit)
wxEND_EVENT_TABLE()

/// A constructor
CSettingsDialog::CSettingsDialog(wxWindow* parent, const wxString& title)
	: wxDialog(parent, wxID_ANY, title, wxPoint(wxID_ANY, wxID_ANY), wxSize(380, 495))
{
	m_isInput = true;
	try {
		m_model = new CSettingsModel(wxT("settings.xml"));
		m_settings = m_model->LoadSettings(SAVED_SETTINGS);
	}
	catch(CException* e)
	{
		wxLogError(e->GetMessage());
		delete e;
		
		// if isn't input file with settings, use default value
		m_isInput = false;
		m_model = NULL;
		m_settings = new DETECTOR_SETTINGS(10, 60, 3.65, 3.65, 0, 5, 4, 300, 50, 0.005, 0.12, 200);
	}
	
	wxArrayString filterTypes;
	filterTypes.Add("Chebyshev II");
	
	wxPanel* panel = new wxPanel(this, wxID_ANY);
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* hbox1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox4 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox5 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox6 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox7 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox8 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox9 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox10 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox11 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox12 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox13 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbox14 = new wxBoxSizer(wxHORIZONTAL);

	// labels
	int xSize = 140;
	messageLabel = new wxStaticText(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize);

	bandLowLabel = new wxStaticText(panel, wxID_ANY, wxT("Band low: "),
									wxDefaultPosition, wxSize(xSize, -1));
	bandHighLabel = new wxStaticText(panel, wxID_ANY, wxT("Band high: "),
									 wxDefaultPosition, wxSize(xSize, -1));
	k1Label = new wxStaticText(panel, wxID_ANY, wxT("K1: "),
							   wxDefaultPosition, wxSize(xSize, -1));
	k2Label = new wxStaticText(panel, wxID_ANY, wxT("K2 (if K2=K1, isn't use): "),
							   wxDefaultPosition, wxSize(xSize, -1));
	k3Label = new wxStaticText(panel, wxID_ANY, wxT("K3: "),
							   wxDefaultPosition, wxSize(xSize, -1));
	winsizeLabel = new wxStaticText(panel, wxID_ANY, wxT("Winsize: "),
									wxDefaultPosition, wxSize(xSize, -1));
	noverlapLabel = new wxStaticText(panel, wxID_ANY, wxT("Noverlap: "),
									 wxDefaultPosition, wxSize(xSize, -1));
	bufferingLabel = new wxStaticText(panel, wxID_ANY, wxT("Buffering: "),
									  wxDefaultPosition, wxSize(xSize, -1));
	mhfLabel = new wxStaticText(panel, wxID_ANY, wxT("Main hum. freq.: "),
								wxDefaultPosition, wxSize(xSize, -1));
	decLabel = new wxStaticText(panel, wxID_ANY, wxT("Decimation: "),
								wxDefaultPosition, wxSize(xSize, -1));
	dtLabel = new wxStaticText(panel, wxID_ANY, wxT("Discharge tol: "),
							   wxDefaultPosition, wxSize(xSize, -1));
	putLabel = new wxStaticText(panel, wxID_ANY, wxT("Polyspike union time: "),
								wxDefaultPosition, wxSize(xSize, -1));
	
	// entries
	bandLowEntry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%i"), m_settings->m_band_low),
								  wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxIntegerValidator<int>(&m_settings->m_band_low, wxNUM_VAL_DEFAULT | wxFILTER_EMPTY));
	
	bandHighEntry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%i"), m_settings->m_band_high),
								   wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxIntegerValidator<int>(&m_settings->m_band_high, wxNUM_VAL_DEFAULT | wxFILTER_EMPTY));
	
	k1Entry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%f"), m_settings->m_k1),
							 wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxFloatingPointValidator<double>(&m_settings->m_k1, wxFILTER_NUMERIC | wxFILTER_EMPTY));
	
	k2Entry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%f"), m_settings->m_k2),
							 wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxFloatingPointValidator<double>(&m_settings->m_k2, wxFILTER_NUMERIC | wxFILTER_EMPTY));

	k3Entry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%f"), m_settings->m_k3),
							 wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxFloatingPointValidator<double>(&m_settings->m_k3, wxFILTER_NUMERIC | wxFILTER_EMPTY));
	
	winsizeEntry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%i"), m_settings->m_winsize),
								  wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT,  wxIntegerValidator<int>(&m_settings->m_winsize, wxNUM_VAL_DEFAULT | wxFILTER_EMPTY));
	
	noverlapEntry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%f"), m_settings->m_noverlap),
								   wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxFloatingPointValidator<double>(&m_settings->m_noverlap, wxFILTER_NUMERIC | wxFILTER_EMPTY));
	
	bufferingEntry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%i"), m_settings->m_buffering),
									wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT,  wxIntegerValidator<int>(&m_settings->m_buffering, wxNUM_VAL_DEFAULT | wxFILTER_EMPTY));
	
	mhfEntry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%i"), m_settings->m_main_hum_freq),
							  wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT,  wxIntegerValidator<int>(&m_settings->m_main_hum_freq, wxNUM_VAL_DEFAULT | wxFILTER_EMPTY));

	decEntry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%i"), m_settings->m_decimation),
							  wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT,  wxIntegerValidator<int>(&m_settings->m_decimation, wxNUM_VAL_DEFAULT | wxFILTER_EMPTY));

	dtEntry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%f"), m_settings->m_discharge_tol),
							 wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxFloatingPointValidator<double>(&m_settings->m_discharge_tol, wxFILTER_NUMERIC | wxFILTER_EMPTY));
	
	putEntry = new wxTextCtrl(panel, wxID_ANY, wxString::Format(wxT("%f"), m_settings->m_polyspike_union_time),
							  wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxFloatingPointValidator<double>(&m_settings->m_polyspike_union_time, wxFILTER_NUMERIC | wxFILTER_EMPTY));

	hbox1->Add(bandLowLabel, 0);
	hbox1->Add(bandLowEntry, 1);
	hbox2->Add(bandHighLabel, 0);
	hbox2->Add(bandHighEntry, 1);
	hbox3->Add(k1Label, 0);
	hbox3->Add(k1Entry, 1);
	hbox4->Add(k2Label, 0);
	hbox4->Add(k2Entry, 1);
	hbox5->Add(k3Label, 0);
	hbox5->Add(k3Entry, 1);
	hbox6->Add(winsizeLabel, 0);
	hbox6->Add(winsizeEntry, 1);
	hbox7->Add(noverlapLabel, 0);
	hbox7->Add(noverlapEntry, 1);
	hbox8->Add(bufferingLabel, 0);
	hbox8->Add(bufferingEntry, 1);
	hbox9->Add(mhfLabel, 0);
	hbox9->Add(mhfEntry, 1);
	hbox10->Add(decLabel, 0);
	hbox10->Add(decEntry, 1);
	hbox11->Add(dtLabel, 0);
	hbox11->Add(dtEntry, 1);
	hbox12->Add(putLabel, 0);
	hbox12->Add(putEntry, 1);
	
	// buttons
	if (m_isInput)
	{
		buttonLoadDefault = new wxButton(panel, BUTTON_LoadDefault, wxT("Load default"));
		buttonLoadSaved = new wxButton(panel, BUTTON_LoadSaved, wxT("Load saved"));
		buttonSave = new wxButton(panel, BUTTON_Save, wxT("Save"));
		
		hbox13->Add(buttonLoadDefault);
		hbox13->Add(buttonLoadSaved);
		hbox13->Add(buttonSave);
	}
	buttonUse = new wxButton(panel, BUTTON_Use, wxT("Use"));
	hbox13->Add(buttonUse);
	
	hbox14->Add(messageLabel);

	vbox->Add(hbox1, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox2, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox3, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox4, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox5, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox6, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox7, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox8, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox9, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox10, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox11, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox12, 2, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox13, 2, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
	vbox->Add(hbox14, 2, wxEXPAND | wxLEFT | wxTOP | wxRIGHT | wxBOTTOM, 10);
	
	panel->SetSizer(vbox);
	
	// if user click close button window
	Bind(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(CSettingsDialog::OnClose), this);

	// connect methods to buttons
	Connect(BUTTON_Use, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CSettingsDialog::OnUse));
	if (m_isInput)
	{
		Connect(BUTTON_LoadDefault, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CSettingsDialog::OnLoadDefault));
		Connect(BUTTON_LoadSaved, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CSettingsDialog::OnLoadSaved));
		Connect(BUTTON_Save, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CSettingsDialog::OnSave));
	}
	
	ValidateInput();
	CenterOnParent();
	Centre();
}

/// A destructor
CSettingsDialog::~CSettingsDialog()
{
	if (m_model != NULL) delete m_model;
	delete m_settings;
}

/// Is called on quit the settings modal dialog
void CSettingsDialog::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Destroy();
	Close(true);
}

/// Set the changet values
void CSettingsDialog::OnUse(wxCommandEvent& WXUNUSED(event))
{
	double k1, k2, k3, noverlap, discharge_tol, polyspike_union_time;
	long   band_low, band_high, winsize, buffering, main_hum_freq, decimation;
	
	if (!ValidateInput()) return;
	
	bandLowEntry->GetValue().ToLong(&band_low);
	bandHighEntry->GetValue().ToLong(&band_high);
	k1Entry->GetValue().ToDouble(&k1);
	k2Entry->GetValue().ToDouble(&k2);
	k3Entry->GetValue().ToDouble(&k3);
	winsizeEntry->GetValue().ToLong(&winsize);
	noverlapEntry->GetValue().ToDouble(&noverlap);
	bufferingEntry->GetValue().ToLong(&buffering);
	mhfEntry->GetValue().ToLong(&main_hum_freq);
	dtEntry->GetValue().ToDouble(&discharge_tol);
	putEntry->GetValue().ToDouble(&polyspike_union_time);
	decEntry->GetValue().ToLong(&decimation);
	
	// save settings
	if (m_settings != NULL) delete m_settings;
	m_settings = new DETECTOR_SETTINGS(band_low, band_high,  k1, k2, k3, winsize, noverlap, buffering, main_hum_freq, discharge_tol,
									   polyspike_union_time, decimation);
	
	Freeze();
	messageLabel->SetLabel(wxT("Settings were set."));
	Thaw();
}

/// Close settings modal dialog
void CSettingsDialog::OnClose(wxCloseEvent& event)
{
	GetParent()->Enable();
	event.Skip();
}

/// Verify if the inputs from entries are correct
bool CSettingsDialog::ValidateInput()
{
	bool ret = true;
	wxString colorBad = "#FF0000";
	wxString colorOk =  "#FFFFFF";
	
	wxString errMessage;
	
	long bandLow, bandHigh, winsize, buffering, mhf, decimation;
	double k1, k2, k3, noverlap, dt, put;

	// load values from entries
	bandLowEntry->GetValue().ToLong(&bandLow);
	bandHighEntry->GetValue().ToLong(&bandHigh);
	k1Entry->GetValue().ToDouble(&k1);
	k2Entry->GetValue().ToDouble(&k2);
	k3Entry->GetValue().ToDouble(&k3);
	winsizeEntry->GetValue().ToLong(&winsize);
	noverlapEntry->GetValue().ToDouble(&noverlap);
	bufferingEntry->GetValue().ToLong(&buffering);
	mhfEntry->GetValue().ToLong(&mhf);
	dtEntry->GetValue().ToDouble(&dt);
	putEntry->GetValue().ToDouble(&put);
	decEntry->GetValue().ToLong(&decimation);

	// check if values are correct
	Freeze();
	if (bandLow <= 0 || (bandLow > bandHigh && bandHigh > 0)) {
		ret = false;
		bandLowEntry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nBand low must be: 0 < band low < band high";
	} else bandLowEntry->SetBackgroundColour(wxColour(colorOk));
	
	if (bandHigh <= 0 || bandHigh > decimation) {
		ret = false;
		bandHighEntry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nBand hight must be: band low < band high < decimation";
	} else bandHighEntry->SetBackgroundColour(wxColour(colorOk));
	
	if (k1 <= 0) {
		ret = false;
		k1Entry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nK1 must be > 0";
	} else k1Entry->SetBackgroundColour(wxColour(colorOk));
	
	if (k2 < 0 || k2 > k1)
	{
		ret = false;
		k2Entry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nK2 must be: 0 < K2 <= K1";
	} else k2Entry->SetBackgroundColour(wxColour(colorOk));
	
	if (k3 < 0)
	{
		ret = false;
		k3Entry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nK3 must be >= 0";
	} else k3Entry->SetBackgroundColour(wxColour(colorOk));

	if (winsize <= 0)
	{
		ret = false;
		winsizeEntry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nWinsize must be > 0";
	} else winsizeEntry->SetBackgroundColour(wxColour(colorOk));
	
	if (noverlap <= 0)
	{
		ret = false;
		noverlapEntry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nNoverlap must be > 0";
	} else noverlapEntry->SetBackgroundColour(wxColour(colorOk));
	
	int ratio = buffering / winsize;
	if (ratio < 10)
	{
		ret = false;
		bufferingEntry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nBuffering must be: 10*winsize <= buffering";
	} else bufferingEntry->SetBackgroundColour(wxColour(colorOk));
	
	if (mhf <= 0)
	{
		ret = false;
		mhfEntry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nMain hum. f. must be > 0";
	} else mhfEntry->SetBackgroundColour(wxColour(colorOk));
	
	if (dt <= 0)
	{
		ret = false;
		dtEntry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nDischarge tool must be > 0";
	} else dtEntry->SetBackgroundColour(wxColour(colorOk));
	
	if (put < 0)
	{
		ret = false;
		putEntry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nPolyspike un. time must be > 0";
	} else putEntry->SetBackgroundColour(wxColour(colorOk));

	if (decimation <= 0)
	{
		ret = false;
		decEntry->SetBackgroundColour(wxColour(colorBad));
		errMessage += "\nDecimation must be > 0";
	} else decEntry->SetBackgroundColour(wxColour(colorOk));
	
	// if error show message
	if (!ret)
	{
		messageLabel->SetLabel(wxT("Settings are invalid."));
		wxLogMessage(errMessage);
	}
	else messageLabel->SetLabel(wxT("All settings are valid."));
	Thaw();

	return ret;
}

/// Save changed values of settings to the XML
void CSettingsDialog::OnSave(wxCommandEvent& event)
{
	try
	{
		if (!ValidateInput()) return;
		this->OnUse(event);
		m_model->SaveSettings(m_settings);
	}
	catch(CException * e)
	{
		wxLogError(e->GetMessage());
		delete e;
	}
	
	Freeze();
	messageLabel->SetLabel(wxT("Settings were saved."));
	Thaw();
}

/// Load defautl settings from XML
void CSettingsDialog::OnLoadDefault(wxCommandEvent& WXUNUSED(event))
{
	try {
		if (m_settings != NULL) delete m_settings;
		m_settings = m_model->LoadSettings(DEFAULT_SETTINGS);
		RefreshControls();
	}
	catch (CException * e)
	{
		wxLogError(e->GetMessage());
		delete e;
	}
}

/// Load saved settings from XML
void CSettingsDialog::OnLoadSaved(wxCommandEvent& WXUNUSED(event))
{
	try {
		if (m_settings != NULL) delete m_settings;
		m_settings = m_model->LoadSettings(SAVED_SETTINGS);
		RefreshControls();
	}
	catch (CException * e)
	{
		wxLogError(e->GetMessage());
		delete e;
	}
}

/// Refresch controls
void CSettingsDialog::RefreshControls()
{
	Freeze();
	bandLowEntry->SetValue(wxString::Format(wxT("%i"), m_settings->m_band_low));
	bandHighEntry->SetValue(wxString::Format(wxT("%i"), m_settings->m_band_high));
	k1Entry->SetValue(wxString::Format(wxT("%f"), m_settings->m_k1));
	k2Entry->SetValue(wxString::Format(wxT("%f"), m_settings->m_k2));
	k3Entry->SetValue(wxString::Format(wxT("%f"), m_settings->m_k3));
	winsizeEntry->SetValue(wxString::Format(wxT("%i"), m_settings->m_winsize));
	noverlapEntry->SetValue(wxString::Format(wxT("%f"), m_settings->m_noverlap));
	bufferingEntry->SetValue(wxString::Format(wxT("%i"), m_settings->m_buffering));
	mhfEntry->SetValue(wxString::Format(wxT("%i"), m_settings->m_main_hum_freq));
	dtEntry->SetValue(wxString::Format(wxT("%f"), m_settings->m_discharge_tol));
	putEntry->SetValue(wxString::Format(wxT("%f"), m_settings->m_polyspike_union_time));
	decEntry->SetValue(wxString::Format(wxT("%i"), m_settings->m_decimation));
	Thaw();
}

/// Verify if the inputs from entries are correct
bool CSettingsDialog::ValidateValues()
{
	int tmp;

	// the right thing would not happen
	if (m_settings == NULL) throw CException(wxT("Settings isn't loaded / created."), wxT("CSettingsDialog::ValidateValues"));

	// VERIFY buffering
	// if buffering/(winsize/10) < 10 -> set buffering to (winsize * 10)
	tmp = m_settings->m_buffering / m_settings->m_winsize;

	if (tmp < 10)
	{
		m_settings->m_buffering = m_settings->m_winsize  * 10;
		RefreshControls();

		wxLogMessage(wxString::Format(wxT("Warning\nBuffer size incresed to %i"), m_settings->m_buffering));
	}

	return true;
}
