#ifndef CInputModelEDF_H
#define	CInputModelEDF_H

#include "CInputModel.h"
#include "lib/edflib.h"

/**
 * This representing a Model for loading input data (iEEG signal) from EDF/EDF+ file.
 */
class CInputModelEDF : public CInputModel
{
	// methods
public:
	/**
	 * A constructor.
	 */
	CInputModelEDF();

	/**
	 * A desctructor.
	 * Close file if is open.
	 */
	~CInputModelEDF();

	// required methods
	/**
	 * Open the input file.
	 * @param fileName name of the input file
	 */
	void OpenFile(const char* fileName);

	/**
	 * Open the input file.
	 * Call CInputModelEDF::#OpenFile(const char* fileName)
	 * @param fileName name of the input file
	 */
	void OpenFile(const wchar_t* fileName);
	
	/**
	 * Close input file if is open.
	 */
	void CloseFile();

	/**
	 * Indicates whether is the file open.
	 * @return If is file open return true. If isn't return false.
	 */
	bool IsOpen() const;

	/**
	 * Return next segment of data.
	 * Before calling must be set buffering and winsize: CModel::SetBufferSizes(const int&, const int&)
	 */
	//CSignal* GetNextSegment();
	
	/**
	 * Return the header structure.
	 * @return edf_hdr_struct
	 */
	inline struct edf_hdr_struct GetHeader() const { return m_hdr; };
	
	/**
	 * Do rewind of all channels.
	 */
	void Rewind();

	/**
	 * Do seek of all channels.
	 * @param offset
	 * @param whence
	 */
	void Seek(const long long& offset, const int& whence);
	
	/**
	 * Signalise if is end of file.
	 * @return bool indicates end of file.
	 */
	bool IsEnd() const;

	/**
	 * Read abd return a segment of data.
	 * If is not end of file return array of vectors, else return NULL
	 * @param start a position of start sample.
	 * @param end a position of end sample.
	 * @return a array of wxVectors obtaining samples or NULL, if isn´t any data to read.
	 */
	wxVector<SIGNALTYPE>* GetSegment(const int& start, const int& end);
private:

	// variables
public:
	/* none */
private:
	/// A private variable. Header structure.
	struct edf_hdr_struct 	m_hdr;

	/// A private variable. Indicates whether is the end of the file.
	bool				  	m_endOfFile;
	/// A private variable. Indicates whether is the file open.
	bool					m_isOpen;
	/// A private variable. Positions start of a new segment.
	int 				  	m_start;
	/// A private variable. T_seg.
	int  				  	m_T_seg;
};

#endif
