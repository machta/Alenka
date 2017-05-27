#include "CResultsModel.h"

#include <matio.h>

/// A constructor.
CResultsModel::CResultsModel()
{
	/* empty */
}

/// A virtual destructor.
CResultsModel::~CResultsModel()
{
	/* empty */
}

/// Save results: @see #CDetectorOutput and @see #CDischarges to the XML file.
//void CResultsModel::SaveResultsXML(const wchar_t* fileName, const CDetectorOutput* out, const CDischarges* disch)
//{
//	char buffer[2048];
//	convertWCHARTtoCHAR(buffer, fileName);
//	SaveResultsXML(buffer, out, disch);
//}

/// Save results: @see #CDetectorOutput and @see #CDischarges to the XML file.
//void CResultsModel::SaveResultsXML(const char* fileName, const CDetectorOutput* out, const CDischarges* disch)
//{
//	TiXmlDocument     doc;
//	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
//	TiXmlElement*     root = new TiXmlElement("spikedetector");

//	doc.LinkEndChild(decl);
//	doc.LinkEndChild(root);
//	saveDetectorOutputXML(root, out);
//	saveDischargesXML(root, disch);

//	doc.SaveFile(fileName);
//}

/// Create XML struct from @see #CDetectorOutput and save it to the TiXmlDocument
//void CResultsModel::saveDetectorOutputXML(TiXmlElement*& root, const CDetectorOutput* out)
//{
//	unsigned countRecords = out->m_pos.size();
//	unsigned i;

//	TiXmlElement* subroot = new TiXmlElement("Output");
//	TiXmlElement* pos;
	
//	root->LinkEndChild(subroot);
//	for (i = 0; i < countRecords; i++)
//	{
//		pos = new TiXmlElement("Position");
//		subroot->LinkEndChild(pos);

//		pos->SetDoubleAttribute("pos", out->m_pos.at(i));
//		pos->SetAttribute("chan", out->m_chan.at(i));
//		pos->SetDoubleAttribute("dur", out->m_dur.at(i));
//		pos->SetDoubleAttribute("con", out->m_con.at(i));
//		pos->SetDoubleAttribute("weight", out->m_weight.at(i));
//		pos->SetDoubleAttribute("pdf", out->m_pdf.at(i));
//	}
//}

/// Create XML struct from @see #CDischarges and save it to the TiXmlDocument
//void CResultsModel::saveDischargesXML(TiXmlElement*& root, const CDischarges* disch)
//{
//	unsigned countRecords = disch->m_MP[0].size();
//	unsigned countChannels = disch->GetCountChannels();
//	unsigned i, j;

//	TiXmlElement* subroot = new TiXmlElement("Discharges");
//	TiXmlElement* elemDisch;
//	TiXmlElement* channel;

//	root->LinkEndChild(subroot);
//	for (i = 0; i < countChannels; i++)
//	{
//		channel = new TiXmlElement("Channel");
//		subroot->LinkEndChild(channel);
//		channel->SetAttribute("number", i);

//		for (j = 0; j < countRecords; j++)
//		{
//			elemDisch = new TiXmlElement("Disch");
//			channel->LinkEndChild(elemDisch);

//			elemDisch->SetAttribute("number", j);
//			elemDisch->SetDoubleAttribute("MV", disch->m_MV[i].at(j));
//			elemDisch->SetDoubleAttribute("MA", disch->m_MA[i].at(j));
//			elemDisch->SetDoubleAttribute("MP", disch->m_MP[i].at(j));
//			elemDisch->SetDoubleAttribute("MD", disch->m_MD[i].at(j));
//			elemDisch->SetDoubleAttribute("MW", disch->m_MW[i].at(j));
//			elemDisch->SetDoubleAttribute("MPDF", disch->m_MPDF[i].at(j));
//		}
//	}
//}

/// Load results: @see #CDetectorOutput and @see #CDischarges from XML file.
//void CResultsModel::LoadResultsXML(const wchar_t* fileName, CDetectorOutput*& out, CDischarges*& disch)
//{
//	char buffer[2048];
//	convertWCHARTtoCHAR(buffer, fileName);
//	LoadResultsXML(buffer, out, disch);
//}

