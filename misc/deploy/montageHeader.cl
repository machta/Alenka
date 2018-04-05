// Return the index of the channel with the label passed as the first parameter.
int label(int i) {
  return i;
}

bool contains(int e, int* arr, int n) {
  for (int i = 0; i < n; ++i) {
    if (e == arr[i])
      return true;
  }
  return false;
}

// Sum channels in range of indexes [from, to] but skip all indexes contained in
// an array.
float sumSkip(int from, int to, int* skipIndexes, int skipIndexesN, PARA) {
  float sum = 0;
  for (int i = from; i <= to; ++i) {
    if (!contains(i, skipIndexes, skipIndexesN))
      sum += in(i);
  }
  return sum;
}
#define sumSkip(a_, b_, c_, d_) sumSkip(a_, b_, c_, d_, PASS)

// Sum channels in range of indexes [from, to].
float sum(int from, int to, PARA) {
  return sumSkip(from, to, 0, 0);
}
#define sum(a_, b_) sum(a_, b_, PASS)

// Sum all channels.
float sumAll(PARA) { return sum(0, IN_COUNT - 1); }
#define sumAll() sumAll(PASS)

// Sum channels in an array of indexes.
float sumArr(int *indexArr, int n, PARA) {
  float tmp = 0;
  for (int i = 0; i < n; ++i) {
    tmp += in(indexArr[i]);
  }
  return tmp;
}
#define sumArr(a_, b_) sumArr(a_, b_, PASS)

// Arithmetic average of all channels.
float average(PARA) { return sumAll() / IN_COUNT; }
#define average() average(PASS)

// Euclidean distance between channels i and j.
float dist(int i, int j, PARA) {
  return sqrt(pown(x(i) - x(j), 2) + pown(y(i) - y(j), 2) +
              pown(z(i) - z(j), 2));
}
#define dist(a_, b_) dist(a_, b_, PASS)

// Weigted average by distance d where weights = 1/(d*coeff + 1).
// For ceoff = 1 the weights for distances 0, 1, 2, ... are 1, 1/2, 1/3, ...
float distAverage(int i, int coeff, PARA) {
  float tmp = 0;
  for (int j = 0; j < IN_COUNT; ++j) {
    if (i != j) {
      float d = dist(i, j) * coeff + 1;
      tmp += (in(i) - in(j)) * 1 / d;
    }
  }
  return tmp;
}
#define distAverage(a_, b_) distAverage(a_, b_, PASS)

// Weigted average by distance d where weights = -d/maxDist + 1.
// It's a linear function with maximum at [0, 1] and minimum at [maxDist, 0].
// For d > maxDist weights are 0.
float distAverageLinear(int i, int maxDist, PARA) {
  float tmp = 0;
  for (int j = 0; j < IN_COUNT; ++j) {
    float d = dist(i, j);
    if (i != j && d < maxDist) {
      d = -(d / maxDist) + 1;
      tmp += (in(i) - in(j)) * d;
    }
  }
  return tmp;
}
#define distAverageLinear(a_, b_) distAverageLinear(a_, b_, PASS)

