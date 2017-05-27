#include "CDSP.h"

#include <samplerate.h>
#include <Eigen/Dense>
#include <fasttransforms.h>
#include <resample.h>

using namespace Eigen;
using namespace std;

// Digital signal resampling ----------------------------------------------------------------------
/// Method for digital signal resampling - In this program is used for decimating.
void CDSP::Resample(wxVector<SIGNALTYPE>*& data, const int& countChannels, const int& actualFS, const int& requiredFS, bool original)
{
	int i, j, outputSize;
	double val;
	CResamplingThread** threads = new CResamplingThread*[countChannels];
	int expectedOutputSize = ceil(data[0].size() * (double)requiredFS/(double)actualFS);

	// run threads
	for (i = 0; i < countChannels; i++)
	{
		threads[i] = new CResamplingThread(&data[i], actualFS, requiredFS, original);
		threads[i]->Run();
	}

	// wait until they works
	for (i = 0; i < countChannels; i++)
	{
		threads[i]->Wait();
		delete threads[i];
	}

	// check if outputs have expected size, if not, add elems
	for (i = 0; i < countChannels; i++)
	{
		outputSize = data[i].size();
		if (outputSize < expectedOutputSize)
		{
			val = data[i].back();

			for (j = 0; j < expectedOutputSize - outputSize; j++)
			{
				data[i].push_back(val);
			}
		}
		else if (outputSize > expectedOutputSize)
		{
			for (j = 0; j < outputSize - expectedOutputSize; j++)
			{
				data[i].pop_back();
			}
		}
	}

	delete [] threads;
}

// --------------------------
// CResampling
// --------------------------
/// A constructor.
CResamplingThread::CResamplingThread(wxVector<SIGNALTYPE>* data, const int& actFS, const int& requiredFS, bool original)
	: wxThread(wxTHREAD_JOINABLE)
{
	m_actFS = actFS;
	m_requiredFS = requiredFS;
	m_data = data;
	m_original = original;
}

/// A virtual destructor.
CResamplingThread::~CResamplingThread()
{
	/* empty */
}

/// This is the entry point of the thread.
wxThread::ExitCode CResamplingThread::Entry()
{
	if (!m_original)
	{
		vector<double> m_data_double(m_data->begin(), m_data->end()), output;
		resample(m_requiredFS, m_actFS, m_data_double, output);
		m_data->assign(output.begin(), output.end());
		return 0;
	}

	int err, ret;
	float * out = new float[m_data->size()];

	SRC_STATE* state = src_new(SRC_SINC_BEST_QUALITY, 1, &err);
	SRC_DATA*  src_data = new SRC_DATA();

	vector<float> m_data_float(m_data->begin(), m_data->end());
	src_data->data_in = m_data_float.data();
	src_data->data_out = out;
	src_data->input_frames = m_data->size();
	src_data->output_frames = m_data->size();
	src_data->src_ratio = (float)m_requiredFS / (float)m_actFS;
	src_data->end_of_input = m_data->size();

	ret = src_process(state, src_data);

	src_delete(state);
	state = NULL;

	m_data->clear();
	m_data->assign(out, out + src_data->output_frames_gen);

	delete [] out;
	delete src_data;

	return (ExitCode)ret;
}


// Hilbert transform ----------------------------------------------------------------------------------

/// Calculation of the absolute values of the Hilbert transform
void CDSP::AbsHilbert(wxVector<SIGNALTYPE>& data)
{
	int i, sizeInput;
	alglib::complex_1d_array in;

	// create complex array
	sizeInput = data.size();
	in.setlength(sizeInput);

	for (i = 0; i < sizeInput; i++)
	{
		in[i].x = data.at(i);
		in[i].y = 0;
	}

	// FFT
	alglib::fftc1d(in);

	// H
	wxVector<int> h(data.size(), 0);
	if (2*floor(sizeInput/2) == sizeInput)
	{
		// even
		h.at(0) = 1;
		h.at(sizeInput/2) = 1;
		for (i = 1; i < sizeInput/2; i++)
			h.at(i) = 2;
	}
	else
	{
		// odd
		h.at(0) = 1;
		for (i = 1; i < (sizeInput+1)/2; i++)
			h.at(i) = 2;
	}

	// IFFT
	for (i = 0; i < sizeInput; i++)
	{
		in[i].x *= h[i];
		in[i].y *= h[i];
	}

	// IFFT
	alglib::fftc1dinv(in);

	// Absolute value
	for (i = 0; i < sizeInput; i++)
	{
		complex<SIGNALTYPE> tmp(in[i].x, in[i].y);
		data.at(i) = abs(tmp);
	}
}

