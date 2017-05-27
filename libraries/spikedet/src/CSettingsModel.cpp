#include "CSettingsModel.h"

#include <iostream>

using namespace std;

/// A constructor
CSettingsModel::CSettingsModel(const wxString& fileName)
{
	m_fileName = fileName;
}

/// A destructor
CSettingsModel::~CSettingsModel()
{
	/* empty */
	//delete m_settings;
}

/// Load settings from XML file, file must be named "settings.xml"
DETECTOR_SETTINGS * CSettingsModel::LoadSettings(SETTINGS_TYPE type)
{
	TiXmlElement *pRoot, *pSettings, *pElem;
	TiXmlDocument doc(m_fileName.mb_str(wxConvUTF8));
	
	wxString inputType = wxString::Format(wxT("%i"), type);
	
	bool loadOk = doc.LoadFile();

	if (loadOk)
		// loading file is ok
	{
		double k1, k2, k3, noverlap, discharge_tol, polyspike_union_time;
		long   band_low, band_high, winsize, buffering, main_hum_freq, decimation;
		
		pRoot = doc.FirstChildElement("spikedetector");
		if (pRoot)
		{
			pSettings = pRoot->FirstChildElement("settings");
			while (pSettings)
			{
				if (wxStricmp(pSettings->Attribute("type"), inputType.mb_str()) == 0)
				{
					wxString bl, bh, sk1, sk2, sk3, ws, nl, bf, mhf, dt, put, dec;
					
					pElem = pSettings->FirstChildElement("band_low");
					bl = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("band_high");
					bh = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("k1");
					sk1 = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("k2");
					sk2 = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("k3");
					sk3 = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("winsize");
					ws = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("noverlap");
					nl = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("buffering");
					bf = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("main_hum_freq");
					mhf = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("discharge_tol");
					dt = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("polyspike_union_time");
					put = wxString::FromUTF8(getTextValue(pElem));
					pElem = pSettings->FirstChildElement("decimation");
					dec = wxString::FromUTF8(getTextValue(pElem));
					
					if (!bl.ToLong(&band_low) || !bh.ToLong(&band_high) || !sk1.ToDouble(&k1) || !sk2.ToDouble(&k2) || !sk3.ToDouble(&k3) || !ws.ToLong(&winsize) ||
						!nl.ToDouble(&noverlap) || !bf.ToLong(&buffering) || !mhf.ToLong(&main_hum_freq) || !dt.ToDouble(&discharge_tol) ||
						!put.ToDouble(&polyspike_union_time) || !dec.ToLong(&decimation))
						throw new CException(wxT("Error loading settings data: bad format."), wxT("CSettingsModel::LoadSettings"));

					break;
				}
				pSettings = pSettings->NextSiblingElement("settings");
			}
			
			return new DETECTOR_SETTINGS(band_low, band_high,  k1, k2, k3, winsize, noverlap, buffering, main_hum_freq, discharge_tol,
										 polyspike_union_time, decimation);
		}
		else
			// error
		{
			throw new CException(wxT("Cannot find the element \'spikedetector\'in settings file!"), wxT("CSettingsModel::LoadSettings"));
		}
	}
	
	throw new CException(wxT("Cannot open the settings.xml file!"), wxT("CSettingsModel::LoadSettings"));
}

/// Return text value form XML element
const char * CSettingsModel::getTextValue(TiXmlElement * e) const
{
	TiXmlNode * first = e->FirstChild();
	if (first != 0 && first == e->LastChild() && first->Type() == TiXmlNode::TINYXML_TEXT)
	{
		// return the child's
		return first->Value();
	}
	
	throw new CException(wxT("Bad XML element."), wxT("CSettingsModel::getTextValue"));
}

/// Set text value to the XML element
void CSettingsModel::setTextValue(TiXmlElement* e, const int& val) const
{
	wxString stringVal = wxString::Format(wxT("%i"), val);
	setTextValue(e, stringVal.mb_str());
}

/// Set text value to the XML element
void CSettingsModel::setTextValue(TiXmlElement* e, const double& val) const
{
	wxString stringVal = wxString::Format(wxT("%f"), val);
	setTextValue(e, stringVal.mb_str());
}

/// Set text value to the XML element
void CSettingsModel::setTextValue(TiXmlElement* e, const char * val) const
{
	TiXmlNode * first = e->FirstChild();
	if (first != 0 && first == e->LastChild() && first->Type() == TiXmlNode::TINYXML_TEXT)
	{
		first->SetValue(val);
	}
	else throw new CException(wxT("Bad XML element."), wxT("CSettingsModel::getTextValue"));
}

/// Save settings to the XML file - "settings.xml"
void CSettingsModel::SaveSettings(DETECTOR_SETTINGS * settings)
{
	TiXmlElement *pRoot, *pSettings, *pElem;
	TiXmlDocument doc(m_fileName.mb_str(wxConvUTF8));
	bool loadOk = doc.LoadFile();

	if (loadOk)
		// loading file is ok
	{
		wxString type = wxString::Format(wxT("%i"), SETTINGS_TYPE::SAVED_SETTINGS);
		
		pRoot = doc.FirstChildElement("spikedetector");
		if (pRoot)
		{
			pSettings = pRoot->FirstChildElement("settings");
			while (pSettings)
			{
				if (wxStricmp(pSettings->Attribute("type"), type.mb_str()) == 0)
				{
					wxString bl, bh, sk1, sk2, ws, nl, bf, mhf, dt, put;
					
					pElem = pSettings->FirstChildElement("band_low");
					setTextValue(pElem, settings->m_band_low);

					pElem = pSettings->FirstChildElement("band_high");
					setTextValue(pElem, settings->m_band_high);
					
					pElem = pSettings->FirstChildElement("k1");
					setTextValue(pElem, settings->m_k1);
					
					pElem = pSettings->FirstChildElement("k2");
					setTextValue(pElem, settings->m_k2);
					
					pElem = pSettings->FirstChildElement("winsize");
					setTextValue(pElem, settings->m_winsize);
					
					pElem = pSettings->FirstChildElement("noverlap");
					setTextValue(pElem, settings->m_noverlap);
					
					pElem = pSettings->FirstChildElement("buffering");
					setTextValue(pElem, settings->m_buffering);
					
					pElem = pSettings->FirstChildElement("main_hum_freq");
					setTextValue(pElem, settings->m_main_hum_freq);
					
					pElem = pSettings->FirstChildElement("discharge_tol");
					setTextValue(pElem, settings->m_discharge_tol);
					
					pElem = pSettings->FirstChildElement("polyspike_union_time");
					setTextValue(pElem, settings->m_polyspike_union_time);

					pElem = pSettings->FirstChildElement("decimation");
					setTextValue(pElem, settings->m_decimation);
					
					doc.SaveFile(m_fileName.mb_str(wxConvUTF8));
					return;
				}
				pSettings = pSettings->NextSiblingElement("settings");
			}
		}
		else
			// error
		{
			throw new CException(wxT("Cannot find the element \'spikedetector\'in settings file!"), wxT("CSettingsModel::LoadSettings"));
		}
	}
	
	throw new CException(wxT("Cannot open the settings.xml file!"), wxT("CSettingsModel::LoadSettings"));
}
