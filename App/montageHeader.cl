#define in(a_) input[inputRowLength*(a_) + inputRowOffset + get_global_id(0)]

float4 sum(int from, int to, __global float4* input, int inputRowLength, int inputRowOffset)
{
	float4 tmp = 0;
	for (int i = from; i <= to; ++i)
	{
		tmp += in(i);
	}
	return tmp;
}
#define sum(a_, b_) sum(a_, b_, input, inputRowLength, inputRowOffset)