// Filtering ------------------------------------------------------------------------------------------

/// Digital signal filtering 10-60Hz
void CDSP::Filtering(wxVector<SIGNALTYPE>* data, const int& countChannels, const int& fs, const BANDWIDTH& bandwidth)
{
	int 			 i;
	wxVector<double> a, b;
	CFiltFilt** 	 threads = new CFiltFilt*[countChannels];

	double           Wp, Ws, Rp = 6, Rs = 60;
	int              order;

	CException*      e = NULL;

	// Precomputation values - Chebyshev II, for band low = 10, band high = 60
	// high
	const double bh[7] = {0.609690631014657, -3.63559011319046, 9.05535356429358, -12.0589078771451, 9.05535356429359, -3.63559011319046,
						  0.609690631014657};
	const double ah[7] = {1, -4.98146241953972, 10.4387837885612, -11.7670343111137, 7.51972533817281, -2.58144797118803, 0.371722665567175};

	// low
	const double bl[9] = {0.0756582074206937, 0.473325643095456, 1.40026011018072, 2.53803955902203, 3.07132128851322, 2.53803955902203, 1.40026011018072,
						  0.473325643095457, 0.0756582074206939};
	const double al[9] = {1, 2.04764667081498, 3.02818564408939, 2.79774379822106, 1.90643217355482, 0.903943414012386, 0.296359133636092, 0.0598505403817098,
						  0.00572695324055361};


	// HIGH pass filtering
	if (bandwidth.m_bandLow != 10 || bandwidth.m_bandHigh != 60)
	{
		Wp = 2*bandwidth.m_bandLow/(double)fs;
		Ws = (2*bandwidth.m_bandLow/(double)fs)-0.05;
		if (Ws < 0) Ws = 0.05;

		// calc filter order
		Buttord(Wp, Ws, Rp, Rs, order);

		// if order is odd + 1
		if (order % 2 != 0)
			order ++;

		// filter design
		if (!Butter(b, a, order, Ws, HIGHPASS))
			throw new CException(wxT("Error calculating filter design for HIGHPASS filter!"), wxT("CDSP::Filtering"));
	}
	else
	{
		a.assign(ah, ah+7);
		b.assign(bh, bh+7);
	}

	// run filtering
	for (i = 0; i < countChannels; i++)
	{
		threads[i] = new CFiltFilt(b, a, &data[i]);
		threads[i]->Run();
	}
	// wait until they work
	for (i = 0; i < countChannels; i++)
	{
		threads[i]->Wait();
		delete threads[i];
	}

	if (bandwidth.m_bandHigh == fs/2)
	{
		delete [] threads;
		return;
	}

	// LOW pass filtering
	if (bandwidth.m_bandLow != 10 || bandwidth.m_bandHigh != 60)
	{
		Wp = 2*bandwidth.m_bandHigh/(double)fs;
		Ws = (2*bandwidth.m_bandHigh/(double)fs)+0.1;
		if (Ws > 1) Ws = 1;

		// calc filter order
		Buttord(Wp, Ws, Rp, Rs, order);

		// if order is odd + 1
		if (order % 2 != 0)
			order ++;

		// filter design
		if (!Butter(b, a, order, Ws, LOWPASS))
			throw new CException(wxT("Error calculating filter design for HIGHPASS filter!"), wxT("CDSP::Filtering"));
	}
	else
	{
		a.assign(al, al+9);
		b.assign(bl, bl+9);
	}

	for (i = 0; i < countChannels; i++)
	{
		threads[i] = new CFiltFilt(b, a, &data[i]);
		threads[i]->Run();
	}
	// wait until they work
	for (i = 0; i < countChannels; i++)
	{
		e = (CException*)threads[i]->Wait();
		delete threads[i];

		if (e != NULL)
			throw e;
	}

	delete [] threads;
}

