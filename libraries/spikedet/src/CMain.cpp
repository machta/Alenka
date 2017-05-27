#include "CMain.h"

IMPLEMENT_APP(CMain);

/// This method is called when the application starts
bool CMain::OnInit()
{
	if (!wxApp::OnInit())
	{
		return false;
	}

	m_model = new CInputModelEDF();
	m_frame = new CMainFrame( "Spike detector", wxPoint(50, 50), wxSize(1050, 550), m_model);

	m_frame->Show(true);

	return true;
}

/// This method is called when the application exits
int CMain::OnExit()
{
	delete m_model;

	return 0;
}
