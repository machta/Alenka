#ifndef CResultsModel_H
#define	CResultsModel_H

#include "CSpikeDetector.h"
//#include "lib/tinyxml/tinyxml.h"

typedef struct _mat_t mat_t;

/**
 * Static class for saving and loading results of the detector.
 * The results are saved in the XML files.
 */
class CResultsModel
{
	// methods
public:
	/**
	 * A constructor.
	 */
	CResultsModel();

	/**
	 * A virtual destructor.
	 */
	virtual ~CResultsModel();

	/**
	 * Save results: \ref CDetectorOutput and \ref CDischarges to the XML file.
	 * @param fileName file name as char array.
	 * @param out pointer to \ref CDetectorOutput
	 * @param disch pointer to \ref CDischarges
	 */
//	static void SaveResultsXML(const char* fileName, const CDetectorOutput* out, const CDischarges* disch);
	
	/**
	 * Save results: \ref CDetectorOutput and \ref CDischarges to the XML file.
	 * It convert fileName from wchar_t* to char* and call CResultsModel::SaveResultsXML(const char*, const CDetectorOutput*, const CDischarges*)
	 * @param fileName file name as wchar_t array.
	 * @param out input data - pointer to \ref CDetectorOutput
	 * @param disch input data - pointer to \ref CDischarges
	 */
//	static void SaveResultsXML(const wchar_t* fileName, const CDetectorOutput* out, const CDischarges* disch);

	/**
	 * Load results: \ref CDetectorOutput and \ref CDischarges from XML file.
	 * @param fileName file name as char array.
	 * @param out output pointer where is saved \ref CDetectorOutput
	 * @param disch output pointer where is saved \ref CDischarges
	 */
//	static void LoadResultsXML(const char* fileName, CDetectorOutput*& out, CDischarges*& disch);
	
	/**
	 * Load results: \ref CDetectorOutput and \ref CDischarges from XML file.
	 * It convert fileName from wchar_t* to char* and call CResultsModel::SaveResultsXML(const char*, const CDetectorOutput*, const CDischarges*)
	 * @param fileName file name as wchar_t array.
	 * @param out output pointer where is saved \ref CDetectorOutput
	 * @param disch output pointer where is saved \ref CDischarges
	 */
//	static void LoadResultsXML(const wchar_t* fileName, CDetectorOutput*& out, CDischarges*& disch);

	/**
	 * Save results: \ref CDetectorOutput and \ref CDischarges to the MAT file.
	 * @param fileName file name as char array.
	 * @param out pointer to \ref CDetectorOutput
	 * @param disch pointer to \ref CDischarges
	 */
	static void SaveResultsMAT(const char* fileName, const CDetectorOutput* out, const CDischarges* disch);
	
	/**
	 * Save results: \ref CDetectorOutput and \ref CDischarges to the MAT file.
	 * It convert fileName from wchar_t* to char* and call CResultsModel::SaveResultsMAT(const char*, const CDetectorOutput*, const CDischarges*)
	 * @param fileName file name as wchar_t array.
	 * @param out input data - pointer to \ref CDetectorOutput
	 * @param disch input data - pointer to \ref CDischarges
	 */
	static void SaveResultsMAT(const wchar_t* fileName, const CDetectorOutput* out, const CDischarges* disch);

	/**
	 * Load results: \ref CDetectorOutput and \ref CDischarges from MAT file.
	 * @param fileName file name as char array.
	 * @param out output pointer where is saved \ref CDetectorOutput
	 * @param disch output pointer where is saved \ref CDischarges
	 */
	static void LoadResultsMAT(const char* fileName, CDetectorOutput*& out, CDischarges*& disch);
	
	/**
	 * Load results:\ref CDetectorOutput and \ref CDischarges from MAT file.
	 * It convert fileName from wchar_t* to char* and call CResultsModel::SaveResultsMAT(const char*, const CDetectorOutput*, const CDischarges*)
	 * @param fileName file name as wchar_t array.
	 * @param out output pointer where is saved \ref CDetectorOutput
	 * @param disch output pointer where is saved \ref CDischarges
	 */
	static void LoadResultsMAT(const wchar_t* fileName, CDetectorOutput*& out, CDischarges*& disch);

	// temporary functions for testing
	static void SaveVectorMAT(const char* fileName, const char* vectorName, const wxVector<float>& data);
	static void SaveVectorMAT(const char* fileName, const char* vectorName, const wxVector<double>& data);
private:
	/**
	 * Create XML struct from \ref CDetectorOutput and save it to the TiXmlDocument
	 * @param root TiXmlElement root elem in which si connect output struct.
	 * @param out input data - pointer to \ref CDetectorOutput
	 */
//	static void saveDetectorOutputXML(TiXmlElement*& root, const CDetectorOutput* out);
	
	/**
	 * Create XML struct from \ref CDischarges and save it to the TiXmlDocument
	 * @param root TiXmlElement root elem in which si connect output struct.
	 * @param disch data - pointer to \ref CDischarges
	 */
//	static void saveDischargesXML(TiXmlElement*& root, const CDischarges* disch);

	/**
	 * Convert WCHAR_T to CHAR. Is using for converting path of files.
	 * @param out ouput char array
	 * @param in input wchar_t array
	 */
	static void convertWCHARTtoCHAR(char* out, const wchar_t* in);

	/**
	 * Load data to \ref CDetectorOutput
	 * @param root TiXmlElement root elem elem where are find data.
	 * @param out output pointer to \ref CDetectorOutput
	 */
//	static void loadDetectorOutputXML(TiXmlElement*& root, CDetectorOutput*& out);
	
	/**
	 * Load data to \ref CDischarges
	 * @param root TiXmlElement root elem where are find data.
	 * @param disch output pointer to \ref CDischarges
	 */
//	static void loadDischargesXML(TiXmlElement*& root, CDischarges*& disch);

	/**
	 * Save \ref CDetectorOutput to the MAT file.
	 * @param matFile a pointer to parent mat_t to which is saved strcuture
	 * @param out the input class \ref CDetectorOutput
	 */
	static void saveDetectorOutputMAT(mat_t*& matFile, const CDetectorOutput* out);
	
	/**
	 * Save \ref CDischarges to the MAT file.
	 * @param matFile a pointer to parent mat_t to which is saved strcuture
	 * @param out the input class \ref CDischarges
	 */
	static void saveDischargesMAT(mat_t*& matFile, const CDischarges* disch);

	/**
	 * Load data to \ref CDetectorOutput
	 * @param matFile pointer to mat_t representig input file.
	 * @param out output pointer to \ref CDetectorOutput
	 */
	static void loadDetectorOutputMAT(mat_t*& matFile, CDetectorOutput*& out);
	
	/**
	 * Load data to \ref CDischarges
	 * @param matFile pointer to mat_t representig input file.
	 * @param disch output pointer to \ref CDischarges
	 */
	static void loadDischargesMAT(mat_t*& matFile, CDischarges*& disch);

	// variables - none
};

#endif