/// Digital signal filtering Nx50hz
void CDSP::Filt50Hz(wxVector<SIGNALTYPE>* data, const int& countChannels, const int& fs, const int& hum_fs, const BANDWIDTH& bandwidth)
{
	double 			 R = 1, r = 0.985, tmp, i;
	wxVector<int> 	 f0;
	wxVector<double> b, a;

	for (i = hum_fs; i <= fs/2 && i <= bandwidth.m_bandHigh; i+= hum_fs)
		f0.push_back(i);

	for (i = 0; i < f0.size(); i++)
	{
		b.clear();
		a.clear();

		b.push_back(1);
		tmp = -2 * R * cos( 2*M_PI*f0[i] / fs);
		b.push_back(tmp);
		b.push_back(R*R);

		a.push_back(1);
		tmp = -2 * r * cos( 2*M_PI*f0[i] / fs);
		a.push_back(tmp);
		a.push_back(r*r);

		filt50Hz(data, countChannels, b, a);
	}

}

/// Digital signal filtering Nx50hz
void CDSP::filt50Hz(wxVector<SIGNALTYPE>* data, const int& countChannels, const wxVector<double>& B, const wxVector<double>& A)
{
	int i;
	CFiltFilt** threads = new CFiltFilt*[countChannels];

	for (i = 0; i < countChannels; i++)
	{
		threads[i] = new CFiltFilt(B, A, &data[i]);
		threads[i]->Run();
	}

	for (i = 0; i < countChannels; i++)
	{
		threads[i]->Wait();
		delete threads[i];
	}

	delete [] threads;
}


/// Zero-phase forward and reverse digital IIR filtering.
void CDSP::FiltFilt(const wxVector<double>& B, const wxVector<double>& A, wxVector<SIGNALTYPE>* X)
{
	CFiltFilt * thread;

	thread = new CFiltFilt(B, A, X);
	thread->Run();
	thread->Wait();

	delete thread;
}

// Butterworth filter design ----

/// Butterworth filter order selection
void CDSP::Buttord(const double& wp, double& ws, const double& rp, const double& rs, int& order)
{
	int ftype = 0;
	double WP, WS, WA, WN, W0;
	double ord;
	double x, y, f1, f2, log;

	wxString str;

	if ( wp <= 0 || wp >= 1 || ws <= 0 || ws >= 1)
	{
		str.Printf(wxT("Bad input parameters for Buttord! \n wp = %f, ws = %f"), wp, ws);
		throw new CException(str, wxT("CDSP::Buttord"));
	}

	if (wp < ws)
		ftype = ftype + 1;  // low (1)
	else
		ftype = ftype + 2;  // high (2)

	WP = tan(PId*wp/2);
	WS = tan(PId*ws/2);

	if (ftype == 1)         // low (1)
		WA = WS/WP;
	else if (ftype == 2)    // high (2)
		WA = WP/WS;

	WA = abs(WA);

	x = 0.1 * abs(rs);
	y = 0.1 * abs(rp);
	f1 = pow(10, x) - 1;
	f2 = pow(10, y) - 1;
	log = log10(f1/f2);
	ord = ceil( log / (2*log10(WA)) );

	x = pow(10, 0.1*abs(rs)) - 1;
	y = 1/(2*(abs(ord)));
	W0 = WA/pow(x,y);

	if (ftype == 1)         // low
		WN = W0 * WP;
	else if (ftype == 2)    // high
		WN = WP / W0;

	order = ord;
	ws = ((2/PId)*atan(WN));
}

