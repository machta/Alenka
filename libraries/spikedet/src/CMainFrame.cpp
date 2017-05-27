#include "CMainFrame.h"

wxBEGIN_EVENT_TABLE(CMainFrame, wxFrame)
EVT_MENU(MENU_FILE_Open, CMainFrame::OnOpen)
EVT_MENU(MENU_FILE_Load, CMainFrame::OnLoadResults)
EVT_MENU(MENU_FILE_Save, CMainFrame::OnSaveResults)

EVT_MENU(MENU_DETECTOR_Run, CMainFrame::OnRun)
EVT_MENU(MENU_DETECTOR_Option, CMainFrame::OnOptions)

EVT_MENU(wxID_EXIT, CMainFrame::OnExit)
EVT_MENU(wxID_ABOUT, CMainFrame::OnAbout)

EVT_THREAD(DETECTOR_EVENT, CMainFrame::OnDetectorEvent)

EVT_CLOSE(CMainFrame::OnClose)
wxEND_EVENT_TABLE()


/// A constructor
CMainFrame::CMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, CInputModel * model)
	: wxFrame(NULL, wxID_ANY, title, pos, size), m_model(model), m_out(NULL), m_disch(NULL)
{
	wxImage::AddHandler(new wxPNGHandler);
	wxBitmap bitmapPlay(wxT("icon/play-icon.png"), wxBITMAP_TYPE_PNG);
	wxBitmap bitmapOpen(wxT("icon/open-icon.png"), wxBITMAP_TYPE_PNG);
	wxBitmap bitmapSaveResults(wxT("icon/save-icon.png"), wxBITMAP_TYPE_PNG);
	wxBitmap bitmapLoadResults(wxT("icon/load-icon.png"), wxBITMAP_TYPE_PNG);
	wxBitmap bitmapSett(wxT("icon/settings-icon.png"), wxBITMAP_TYPE_PNG);
	
	// Create menu
	m_menuFile = new wxMenu;
	m_menuFile->Append(MENU_FILE_Open, "&Open file\tCtrl+O", "Open EDF file to analyse.");
	m_menuFile->AppendSeparator();
	m_menuFile->Append(MENU_FILE_Load, "&Load results\tCtrl+L", "Load analysis results.");
	m_menuFile->Append(MENU_FILE_Save, "&Save results\tCtrl+S", "Save analysis results.");
	m_menuFile->Enable(MENU_FILE_Save, false);
	m_menuFile->AppendSeparator();
	m_menuFile->Append(wxID_EXIT);

	m_menuDetector = new wxMenu;
	m_menuDetector->Append(MENU_DETECTOR_Run, "&Run\tCtrl+R", "Run analysis.");
	m_menuDetector->Enable(MENU_DETECTOR_Run, false);
	m_menuDetector->Append(MENU_DETECTOR_Option, "&Options\tCtrl+T", "Change settings of the detector.");
	//m_menuDetector->Enable(MENU_DETECTOR_Option, false);
	
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(m_menuFile, "&File");
	menuBar->Append(m_menuDetector, "&Detector");
	menuBar->Append(menuHelp, "&Help");
	
	SetMenuBar(menuBar);
	CreateStatusBar();

	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* vbox1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* vbox2 = new wxBoxSizer(wxVERTICAL);
	
	// TOOLBAR
	m_mainToolbar = CreateToolBar();
	m_mainToolbar->AddTool(TOOLBAR_Open, wxT("Open file"), bitmapOpen, wxT("Open EDF file to analyse."), wxITEM_NORMAL);
	m_mainToolbar->AddSeparator();
	m_mainToolbar->AddTool(TOOLBAR_Run, wxT("Run analysis"), bitmapPlay, wxT("Run analysis."), wxITEM_NORMAL);
	m_mainToolbar->AddTool(TOOLBAR_Sett, wxT("Set detector"), bitmapSett, wxT("Change settings of the detector."), wxITEM_NORMAL);
	m_mainToolbar->AddSeparator();
	m_mainToolbar->AddTool(TOOLBAR_Save, wxT("Save results"), bitmapSaveResults, wxT("Save analysis results."), wxITEM_NORMAL);
	m_mainToolbar->AddTool(TOOLBAR_Load, wxT("Load results"), bitmapLoadResults, wxT("Load analysis results."), wxITEM_NORMAL);
	m_mainToolbar->EnableTool(TOOLBAR_Run, false);
	m_mainToolbar->EnableTool(TOOLBAR_Save, false);
	m_mainToolbar->Realize();
	
	// GRIDS
	// output grid
	m_gridOut = new wxGrid(this, GRID_Output, wxPoint(0,0), wxSize(490,550));
	m_gridOut->CreateGrid(0, 5);

	m_gridOut->SetColLabelValue(0, "Position:");
	m_gridOut->SetColLabelValue(1, "Channel:");
	m_gridOut->SetColLabelValue(2, "Type:");
	m_gridOut->SetColLabelValue(3, "CDF:");
	m_gridOut->SetColLabelValue(4, "PDF:");

	m_gridOut->SetColFormatFloat(0);
	m_gridOut->SetColFormatNumber(1);
	m_gridOut->SetColFormatFloat(2);
	m_gridOut->SetColFormatFloat(3);
	m_gridOut->SetColFormatFloat(4);

	// discharges grid
	m_gridDisch = new wxGrid(this, GRID_Discharges, wxPoint(0,0), wxSize(620,550));
	m_gridDisch->CreateGrid(0, 8);

	m_gridDisch->SetColLabelValue(0, "Record:");
	m_gridDisch->SetColLabelValue(1, "Chan:");
	m_gridDisch->SetColLabelValue(2, "MV:");
	m_gridDisch->SetColLabelValue(3, "MA:");
	m_gridDisch->SetColLabelValue(4, "MP:");
	m_gridDisch->SetColLabelValue(5, "MD:");
	m_gridDisch->SetColLabelValue(6, "MW:");
	m_gridDisch->SetColLabelValue(7, "MPDF:");

	m_gridDisch->SetColFormatNumber(0);
	m_gridDisch->SetColFormatNumber(1);
	m_gridDisch->SetColFormatFloat(2);
	m_gridDisch->SetColFormatFloat(3);
	m_gridDisch->SetColFormatFloat(4);
	m_gridDisch->SetColFormatFloat(5);
	m_gridDisch->SetColFormatFloat(6);
	m_gridDisch->SetColFormatFloat(7);

	// --

	vbox1->Add(m_gridOut, 1);
	vbox2->Add(m_gridDisch, 1);
	hbox->Add(vbox1, 1, wxEXPAND);
	hbox->Add(vbox2, 1, wxEXPAND);

	SetSizer(hbox);
	Connect(TOOLBAR_Run,  wxEVT_COMMAND_TOOL_CLICKED,  wxCommandEventHandler(CMainFrame::OnRun));
	Connect(TOOLBAR_Open, wxEVT_COMMAND_TOOL_CLICKED,  wxCommandEventHandler(CMainFrame::OnOpen));
	Connect(TOOLBAR_Save, wxEVT_COMMAND_TOOL_CLICKED,  wxCommandEventHandler(CMainFrame::OnSaveResults));
	Connect(TOOLBAR_Load, wxEVT_COMMAND_TOOL_CLICKED,  wxCommandEventHandler(CMainFrame::OnLoadResults));
	Connect(TOOLBAR_Sett, wxEVT_COMMAND_TOOL_CLICKED,  wxCommandEventHandler(CMainFrame::OnOptions));

	m_settingsDialog = new CSettingsDialog(this, wxT("Settings of detector"));
	
	Centre();
}