/// Load results: @see #CDetectorOutput and @see #CDischarges from XML file.
//void CResultsModel::LoadResultsXML(const char* fileName, CDetectorOutput*& out, CDischarges*& disch)
//{
//	TiXmlDocument doc(fileName);
//	bool loadOk = doc.LoadFile();
//	TiXmlElement* pRoot;

//	if (!loadOk)
//		throw new CException(wxT("Error loading XML file with results!"), wxT("CResultsModel::LoadResultsXML"));

//	pRoot = doc.FirstChildElement("spikedetector");
//	if (!pRoot)
//		throw new CException(wxT("Cannot find the element \'spikedetector\'in input file!"), wxT("CResultsModel::LoadResultsXML"));

//	loadDetectorOutputXML(pRoot, out);
//	loadDischargesXML(pRoot, disch);
//}

/// Load data to @see #CDetectorOutput
//void CResultsModel::loadDetectorOutputXML(TiXmlElement*& root, CDetectorOutput*& out)
//{
//	TiXmlElement* 	 pOut, * pElem;
//	double  	  	 pos, dur, con, weight, pdf;
//	long    	  	 chan;
//	wxString 	  	 tmp;
//	bool             err = false;
	
//	CDetectorOutput* o = new CDetectorOutput();

//	pOut = root->FirstChildElement("Output");
//	pElem = pOut->FirstChildElement("Position");
//	while (pElem)
//	{
//		tmp = wxString::FromUTF8(pElem->Attribute("pos"));
//		if (!tmp.ToDouble(&pos))
//			err = true;
//		tmp = wxString::FromUTF8(pElem->Attribute("chan"));
//		if (!tmp.ToLong(&chan))
//			err = true;
//		tmp = wxString::FromUTF8(pElem->Attribute("dur"));
//		if (!tmp.ToDouble(&dur))
//			err = true;
//		tmp = wxString::FromUTF8(pElem->Attribute("con"));
//		if (!tmp.ToDouble(&con))
//			err = true;
//		tmp = wxString::FromUTF8(pElem->Attribute("weight"));
//		if (!tmp.ToDouble(&weight))
//			err = true;
//		tmp = wxString::FromUTF8(pElem->Attribute("pdf"));
//		if (!tmp.ToDouble(&pdf))
//			err = true;
		
//		if (err)
//		{
//			delete o;
//			throw new CException(wxT("Error loading data from file. Bad format of record!"), wxT("CResultsModel::loadDetectorOutput"));
//		}

//		o->Add(pos, dur, chan, con, weight, pdf);

//		pElem = pElem->NextSiblingElement("Position");
//	}

//	out = o;
//}

/// Load data to @see #CDischarges
//void CResultsModel::loadDischargesXML(TiXmlElement*& root, CDischarges*& disch)
//{
//	TiXmlElement* pDisch, * pElem, * pChannel;
//	unsigned 	  channels = 0, channel = 0;
//	double 		  MV, MA, MP, MD, MW, MPDF;
//	wxString 	  tmp;
//	CDischarges*  d;
//	bool 		  err = false;

//	pDisch = root->FirstChildElement("Discharges");
//	pChannel = pDisch->FirstChildElement("Channel");

//	// get count channels
//	while (pChannel)
//	{
//		channels++;
//		pChannel = pChannel->NextSiblingElement("Channel");
//	}

//	// load data
//	d = new CDischarges(channels);
//	pChannel = pDisch->FirstChildElement("Channel");
//	while (pChannel)
//	{
//		pElem = pChannel->FirstChildElement("Disch");
//		while (pElem)
//		{
//			tmp = wxString::FromUTF8(pElem->Attribute("MV"));
//			if (!tmp.ToDouble(&MV))
//				err = true;
//			tmp = wxString::FromUTF8(pElem->Attribute("MA"));
//			if (!tmp.ToDouble(&MA))
//				err = true;
//			tmp = wxString::FromUTF8(pElem->Attribute("MP"));
//			if (tmp.IsSameAs("nan"))
//				MP = NAN;
//			else if (!tmp.ToDouble(&MP))
//				err = true;
//			tmp = wxString::FromUTF8(pElem->Attribute("MD"));
//			if (!tmp.ToDouble(&MD))
//				err = true;
//			tmp = wxString::FromUTF8(pElem->Attribute("MW"));
//			if (!tmp.ToDouble(&MW))
//				err = true;
//			tmp = wxString::FromUTF8(pElem->Attribute("MPDF"));
//			if (!tmp.ToDouble(&MPDF))
//				err = true;

