/**
 * Zero-phase forward and reverse digital IIR filtering.
 * This function is similar as Matlab function: y = filtfilt(b,a,x).
 * @param A vector describing filter.
 * @param B vector describing filter.
 * @param X input vector.
 * @param Y output vector.
 */
void filtfilt(vector<T> B, vector<T> A, const vector<T> &X, vector<T> &Y)
{
	using namespace Eigen;

	int len = X.size();     // length of input
	int na = A.size();
	int nb = B.size();
	int nfilt = (nb > na) ? nb : na;
	int nfact = 3 * (nfilt - 1); // length of edge transients

	if (len <= nfact)
	{
		throw std::domain_error("Input data too short! Data must have length more than 3 times filter order.");
	}

	// set up filter's initial conditions to remove DC offset problems at the
	// beginning and end of the sequence
	B.resize(nfilt, 0);
	A.resize(nfilt, 0);

	vector<int> rows, cols;
	//rows = [1:nfilt-1           2:nfilt-1             1:nfilt-2];
	add_index_range(rows, 0, nfilt - 2);
	if (nfilt > 2)
	{
		add_index_range(rows, 1, nfilt - 2);
		add_index_range(rows, 0, nfilt - 3);
	}
	//cols = [ones(1,nfilt-1)         2:nfilt-1          2:nfilt-1];
	add_index_const(cols, 0, nfilt - 1);
	if (nfilt > 2)
	{
		add_index_range(cols, 1, nfilt - 2);
		add_index_range(cols, 1, nfilt - 2);
	}
	// data = [1+a(2)         a(3:nfilt)        ones(1,nfilt-2)    -ones(1,nfilt-2)];

	auto klen = rows.size();
	vector<T> data;
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

	vector<T> leftpad = subvector_reverse(X, nfact, 1);
	double _2x0 = 2 * X[0];
	std::transform(leftpad.begin(), leftpad.end(), leftpad.begin(), [_2x0](double val) {return _2x0 - val; });

	vector<T> rightpad = subvector_reverse(X, len - 2, len - nfact - 1);
	double _2xl = 2 * X[len-1];
	std::transform(rightpad.begin(), rightpad.end(), rightpad.begin(), [_2xl](double val) {return _2xl - val; });

	double y0;
	vector<T> signal1, signal2, zi;

	signal1.reserve(leftpad.size() + X.size() + rightpad.size());
	append_vector(signal1, leftpad);
	append_vector(signal1, X);
	append_vector(signal1, rightpad);

	// Calculate initial conditions
	M sp = M::Zero(max_val(rows) + 1, max_val(cols) + 1);
	for (size_t k = 0; k < klen; ++k)
	{
		sp(rows[k], cols[k]) = data[k];
	}
	auto bb = V::Map(B.data(), B.size());
	auto aa = V::Map(A.data(), A.size());
	M zzi = (sp.inverse() * (bb.segment(1, nfilt - 1) - (bb(0) * aa.segment(1, nfilt - 1))));
	zi.resize(zzi.size());

	// Do the forward and backward filtering
	y0 = signal1[0];
	std::transform(zzi.data(), zzi.data() + zzi.size(), zi.begin(), [y0](double val){ return val*y0; });
	filter(B, A, signal1, signal2, zi);
	std::reverse(signal2.begin(), signal2.end());
	y0 = signal2[0];
	std::transform(zzi.data(), zzi.data() + zzi.size(), zi.begin(), [y0](double val){ return val*y0; });
	filter(B, A, signal2, signal1, zi);
	Y = subvector_reverse(signal1, signal1.size() - nfact - 1, nfact);
}
