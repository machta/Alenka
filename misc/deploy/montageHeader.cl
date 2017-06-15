float sum(int from, int to, PARA) {
  float tmp = 0;
  for (int i = from; i <= to; ++i) {
    tmp += in(i);
  }
  return tmp;
}
#define sum(a_, b_) sum(a_, b_, PASS)

