#ifndef CSettingsModel_H
#define CSettingsModel_H

#include "wx/wx.h"

//#include "lib/tinyxml/tinyxml.h"
#include "CException.h"

/**
 * Strucutre containing settings of the detector.
 */
typedef struct detectorSettings {
public: 
	int    m_band_low;                // (-fl)
	int    m_band_high;               // (-fh)
	double m_k1;               		  // (-k1)
	double m_k2;               		  // (-k2)
	double m_k3;					  // (-k3)
	int    m_winsize;     			  // (-w)
	double m_noverlap;     			  // (-n)
	int    m_buffering;               // (-buf)
	int    m_main_hum_freq;           // (-h)
	double m_discharge_tol;           // (-dt)
	double m_polyspike_union_time ;	  // (-pt)
	int    m_decimation;
	
	bool   m_original = true;

	/// A constructor
	detectorSettings(int band_low, int band_high, double k1, double k2, double k3, int winsize, double noverlap, int buffering, int main_hum_freq,
					 double discharge_tol, double polyspike_union_time, int decimation)
		: m_band_low(band_low), m_band_high(band_high), m_k1(k1), m_k2(k2), m_k3(k3), m_winsize(winsize), m_noverlap(noverlap), m_buffering(buffering),
		  m_main_hum_freq(main_hum_freq), m_discharge_tol(discharge_tol), m_polyspike_union_time(polyspike_union_time), m_decimation(decimation)
	{
		/* empty */
	}
	
} DETECTOR_SETTINGS;

/// Definition od setting type - default or saved (own saved settings)
enum SETTINGS_TYPE
{
	/// marker of default setting
	DEFAULT_SETTINGS = 1,
	/// marker of saved setting
	SAVED_SETTINGS = 2
};

/**
 * Class for manipulating with settings in file.
 * Loading and saving settings for the detector from XML.
 * Use TinyXML.
 */
//class CSettingsModel
//{
//	//methods
//public:
//	/**
//	 * A csontructor.
//	 * @param fileName a file name.
//	 */
//	CSettingsModel(const wxString& fileName);

//	/**
//	 * A virtual destrcutor.
//	 */
//	virtual ~CSettingsModel();
	
//	/**
//	 * Load settings from XML file, file must be named "settings.xml".
//	 * @param type flag of type settigns: 1 - default, 2 - saved
//	 * @return pointer to the DETECOR_SETTINGS strucutre containing settings of the detector
//	 */
//	DETECTOR_SETTINGS* LoadSettings(SETTINGS_TYPE type);

//	/**
//	 * Save settings to the XML file - "settings.xml".
//	 * @param settings pointer to the DETECOR_SETTINGS strucutre containing settings of the detector, which are saved
//	 */
//	void SaveSettings(DETECTOR_SETTINGS * settings);
//private:
//	/**
//	 * Return text value form XML element.
//	 * @param e a XML element
//	 * @return a string value
//	 */
//	const char* getTextValue(TiXmlElement* e) const;

//	/**
//	 * Set text value to the XML element.
//	 * @param e a XML element
//	 * @param val a integer value
//	 */
//	void setTextValue(TiXmlElement* e, const int& val) const;

//	/**
//	 * Set text value to the XML element.
//	 * @param e a XML element
//	 * @param val a double value
//	 */
//	void setTextValue(TiXmlElement* e, const double& val) const;

//	/**
//	 * Set text value to the XML element.
//	 * @param e a XML element
//	 * @param val a string value
//	 */
//	void setTextValue(TiXmlElement* e, const char * val) const;

//	// variables
//public:
//	/* none */
//private:
//	/// file name
//	wxString		    m_fileName;
	
//};

#endif