//			if (err)
//			{
//				delete d;
//				throw new CException(wxT("Error loading data from file. Bad format of record!"), wxT("CResultsModel::loadDischarges"));
//			}

//			d->m_MV[channel].push_back(MV);
//			d->m_MA[channel].push_back(MA);
//			d->m_MP[channel].push_back(MP);
//			d->m_MD[channel].push_back(MD);
//			d->m_MW[channel].push_back(MW);
//			d->m_MPDF[channel].push_back(MPDF);

//			pElem = pElem->NextSiblingElement("Disch");
//		}

//		channel++;
//		pChannel = pChannel->NextSiblingElement("Channel");
//	}

//	disch = d;
//}

/// Convert WCHAR_T to CHAR. Is using for converting path of files.
void CResultsModel::convertWCHARTtoCHAR(char out[2048], const wchar_t* in)
{
	mbstate_t mbs;
	int       ret;
	
	setlocale(LC_ALL, "");
	memset(&mbs, 0, sizeof(mbs));
	mbrlen(NULL , 0, &mbs);
	ret = wcsrtombs(out, &in, 2048, &mbs);
	setlocale(LC_ALL, "C");
	
	if(!ret)
		throw new CException(wxT("Error converting wchar_t* to char*. The file name or file path is not correct."), wxT("CResultsModel::convertWCHARTtoCHAR"));
}

/// Save results: @see #CDetectorOutput and @see #CDischarges to the MAT file.
void CResultsModel::SaveResultsMAT(const wchar_t* fileName, const CDetectorOutput* out, const CDischarges* disch)
{
	char buffer[2048];
	convertWCHARTtoCHAR(buffer, fileName);
	SaveResultsMAT(buffer, out, disch);
}

/// Save results: @see #CDetectorOutput and @see #CDischarges to the MAT file.
void CResultsModel::SaveResultsMAT(const char* fileName, const CDetectorOutput* out, const CDischarges* disch)
{
	mat_t*    matFile;

	matFile = Mat_CreateVer(fileName, NULL, MAT_FT_DEFAULT);
	if (NULL == matFile)
		throw new CException(wxT("Error opening / creating MAT file!"), wxT("CResultsModel::SaveResultsMAT"));

	saveDetectorOutputMAT(matFile, out);
	saveDischargesMAT(matFile, disch);

	Mat_Close(matFile);
}

/// Save @see #CDetectorOutput to the MAT file.
void CResultsModel::saveDetectorOutputMAT(mat_t*& matFile, const CDetectorOutput* out)
{
	matvar_t* 	matOut;
	matvar_t* 	matField;
	int       	countSamples = out->m_pos.size();
	int 	  	i;
	double*   	pos = new double[countSamples];
	double*  	dur = new double[countSamples];
	double*   	chan = new double[countSamples];
	double*   	con = new double[countSamples];
	double*   	weight = new double[countSamples];
	double*   	pdf = new double[countSamples];

	const char* fieldsname[6] = {"pos","dur", "chan", "con", "weight", "pdf"};
	int         nfields = 6;
	size_t      dims[2] = {1, 1};

	matOut = Mat_VarCreateStruct("out", 2, dims, fieldsname, nfields);

	dims[0] = countSamples; dims[1] = 1;

	for (i = 0; i < countSamples; i++)
	{
		pos[i] = out->m_pos.at(i);
		dur[i] = out->m_dur.at(i);
		chan[i] = out->m_chan.at(i);
		con[i] = out->m_con.at(i);
		weight[i] = out->m_weight.at(i);
		pdf[i] = out->m_pdf.at(i);
	}
	
	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, pos, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matOut, "pos", 0, matField);

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, dur, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matOut, "dur", 0, matField);

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, chan, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matOut, "chan", 0, matField);

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, con, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matOut, "con", 0, matField);

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, weight, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matOut, "weight", 0, matField);

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, pdf, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matOut, "pdf", 0, matField);

	Mat_VarWrite(matFile, matOut, MAT_COMPRESSION_NONE);
	Mat_VarFree(matOut);

	delete [] pos;
	delete [] dur;
	delete [] chan;
	delete [] con;
	delete [] weight;
	delete [] pdf;
}

