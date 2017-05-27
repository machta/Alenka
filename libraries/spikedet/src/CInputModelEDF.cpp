#include <iostream>
#include <cmath>
#include <locale.h>

#include "CInputModelEDF.h"

using namespace std;

/// A constructor.
CInputModelEDF::CInputModelEDF()
{
	m_isOpen = false;
}

/// A desctructor.
CInputModelEDF::~CInputModelEDF()
{
	if (m_isOpen)
		edfclose_file(m_hdr.handle);
}

/// Open the input file.
void CInputModelEDF::OpenFile(const wchar_t* fileName)
{
	char buffer[2048];
	mbstate_t mbs;
	int ret;

	setlocale(LC_ALL, "");
	memset(&mbs, 0, sizeof(mbs));
	mbrlen(NULL , 0, &mbs);
	ret = wcsrtombs ( buffer, &fileName, sizeof(buffer), &mbs );
	setlocale(LC_ALL, "C");

	if(ret)
		OpenFile(buffer);
	else throw new CException(wxT("Error opening file. The name or file path is not correct."), wxT("CModelEDF::OpenFile"));
}

/// Open the input file.
void CInputModelEDF::OpenFile(const char* fileName)
{
	m_endOfFile = false;
	m_fs = 0;
	m_start = 0;

	if(edfopen_file_readonly(fileName, &m_hdr, EDFLIB_READ_ALL_ANNOTATIONS))
	{
		wxString error;
		wxString whence = "CModelEDF::OpenFile";
		switch(m_hdr.filetype)
		{
		case EDFLIB_MALLOC_ERROR:
			error = "Openning file: malloc error.";
			break;
		case EDFLIB_NO_SUCH_FILE_OR_DIRECTORY:
			error = "Can not open file, no such file or directory.";
			break;
		case EDFLIB_FILE_CONTAINS_FORMAT_ERRORS :
			error = "The file is not EDF(+) or BDF(+) compliant ""(it contains format errors).";
			break;
		case EDFLIB_MAXFILES_REACHED:
			error = "To many files opened.";
			break;
		case EDFLIB_FILE_READ_ERROR:
			error = "A read error occurred.";
			break;
		case EDFLIB_FILE_ALREADY_OPENED:
			error = "File has already been opened.";
			break;
		default:
			error = "Unknown error.";
			break;
		}

		throw new CException(error, whence);
	}

	// get highest sample rate and length of signal
	for (int i = 0; i < m_hdr.edfsignals; i++)
	{
		if (m_hdr.signalparam[i].smp_in_datarecord > m_fs)
		{
			m_fs = m_hdr.signalparam[i].smp_in_datarecord;
			m_countSamples = m_hdr.signalparam[i].smp_in_file;
		}
	}

	// get correct channels
	for (int i = 0; i < m_hdr.edfsignals; i++)
	{
		if (m_hdr.signalparam[i].smp_in_datarecord == m_fs && m_hdr.signalparam[i].smp_in_file == m_countSamples)
		{
			m_channels.push_back(i);
		}
	}

	m_isOpen = true;
}

/// Close input file if is open.
void CInputModelEDF::CloseFile()
{
	if (m_isOpen)
	{
		edfclose_file(m_hdr.handle);
		m_isOpen = false;
		m_channels.clear();
	}
}

/// Do rewind of all channels.
void CInputModelEDF::Rewind()
{
	int size = m_channels.size();
	for (int i = 0; i < size; i++)
	{
		edfrewind(m_hdr.handle, m_channels.at(i));
	}
}

/// Do seek of all channels.
void CInputModelEDF::Seek(const long long& offset, const int& whence)
{
	int size = m_channels.size();
	for (int i = 0; i < size; i++)
	{
		edfseek(m_hdr.handle, m_channels.at(i), offset, whence);
	}
}

/// Signalise if is end of file.
bool CInputModelEDF::IsEnd() const
{
	return m_endOfFile;
}

/// Indicates whether is the file open.
bool CInputModelEDF::IsOpen() const
{
	return m_isOpen;
}

/// Read and return a segment of data.
wxVector<SIGNALTYPE>* CInputModelEDF::GetSegment(const int& start, const int& end)
{
	if (m_endOfFile)
		return NULL;

	if (!m_isOpen)
		throw new CException(wxT("Warning: isn't open any file! You must first open input file!"), wxT("CModelEDF::GetNextSegment"));

	int 				  i;
	int 				  buffersize = end - start;
	int 				  countChannels = m_channels.size();
	wxVector<SIGNALTYPE>* data = new wxVector<SIGNALTYPE>[countChannels];
	double* 			  segment = new double[buffersize+10];
	int 				  ret = 0;
	int 				  channels = 0;


	for (i = 0; i < countChannels; i++)
	{
		edfseek(m_hdr.handle, m_channels.at(i), start, EDFSEEK_SET);
		ret = edfread_physical_samples(m_hdr.handle, m_channels.at(i), buffersize, segment);
		// error reading or end of channel
		if (ret == -1 || ret == 0) continue;
		data[channels].assign(segment, segment + ret);
		channels++;
	}

	return data;
}
