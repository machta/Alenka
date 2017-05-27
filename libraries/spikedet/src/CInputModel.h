#ifndef CInputModel_H
#define	CInputModel_H

#include "wx/wx.h"

#include "CException.h"
#include "Definitions.h"

/**
 * Abstract class.
 * This representing a Model for loading input data (iEEG signal) from file.
 */
class CInputModel
{
protected:
	/// sample rate of data in file
	int 		  m_fs;
	/// Number of samples of signal in the file
	int 		  m_countSamples;
	/// Contains a channels which are correct
	wxVector<int> m_channels;
	
public:
	/**
	 * A empty constructor
	 */
	CInputModel() { /* empty */ };

	/**
	 * A empty virtual destructor
	 */
	virtual ~CInputModel() { /* empty */ };
	
	/**
	 * A pure overloaded virtual method for open the input file.
	 * @param fileName name of the input file
	 */
	virtual void OpenFile(const char * fileName) = 0;
	
	/**
	 * A pure overloaded virtual method for open the input file.
	 * Call CInputModel::OpenFile(const char * fileName)
	 * @param fileName name of the input file
	 */
	virtual void OpenFile(const wchar_t * fileName) = 0;
	

	/**
	 * A pure overloaded virtual method for close the input fiel.
	 */
	virtual void CloseFile() = 0;

	/**
	 * A pure virtual method. It returns if is input file open.
	 */
	virtual bool IsOpen() const = 0;
	
	/**
	 * A pure virtual method. It returns if is end of input file.
	 */
	virtual bool IsEnd() const = 0;
	
	/**
	 * Returns the sample rate.
	 */
	inline int GetFS() const
	{
		return m_fs;
	}

	/**
	 * Returns count of channels.
	 */
	inline int GetCountChannels() const
	{
		return m_channels.size();
	}

	/**
	 * Returns count of samples in one channel.
	 */
	inline int GetCountSamples() const
	{
		return m_countSamples;
	}

	/**
	 * A pure virtual method for getting a segment of data.
	 * @param start a position of start sample.
	 * @param end a position of end sample.
	 * @return a array of wxVectors obtaining samples or NULL, if isn´t any data to read.
	 */
	virtual wxVector<SIGNALTYPE>* GetSegment(const int& start, const int& end) = 0;
};

#endif