/// Save @see #CDischarges to the MAT file.
void CResultsModel::saveDischargesMAT(mat_t*& matFile, const CDischarges* disch)
{
	matvar_t* 	matDisch;
	matvar_t* 	matField;
	unsigned    countSamples = disch->m_MP[0].size();
	unsigned 	countChannels = disch->GetCountChannels();
	unsigned    i, j, pointer = 0;
	unsigned    samples = countChannels * countSamples;

	double* 	MV = new double[samples];
	double* 	MA = new double[samples];
	double* 	MP = new double[samples];
	double* 	MD = new double[samples];
	double* 	MW = new double[samples];
	double* 	MPDF = new double[samples];

	const char* fieldsname[6] = {"MV","MA", "MP", "MD", "MW", "MPDF"};
	int         nfields = 6;
	size_t      dims[2];

	for (i = 0; i < countChannels; i++)
	{
		for (j = 0; j < countSamples; j++)
		{
			MV[pointer] = disch->m_MV[i].at(j);
			MA[pointer] = disch->m_MA[i].at(j);
			MP[pointer] = disch->m_MP[i].at(j);
			MD[pointer] = disch->m_MD[i].at(j);
			MW[pointer] = disch->m_MW[i].at(j);
			MPDF[pointer] = disch->m_MPDF[i].at(j);

			pointer++;
		}
	}

	dims[0] = 1; dims[1] = 1;
	matDisch = Mat_VarCreateStruct("discharges", 2, dims, fieldsname, nfields);

	dims[0] = countSamples; dims[1] = countChannels;

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, MV, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matDisch, "MV", 0, matField);

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, MA, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matDisch, "MA", 0, matField);

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, MP, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matDisch, "MP", 0, matField);

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, MD, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matDisch, "MD", 0, matField);

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, MW, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matDisch, "MW", 0, matField);

	matField = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, MPDF, MAT_F_DONT_COPY_DATA);
	Mat_VarSetStructFieldByName(matDisch, "MPDF", 0, matField);

	Mat_VarWrite(matFile, matDisch, MAT_COMPRESSION_NONE);
	Mat_VarFree(matDisch);

	delete [] MV;
	delete [] MA;
	delete [] MP;
	delete [] MD;
	delete [] MW;
	delete [] MPDF;
}

/// Load results: @see #CDetectorOutput and @see #CDischarges from XML file.
void CResultsModel::LoadResultsMAT(const wchar_t* fileName, CDetectorOutput*& out, CDischarges*& disch)
{
	char buffer[2048];
	convertWCHARTtoCHAR(buffer, fileName);
	LoadResultsMAT(buffer, out, disch);
}

/// Load results: @see #CDetectorOutput and @see #CDischarges from XML file.
void CResultsModel::LoadResultsMAT(const char* fileName, CDetectorOutput*& out, CDischarges*& disch)
{
	mat_t*    		 matFile;
	bool 			 err = false;

	matFile = Mat_Open(fileName, MAT_ACC_RDONLY);
	if (NULL != matFile)
	{
		loadDetectorOutputMAT(matFile, out);
		loadDischargesMAT(matFile, disch);

		if (out == NULL || disch == NULL)
			err = true;

		Mat_Close(matFile);
	}

	if (err)
		throw new CException(wxString::Format(wxT("Error opening MAT file: %s"), fileName), wxT("CResultsModel::LoadResultsMAT"));
}

