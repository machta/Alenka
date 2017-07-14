float sum(int from, int to, PARA) {
  float tmp = 0;
  for (int i = from; i <= to; ++i) {
    tmp += in(i);
  }
  return tmp;
}
#define sum(a_, b_) sum(a_, b_, PASS)

float sumAll(PARA) { return sum(0, _inputRowCount_ - 1); }
#define sumAll() sumAll(PASS)

float average(PARA) { return sumAll() / _inputRowCount_; }
#define average() average(PASS)

// Weigted average by distance d where weights = 1/(d*coeff + 1).
// For ceoff = 1 the weights for distances 0, 1, 2, ... are 1, 1/2, 1/3, ...
float distAverage(int i, int coeff, PARA) {
  float tmp = 0;
  for (int j = 0; j < _inputRowCount_; ++j) {
    if (i != j) {
      float d = dist(i, j) * coeff + 1;
      tmp += (in(i) - in(j)) * 1 / d;
    }
  }
  return tmp;
}
#define distAverage(a_, b_) distAverage(a_, b_, PASS)

// Weigted average by distance d where weights = -d/maxDist + 1.
// It's a linear function wiht maximum at [0, 1] and minimum at [maxDist, 0].
// For d > maxDist weights are 0.
float distAverageLinear(int i, int maxDist, PARA) {
  float tmp = 0;
  for (int j = 0; j < _inputRowCount_; ++j) {
    float d = dist(i, j);
    if (i != j && d < maxDist) {
      d = -(d / maxDist) + 1;
      tmp += (in(i) - in(j)) * d;
    }
  }
  return tmp;
}
#define distAverageLinear(a_, b_) distAverageLinear(a_, b_, PASS)