void CDSP::getPoleCoefs(double p, double np, double fc, double r, int highpass, double a[3], double b[3])
{
	double rp, ip, es, vx, kx, t, w, m, d, x0, x1, x2, y1, y2, k;

	// calculate pole locate on the unit circle
	rp = -cos(PId / (np * 2.0) + (p - 1.0) * PId / np);
	ip = sin(PId / (np * 2.0) + (p - 1.0) * PId / np);

	// Warp from a circle to an ellipse
	if (r != 0.0) {
		es = sqrt(pow(1.0 / (1.0 - r), 2) - 1.0);
		vx = asinh(1.0/es) / np;
		kx = acosh(1.0/es) / np;
		kx = cosh( kx );
		rp *= sinh(vx) / kx;
		ip *= cosh(vx) / kx;
	}

	// s to z domains conversion
	t = 2.0*tan(0.5);
	w = 2.0*PId*fc;
	m = rp*rp + ip*ip;
	d = 4.0 - 4.0*rp*t + m*t*t;
	x0 = t*t/d;
	x1 = 2.0*t*t/d;
	x2 = t*t/d;
	y1 = (8.0 - 2.0*m*t*t)/d;
	y2 = (-4.0 - 4.0*rp*t - m*t*t)/d;

	// LP(s) to LP(z) or LP(s) to HP(z)
	if (highpass)
		k = -cos(w/2.0 + 0.5)/cos(w/2.0 - 0.5);
	else
		k = sin(0.5 - w/2.0)/sin(0.5 + w/2.0);
	d = 1.0 + y1*k - y2*k*k;
	a[0] = (x0 - x1*k + x2*k*k)/d;
	a[1] = (-2.0*x0*k + x1 + x1*k*k - 2.0*x2*k)/d;
	a[2] = (x0*k*k - x1*k + x2)/d;
	b[1] = (2.0*k + y1 + y1*k*k - 2.0*y2*k)/d;
	b[2] = (-k*k - y1*k + y2)/d;
	if (highpass) {
		a[1] *= -1.0;
		b[1] *= -1.0;
	}
}

int CDSP::computeChebyIir(double *num, double *den, unsigned int num_pole,
						  int highpass, double ripple, double cutoff_freq)
{
	double *a, *b, *ta, *tb;
	double ap[3], bp[3];
	double sa, sb, gain;
	unsigned int i, p;
	int retval = 1;

	// Allocate temporary arrays
	a = new double[num_pole + 3];
	b = new double[num_pole + 3];
	ta = new double[num_pole + 3];
	tb = new double[num_pole + 3];

	if (!a || !b || !ta || !tb) {
		retval = 0;
		goto exit;
	}
	memset(a, 0, (num_pole + 3) * sizeof(*a));
	memset(b, 0, (num_pole + 3) * sizeof(*b));

	a[2] = 1.0;
	b[2] = 1.0;

	for (p = 1; p <= num_pole / 2; p++) {
		// Compute the coefficients for this pole
		getPoleCoefs(p, num_pole, cutoff_freq, ripple, highpass, ap, bp);

		// Add coefficients to the cascade
		memcpy(ta, a, (num_pole + 3) * sizeof(*a));
		memcpy(tb, b, (num_pole + 3) * sizeof(*b));
		for (i = 2; i <= num_pole + 2; i++) {
			a[i] = ap[0]*ta[i] + ap[1]*ta[i-1] + ap[2]*ta[i-2];
			b[i] = tb[i] - bp[1]*tb[i-1] - bp[2]*tb[i-2];
		}
	}

	// Finish combining coefficients
	b[2] = 0.0;
	for (i = 0; i <= num_pole; i++) {
		a[i] = a[i + 2];
		b[i] = -b[i + 2];
	}

	// Normalize the gain
	sa = sb = 0.0;
	for (i = 0; i <= num_pole; i++) {
		sa += a[i] * ((highpass && i % 2) ? -1.0 : 1.0);
		sb += b[i] * ((highpass && i % 2) ? -1.0 : 1.0);
	}
	gain = sa / (1.0 - sb);
	for (i = 0; i <= num_pole; i++)
		a[i] /= gain;

	// Copy the results to the num and den
	for (i = 0; i <= num_pole; i++) {
		num[i] = a[i];
		den[i] = -b[i];
	}
	// den[0] must be 1.0
	den[0] = 1.0;

exit:
	delete [] a;
	delete [] b;
	delete [] ta;
	delete [] tb;

	return retval;
}

/// Calculate coefficients for Butterworth filter.
int CDSP::calcButterCoeff(unsigned int nchann, int proctype, double fc,
						  unsigned int num_pole, int highpass, struct coeff *coeff)
{
	double* num = NULL,* den = NULL;
	double ripple = 0.0;
	int res = 0;

	if (num_pole % 2 != 0)
		return -1;

	num = new double[num_pole+1];
	if (num == NULL)
		return -2;
	den = new double[num_pole+1];
	if (den == NULL) {
		res = -3;
		goto err1;
	}

	/* Prepare the z-transform of the filter */
	if (!computeChebyIir(num, den, num_pole, highpass, ripple, fc)) {
		res = -4;
		goto err2;
	}

	coeff->num = num;
	coeff->den = den;

	return 0;

err2:
	delete [] den;
err1:
	delete [] num;
	return res;
}

