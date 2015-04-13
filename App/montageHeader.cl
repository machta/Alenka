in_type sum(int from, int to, PARA)
{
	in_type tmp = 0;
	for (int i = from; i <= to; ++i)
	{
		tmp += in(i);
	}
	return tmp;
}
#define sum(a_, b_) sum(a_, b_, PASS)