/// Load data to @see #CDetectorOutput
void CResultsModel::loadDetectorOutputMAT(mat_t*& matFile, CDetectorOutput*& out)
{
	matvar_t* matStruct,* matVar;
	double*   data;
	int   	  start[2] = {0, 0};
	int 	  stride[2] = {1, 1};
	int 	  edge[2];

	out = new CDetectorOutput();
	try
	{
		matStruct = Mat_VarReadInfo(matFile, "out");
		if (!matStruct)
			throw 0;

		// load pos
		matVar = Mat_VarGetStructFieldByName(matStruct, "pos", 0);
		if (matVar->dims[0] < 1 || matVar->dims[1] != 1)
			throw 1;
		data = new double[matVar->dims[0]];
		edge[0] = matVar->dims[0];
		edge[1] = matVar->dims[1];
		Mat_VarReadData(matFile, matVar, data, start, stride, edge);
		out->m_pos.assign(data, data + matVar->dims[0]);
		Mat_VarFree(matVar);

		// load dur
		matVar = Mat_VarGetStructFieldByName(matStruct, "dur", 0);
		if (matVar->dims[0] != (unsigned)edge[0] || matVar->dims[1] != (unsigned)edge[1])
			throw 2;
		Mat_VarReadData(matFile, matVar, data, start, stride, edge);
		out->m_dur.assign(data, data + matVar->dims[0]);
		Mat_VarFree(matVar);

		// load chan
		matVar = Mat_VarGetStructFieldByName(matStruct, "chan", 0);
		if (matVar->dims[0] != (unsigned)edge[0] || matVar->dims[1] != (unsigned)edge[1])
			throw 2;
		Mat_VarReadData(matFile, matVar, data, start, stride, edge);
		out->m_chan.assign(data, data + matVar->dims[0]);
		Mat_VarFree(matVar);

		// load con
		matVar = Mat_VarGetStructFieldByName(matStruct, "con", 0);
		if (matVar->dims[0] != (unsigned)edge[0] || matVar->dims[1] != (unsigned)edge[1])
			throw 2;
		Mat_VarReadData(matFile, matVar, data, start, stride, edge);
		out->m_con.assign(data, data + matVar->dims[0]);
		Mat_VarFree(matVar);

		// load weight
		matVar = Mat_VarGetStructFieldByName(matStruct, "weight", 0);
		if (matVar->dims[0] != (unsigned)edge[0] || matVar->dims[1] != (unsigned)edge[1])
			throw 2;
		Mat_VarReadData(matFile, matVar, data, start, stride, edge);
		out->m_weight.assign(data, data + matVar->dims[0]);
		Mat_VarFree(matVar);

		// load pdf
		matVar = Mat_VarGetStructFieldByName(matStruct, "pdf", 0);
		if (matVar->dims[0] != (unsigned)edge[0] || matVar->dims[1] != (unsigned)edge[1])
			throw 2;
		Mat_VarReadData(matFile, matVar, data, start, stride, edge);
		out->m_pdf.assign(data, data + matVar->dims[0]);
		Mat_VarFree(matVar);

		delete [] data;
		return;
	}
	catch(int e)
	{
		if (e != 0)
			Mat_VarFree(matVar);
		
		if (e == 2)
			delete [] data;
		
		delete out;
		out = NULL;
	}
}