/// Designs an Nth order lowpass digital Butterworth filter and returns the filter coefficients in length N+1 vectors B (numerator) and A (denominator)
bool CDSP::Butter(wxVector<double>& b, wxVector<double>& a, const int& order, const double& Wn, FILTERTYPE ftype)
{
	struct   coeff coeff;
	int      res;
	unsigned i;

	unsigned int nchann     = 1;                /* channels number */
	int proctype            = RTF_DOUBLE;       /* samples have float type */
	double fc               = Wn/2;             /* normalized cut-off frequency, Hz */
	unsigned int num_pole   = order;            /* filter order */
	int highpass            = ftype;            /* lowpass filter */

	res = calcButterCoeff(nchann, proctype, fc, num_pole, highpass, &coeff);
	if (res != 0) {
		cout << "Error: unable to calculate coefficients: " << res << endl;
		return false;
	}

	b.clear();
	a.clear();

	for (i = 0; i < num_pole+1; i++)
	{
		a.push_back(coeff.den[i]);
		b.push_back(coeff.num[i]);
	}

	delete [] coeff.num;
	delete [] coeff.den;
	return true;
}

// ------------------------------------------------------------------------------------------------
// CFiltFilt
// ------------------------------------------------------------------------------------------------

/// A constructor.
CFiltFilt::CFiltFilt(const wxVector<double>& B, const wxVector<double>& A, wxVector<SIGNALTYPE>* X)
	: wxThread(wxTHREAD_JOINABLE)
{
	m_A = A;
	m_B = B;
	m_X = X;
}

/// A destructor.
CFiltFilt::~CFiltFilt()
{
	/* empty */
}

/// This is the entry point of the thread. Run filtering.
wxThread::ExitCode CFiltFilt::Entry()
{
	vector<double> a, b, x, y;

	a.assign(m_A.begin(), m_A.end());
	b.assign(m_B.begin(), m_B.end());
	x.assign(m_X->begin(), m_X->end());

	try
	{
		filtFilt(b, a, x, y);
		m_X->assign(y.begin(), y.end());
	}
	catch(CException* e)
	{
		return e;
	}

	return NULL;
}

// Zero-phase digital filtering
// this codo is from: http://stackoverflow.com/questions/17675053/matlabs-filtfilt-algorithm/27270420#27270420
void CFiltFilt::filtFilt(vector<double> B, vector<double> A, const vector<double> &X, vector<double> &Y)
{
	int len = X.size();     // length of input
	int na = A.size();
	int nb = B.size();
	int nfilt = (nb > na) ? nb : na;
	int nfact = 3 * (nfilt - 1); // length of edge transients

	if (len <= nfact)
		throw new CException(wxT("Input data too short! Data must have length more than 3 times filter order."), wxT("CFiltFilt::filtFilt"));

	// set up filter's initial conditions to remove DC offset problems at the
	// beginning and end of the sequence
	B.resize(nfilt, 0);
	A.resize(nfilt, 0);

	vector<int> rows, cols;
	//rows = [1:nfilt-1           2:nfilt-1             1:nfilt-2];
	add_index_range(rows, 0, nfilt - 2, 1);
	if (nfilt > 2)
	{
		add_index_range(rows, 1, nfilt - 2, 1);
		add_index_range(rows, 0, nfilt - 3, 1);
	}
	//cols = [ones(1,nfilt-1)         2:nfilt-1          2:nfilt-1];
	add_index_const(cols, 0, nfilt - 1);
	if (nfilt > 2)
	{
		add_index_range(cols, 1, nfilt - 2, 1);
		add_index_range(cols, 1, nfilt - 2, 1);
	}
	// data = [1+a(2)         a(3:nfilt)        ones(1,nfilt-2)    -ones(1,nfilt-2)];

	auto klen = rows.size();
	vector<double> data;
	data.resize(klen);
	data[0] = 1 + A[1];  int j = 1;
	if (nfilt > 2)
	{
		for (int i = 2; i < nfilt; i++)
			data[j++] = A[i];
		for (int i = 0; i < nfilt - 2; i++)
			data[j++] = 1.0;
		for (int i = 0; i < nfilt - 2; i++)
			data[j++] = -1.0;
	}

	vector<double> leftpad = subvector_reverse(X, nfact, 1);
	double _2x0 = 2 * X[0];
	transform(leftpad.begin(), leftpad.end(), leftpad.begin(), [_2x0](double val) {return _2x0 - val; });

	vector<double> rightpad = subvector_reverse(X, len - 2, len - nfact - 1);
	double _2xl = 2 * X[len-1];
	transform(rightpad.begin(), rightpad.end(), rightpad.begin(), [_2xl](double val) {return _2xl - val; });

	double y0;
	vector<double> signal1, signal2, zi;

	signal1.reserve(leftpad.size() + X.size() + rightpad.size());
	append_vector(signal1, leftpad);
	append_vector(signal1, X);
	append_vector(signal1, rightpad);

	// Calculate initial conditions
	MatrixXd sp = MatrixXd::Zero(max_val(rows) + 1, max_val(cols) + 1);
	for (size_t k = 0; k < klen; ++k)
		sp(rows[k], cols[k]) = data[k];
	auto bb = VectorXd::Map(B.data(), B.size());
	auto aa = VectorXd::Map(A.data(), A.size());
	MatrixXd zzi = (sp.inverse() * (bb.segment(1, nfilt - 1) - (bb(0) * aa.segment(1, nfilt - 1))));
	zi.resize(zzi.size());

	// Do the forward and backward filtering
	y0 = signal1[0];
	transform(zzi.data(), zzi.data() + zzi.size(), zi.begin(), [y0](double val){ return val*y0; });
	filter(B, A, signal1, signal2, zi);
	reverse(signal2.begin(), signal2.end());
	y0 = signal2[0];
	transform(zzi.data(), zzi.data() + zzi.size(), zi.begin(), [y0](double val){ return val*y0; });
	filter(B, A, signal2, signal1, zi);
	Y = subvector_reverse(signal1, signal1.size() - nfact - 1, nfact);
}