/// Define operations when clicking to exit application
void CMainFrame::OnExit(wxCommandEvent& event)
{
	// invokes wxCloseEvent - is called OnClose(...)
	Close(true);
}

/// Define operations when closing application
void CMainFrame::OnClose(wxCloseEvent& event)
{
	// free memory
	if (m_out != NULL)
		delete m_out;
	if (m_disch != NULL)
		delete m_disch;

	// destroy/end application
	Destroy();
}

/// Show information about application
void CMainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox("This is implementation of spike detector.",
				 "About spike detector", wxOK | wxICON_INFORMATION );
}

// Runs analysis of input data from EDF file
void CMainFrame::OnRun(wxCommandEvent& WXUNUSED(event))
{
	DETECTOR_SETTINGS* settings = m_settingsDialog->GetSettings();

	m_settingsDialog->ValidateValues();
	
	m_out = new CDetectorOutput();
	m_disch = new CDischarges(m_model->GetCountChannels());
	m_detector = new CSpikeDetector((wxEvtHandler*)this, m_model, settings, m_out, m_disch);
	
	// create thread
	if (m_detector->Create() != wxTHREAD_NO_ERROR)
	{
		wxLogError(wxT("Error starting detector. Can't create thread!"));
		return;
	}

	// show progress dialog
	m_dlgProgress = new wxProgressDialog(
				wxT("Progress dialog"),
				wxT("Wait until the detector terminates analysis or [Cancel]"),
				100,
				this,
				wxPD_CAN_ABORT |
				wxPD_APP_MODAL |
				wxPD_ELAPSED_TIME |
				wxPD_ESTIMATED_TIME |
				wxPD_REMAINING_TIME
				);

	m_cancelled = false; // thread is not running yet, no need for crit sect
	m_sw.Start(); // start time measuring
	m_detector->Run(); // run thread
}

