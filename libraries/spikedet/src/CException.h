#ifndef CException_H
#define	CException_H

#include "wx/wx.h"

#include <exception>

/**
 * Class representing exceptions in SpikeDetektor, inherist from std::exception
 */
class CException: public std::exception
{
public:
	/**
	 * A constructor
	 * @param message a message with description of the exception
	 * @param whence the Class::method where exception were thrown
	 */
	CException(wxString message, wxString whence);

	/**
	 * A empty virtual destructor
	 */
	virtual ~CException();
	
	/**
	 * Returns message
	 * @return message as wxString
	 */
	inline wxString GetMessage() const
	{
		return m_message;
	}

	/**
	 * Returns a location where exception was thrown.
	 * @return location where exception was thrown as wxString
	 */
	inline wxString GetLocation() const
	{
		return m_whence;
	}

	/**
	 * A virtual method, return message
	 * @return message as const char*
	 */
	virtual const char* what() const throw()
	{
		return m_message.c_str();
	}

private:
	/// a message
	wxString   m_message;
	/// a location where exception was thrown
	wxString   m_whence;
	/// the time when exception was thrown
//	wxDateTime m_time;
};

#endif
