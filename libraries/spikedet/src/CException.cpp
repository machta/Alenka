#include "CException.h"

/// A constructor
CException::CException(wxString message, wxString whence)
{
	m_message = message;
	m_whence = whence;
//	m_time = wxDateTime::Now();
}

// A empty virtual destructor
CException::~CException()
{
	/* empty */
}