/// Load data to @see #CDischarges
void CResultsModel::loadDischargesMAT(mat_t*& matFile, CDischarges*& disch)
{
	matvar_t* matStruct,* matVar;
	double*   MV, * MA, * MP, * MD, * MW, * MPDF;
	int   	  start[2] = {0, 0};
	int 	  stride[2] = {1, 1};
	int 	  edge[2];
	int       i;

	int 	  begin, end;

	try
	{
		matStruct = Mat_VarReadInfo(matFile, "discharges");
		if (!matStruct)
			throw 0;

		// load MP
		matVar = Mat_VarGetStructFieldByName(matStruct, "MP", 0);
		if (matVar->dims[0] < 1 || matVar->dims[1] < 1)
			throw 1;
		// get count elems
		edge[0] = matVar->dims[0];
		edge[1] = matVar->dims[1];

		// alocate memory
		MP = new double[edge[0] * edge[1]];
		MV = new double[edge[0] * edge[1]];
		MA = new double[edge[0] * edge[1]];
		MD = new double[edge[0] * edge[1]];
		MW = new double[edge[0] * edge[1]];
		MPDF = new double[edge[0] * edge[1]];
		disch = new CDischarges(edge[1]);

		Mat_VarReadData(matFile, matVar, MP, start, stride, edge);
		Mat_VarFree(matVar);

		// load MV
		matVar = Mat_VarGetStructFieldByName(matStruct, "MV", 0);
		if (matVar->dims[0] != (unsigned)edge[0] || matVar->dims[1] != (unsigned)edge[1])
			throw 2;
		Mat_VarReadData(matFile, matVar, MV, start, stride, edge);
		Mat_VarFree(matVar);

		// load MA
		matVar = Mat_VarGetStructFieldByName(matStruct, "MA", 0);
		if (matVar->dims[0] != (unsigned)edge[0] || matVar->dims[1] != (unsigned)edge[1])
			throw 2;
		Mat_VarReadData(matFile, matVar, MA, start, stride, edge);
		Mat_VarFree(matVar);

		// load MD
		matVar = Mat_VarGetStructFieldByName(matStruct, "MD", 0);
		if (matVar->dims[0] != (unsigned)edge[0] || matVar->dims[1] != (unsigned)edge[1])
			throw 2;
		Mat_VarReadData(matFile, matVar, MD, start, stride, edge);
		Mat_VarFree(matVar);

		// load MW
		matVar = Mat_VarGetStructFieldByName(matStruct, "MW", 0);
		if (matVar->dims[0] != (unsigned)edge[0] || matVar->dims[1] != (unsigned)edge[1])
			throw 2;
		Mat_VarReadData(matFile, matVar, MW, start, stride, edge);
		Mat_VarFree(matVar);

		// load MPDF
		matVar = Mat_VarGetStructFieldByName(matStruct, "MPDF", 0);
		if (matVar->dims[0] != (unsigned)edge[0] || matVar->dims[1] != (unsigned)edge[1])
			throw 2;
		Mat_VarReadData(matFile, matVar, MPDF, start, stride, edge);
		Mat_VarFree(matVar);

		// map loaded data to output class
		begin = 0;
		end = edge[0];
		for (i = 0; i < edge[1]; i++)
		{
			disch->m_MP[i].assign(MP + begin, MP + end);
			disch->m_MV[i].assign(MV + begin, MV + end);
			disch->m_MA[i].assign(MA + begin, MA + end);
			disch->m_MD[i].assign(MD + begin, MD + end);
			disch->m_MW[i].assign(MW + begin, MW + end);
			disch->m_MPDF[i].assign(MPDF + begin, MPDF + end);
			begin = end;
			end += edge[0];
		}

		delete [] MP;
		delete [] MV;
		delete [] MA;
		delete [] MD;
		delete [] MW;
		delete [] MPDF;
	}
	catch (int e)
	{
		if (e == 0)
			return;

		Mat_VarFree(matVar);
		delete [] MP;
		delete [] MV;
		delete [] MA;
		delete [] MD;
		delete [] MW;
		delete [] MPDF;
		
		delete disch;
		disch = NULL;
	}
}

// temporary functions for testing
void CResultsModel::SaveVectorMAT(const char* fileName, const char* vectorName, const wxVector<float>& data)
{
	wxVector<double> tmp(data.begin(), data.end());
	SaveVectorMAT(fileName, vectorName, tmp);
}

void CResultsModel::SaveVectorMAT(const char* fileName, const char* vectorName, const wxVector<double>& data)
{
	mat_t*    matFile;
	matvar_t* matField;
	size_t    dims[2] = {1, data.size()};
	double*   tmp = new double[data.size()];
	unsigned  i;

	for (i = 0; i < data.size(); i++)
		tmp[i] = data.at(i);

	matFile = Mat_CreateVer(fileName, NULL, MAT_FT_DEFAULT);
	if (NULL == matFile)
		throw new CException(wxT("Error opening / creating MAT file!"), wxT("CResultsModel::SaveMatrixMAT"));

	matField = Mat_VarCreate(vectorName, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, tmp, MAT_F_DONT_COPY_DATA);
	Mat_VarWrite(matFile, matField, MAT_COMPRESSION_NONE);
	Mat_VarFree(matField);
	Mat_Close(matFile);

	delete [] tmp;
}