/// Show open file dialog for selection of input EDF file
void CMainFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{	  
	int answer;

	// Already is open some file, open new?
	if (m_model->IsOpen())
	{
		if (m_isResultsSaved)
			answer = wxMessageBox("Do you want to open a new file?", "Confirm", wxYES_NO, NULL);
		else
			answer = wxMessageBox("Do you want to open a new file? Results are not saved!", "Confirm", wxYES_NO, NULL);

		if (answer != wxYES) return;
		
		this->clear();
	}

	// show open file dialog
	wxFileDialog openFileDialog(this, _("Open EDF/EDF+ file"), "", "", "EDF files (*.edf)|*.edf", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...

	wxBusyInfo wait("Please wait, opening file ...");

	// load file
	try
	{
		m_model->OpenFile(openFileDialog.GetPath().wc_str());

		m_mainToolbar->EnableTool(TOOLBAR_Run, true);
		m_menuDetector->Enable(MENU_DETECTOR_Run, true);
	}
	catch (CException * e)
	{
		wxLogError(e->GetMessage());
		delete e;
	}
}

/// Show settings dialog
void CMainFrame::OnOptions(wxCommandEvent& WXUNUSED(event))
{
	m_settingsDialog->ShowModal();
}

/// Captures events from CSpikeDetector
void CMainFrame::OnDetectorEvent(wxThreadEvent& event)
{
	int n = event.GetInt();
	wxString err = event.GetString();

	if (m_dlgProgress == NULL)
		return;

	// end or cancell analysis
	if (n == -1)
	{
		m_sw.Pause();	// stop time measuring
		std::cout << "Time for analysis: " << (double)(m_sw.Time()/1000.0) << std::endl;

		m_dlgProgress->Destroy();
		m_dlgProgress = (wxProgressDialog *)NULL;

		// the dialog is aborted because the event came from another thread, so
		// we may need to wake up the main event loop for the dialog to be
		// really closed
		wxWakeUpIdle();
		if (err.length() == 0)
		{
			m_mainToolbar->EnableTool(TOOLBAR_Save, true);
			m_isResultsSaved = false;
			plotOutputs();
		}
		else
		{
			this->clear();
			wxLogError(err);
		}
	}
	// update analysis progress
	else
	{
		if (m_dlgProgress != (wxProgressDialog *)NULL && n < 100)
		{
			if (!m_dlgProgress->Update(n))
			{
				wxCriticalSectionLocker lock(m_csCancelled);
				if (m_detector != NULL)
					m_detector->Delete();
				m_cancelled = true;
			}
		}
	}
}

/// Plot detector outputs (results of analysis) to data grids.
void CMainFrame::plotOutputs()
{
	unsigned i, j, cntElems, rows;

	wxBusyInfo wait("Please wait, plotting results ...");

	// clear grids
	if (m_gridOut->GetNumberRows() > 0)
		m_gridOut->DeleteRows(0, m_gridOut->GetNumberRows(), true);
	if (m_gridDisch->GetNumberRows() > 0)
		m_gridDisch->DeleteRows(0, m_gridDisch->GetNumberRows(), true);

	Freeze();
	// show OUT
	cntElems = m_out->m_pos.size();
	rows = m_gridOut->GetNumberRows();
	for (i = 0; i < cntElems; i++)
	{
		m_gridOut->InsertRows(rows, 1);

		m_gridOut->SetCellValue(rows, 0, wxString::Format(wxT("%f"), m_out->m_pos.at(i)));
		m_gridOut->SetCellValue(rows, 1, wxString::Format(wxT("%d"), m_out->m_chan.at(i)));
		m_gridOut->SetCellValue(rows, 2, wxString::Format(wxT("%f"), m_out->m_con.at(i)));
		m_gridOut->SetCellValue(rows, 3, wxString::Format(wxT("%f"), m_out->m_weight.at(i)));
		m_gridOut->SetCellValue(rows, 4, wxString::Format(wxT("%f"), m_out->m_pdf.at(i)));

		rows++;
	}

	// show DISCHARGES
	rows = m_gridDisch->GetNumberRows();
	cntElems = m_disch->m_MP[0].size();
	for (i = 0; i < cntElems; i++)
	{
		for (j = 0; j < m_disch->GetCountChannels(); j++)
		{
			m_gridDisch->InsertRows(rows, 1);

			m_gridDisch->SetCellValue(rows, 0, wxString::Format(wxT("%d"), i+1));
			m_gridDisch->SetCellValue(rows, 1, wxString::Format(wxT("%d"), j+1));
			m_gridDisch->SetCellValue(rows, 2, wxString::Format(wxT("%f"), (double)m_disch->m_MV[j].at(i)));
			m_gridDisch->SetCellValue(rows, 3, wxString::Format(wxT("%f"), (double)m_disch->m_MA[j].at(i)));
			m_gridDisch->SetCellValue(rows, 4, wxString::Format(wxT("%f"), (double)m_disch->m_MP[j].at(i)));
			m_gridDisch->SetCellValue(rows, 5, wxString::Format(wxT("%f"), (double)m_disch->m_MD[j].at(i)));
			m_gridDisch->SetCellValue(rows, 6, wxString::Format(wxT("%f"), (double)m_disch->m_MW[j].at(i)));
			m_gridDisch->SetCellValue(rows, 7, wxString::Format(wxT("%f"), (double)m_disch->m_MPDF[j].at(i)));

			rows++;
		}
	}

	// autosize
	m_gridOut->SetRowLabelSize( wxGRID_AUTOSIZE );
	m_gridDisch->SetRowLabelSize( wxGRID_AUTOSIZE );
	m_gridDisch->AutoSizeColumn( 0, false );
	m_gridDisch->AutoSizeColumn( 1, false );
	m_gridDisch->AutoSizeColumn( 2, false );
	Thaw();
}

// Show save file dialog to save results to file
void CMainFrame::OnSaveResults(wxCommandEvent& WXUNUSED(event))
{
	// show save file dialog
	wxFileDialog saveFileDialog(this, _("Save results"), "", "", "MAT files (*.mat)|*.mat|XML files (*.xml)|*.xml", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	if (saveFileDialog.ShowModal() == wxID_CANCEL)
		return;

	wxBusyInfo wait("Please wait, saving results ...");

	// save results to the file
	// if is choose MAT use SaveResultsMAT - save results to MAT
	// else if is choose XML use SaveResultsXML - save results to XML
	// else error
	try
	{
		wxString fileType = saveFileDialog.GetFilename().Right(3);

		if (fileType.IsSameAs(wxT("mat")))
			CResultsModel::SaveResultsMAT(saveFileDialog.GetPath().wc_str(), m_out, m_disch);
		else if(fileType.IsSameAs(wxT("xml")))
			CResultsModel::SaveResultsXML(saveFileDialog.GetPath().wc_str(), m_out, m_disch);
		else
			throw new CException(wxT("Error unsupported data format!"), wxT("CMainFrame::OnSaveResults"));

		m_isResultsSaved = true;
	}
	catch (CException * e)
	{
		wxLogError(e->GetMessage());
		delete e;
	}
}

/// Show open file dialog to load results from file
void CMainFrame::OnLoadResults(wxCommandEvent& WXUNUSED(event))
{
	int dialogRet;

	// if is any results and are not saved - warning
	if (m_out != NULL || m_disch != NULL)
	{
		if (!m_isResultsSaved)
		{
			wxMessageDialog* dial = new wxMessageDialog(this,
														wxT("There are already some unsaved results! Do you want throw they away?"),
														wxT("Warning"),
														wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);

			dialogRet = dial->ShowModal();
			dial->Destroy();
			
			if (dialogRet == wxID_NO)
				return;
		}
		
		this->clear();
	}

	// show open file dialog
	wxFileDialog openFileDialog(this, _("Load results"), "", "", "MAT files (*.mat)|*.mat|XML files (*.xml)|*.xml", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;

	wxBusyInfo wait("Please wait, loading results ...");

	// load results from file
	try
	{
		wxString fileType = openFileDialog.GetFilename().Right(3);

		if (fileType.IsSameAs(wxT("mat")))
			CResultsModel::LoadResultsMAT(openFileDialog.GetPath().wc_str(), m_out, m_disch);
		else if(fileType.IsSameAs(wxT("xml")))
			CResultsModel::LoadResultsXML(openFileDialog.GetPath().wc_str(), m_out, m_disch);
		else
			throw new CException(wxT("Error unsupported data format!"), wxT("CMainFrame::OnLoadResults"));

		if (m_out != NULL && m_disch != NULL)
			plotOutputs();
		else
			throw new CException(wxString::Format(wxT("Error loading data from: %s"), openFileDialog.GetPath()), wxT("CMainFrame::OnLoadResults"));
	}
	catch (CException * e)
	{
		wxLogError(e->GetMessage());
		delete e;
	}
}

/// Clear results, close file, etc.
void CMainFrame::clear()
{
	m_model->CloseFile();
	if (m_out != NULL)
	{
		delete m_out;
		m_out = NULL;
	}
	if (m_disch != NULL)
	{
		delete m_disch;
		m_disch = NULL;
	}
	if (m_gridOut->GetNumberRows() > 0)
		m_gridOut->DeleteRows(0, m_gridOut->GetNumberRows(), true);
	if (m_gridDisch->GetNumberRows() > 0)
		m_gridDisch->DeleteRows(0, m_gridDisch->GetNumberRows(), true);
	m_mainToolbar->EnableTool(TOOLBAR_Save, false);
	m_isResultsSaved = false;
	m_mainToolbar->EnableTool(TOOLBAR_Run, false);
}