void CFiltFilt::add_index_range(vector<int> &indices, int beg, int end, int inc = 1)
{
	for (int i = beg; i <= end; i += inc)
		indices.push_back(i);
}

void CFiltFilt::add_index_const(vector<int> &indices, int value, size_t numel)
{
	while (numel--)
		indices.push_back(value);
}

void CFiltFilt::append_vector(vector<double> &vec, const vector<double> &tail)
{
	vec.insert(vec.end(), tail.begin(), tail.end());
}

vector<double> CFiltFilt::subvector_reverse(const vector<double> &vec, int idx_end, int idx_start)
{
	vector<double> result(&vec[idx_start], &vec[idx_end+1]);
	reverse(result.begin(), result.end());

	return result;
}

void CFiltFilt::filter(vector<double> B, vector<double> A, const vector<double> &X, vector<double> &Y, vector<double> &Zi)
{
	if (A.empty())
		throw new CException(wxT("The feedback filter coefficients are empty."), wxT("CFiltFilt::filter"));
	if (all_of(A.begin(), A.end(), [](double coef){ return coef == 0; }))
		throw new CException(wxT("At least one of the feedback filter coefficients has to be non-zero."), wxT("CFiltFilt::filter"));
	if (A[0] == 0)
		throw new CException(wxT("First feedback coefficient has to be non-zero."), wxT("CFiltFilt::filter"));

	// Normalize feedback coefficients if a[0] != 1;
	double a0 = A[0];
	if (a0 != 1.0)
	{
		transform(A.begin(), A.end(), A.begin(), [a0](double v) { return v / a0; });
		transform(B.begin(), B.end(), B.begin(), [a0](double v) { return v / a0; });
	}

	size_t input_size = X.size();
	size_t filter_order = max(A.size(), B.size());
	B.resize(filter_order, 0);
	A.resize(filter_order, 0);
	Zi.resize(filter_order, 0);
	Y.resize(input_size);

	const double *x = &X[0];
	const double *b = &B[0];
	const double *a = &A[0];
	double *z = &Zi[0];
	double *y = &Y[0];

	for (size_t i = 0; i < input_size; ++i)
	{
		size_t order = filter_order - 1;
		while (order)
		{
			if (i >= order)
				z[order - 1] = b[order] * x[i - order] - a[order] * y[i - order] + z[order];
			--order;
		}
		y[i] = b[0] * x[i] + z[0];
	}
	Zi.resize(filter_order - 1);
}
