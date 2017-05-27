/*! \mainpage SpikeDetector
 *
 *  This is implementation of spike detector algorithm created by ISARG. It is based of MATLAB implementation version 20.
 *
 *  Implementation includes:
 *		- Implementation of detection algorithm (\ref CSpikeDetector and \ref COneChannelDetect)
 *		- Implementation GUI (\ref CMainFrame and \ref CSettingsDialog)
 * 		- Implementation of digital signal processing (\ref CDSP)
 * 			-# Filtering
 * 			-# Resampling
 * 			-# Calculationg of absolute value of Hilbert's transform
 *		- Implementation of results processing  (\ref CResultsModel)
 */

#ifndef CMain_H
#define	CMain_H

#include "wx/wx.h"

#include "CMainFrame.h"
#include "CInputModelEDF.h"
#include "Definitions.h"

/**
 * A main class representing the application.
 */
class CMain: public wxApp
{
	// variables
public:
private:
	/**
	 * a private variable.
	 * This variable representing GUI class.
	 */
	CMainFrame * m_frame;
	
	/**
	 * a private variable.
	 * This variable representing the class to work with input data for the detector.
	 */
	CInputModel     * m_model;

	// methods
public:
	/**
	 * A virtual member.
	 * This method is called when the application starts.
	 */
	virtual bool OnInit();
	
	/**
	 * A virtual member.
	 * This method is called when the application exits.
	 */
	virtual int  OnExit();
private:
};

#endif
