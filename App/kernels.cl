// This version takes advantage of the built in vector operations.

__kernel void filter(__global float4* a, __global float4* b)
{
	int id0 = get_global_id(0);
	int id1 = get_global_id(1);
	int size1 = get_global_size(1);

	int id = id0*size1 + id1;

	float4 la = a[id];
	float4 lb = b[id1];

	float4 tmp = (float4)(-la.s1, la.s0, -la.s3, la.s2);
	a[id] = la*lb.s0033 + tmp*lb.s1133;
}

/*
// This version does one multiplication at a time.

__kernel void filter(__global float2* a, __global float2* b)
{
	int id0 = get_global_id(0);
	int id1 = get_global_id(1);
	int size1 = get_global_size(1);

	int id = id0*size1 + id1;

	float2 la = a[id];
	float2 lb = b[id1];

	float2 tmp = (float2)(-la.s1, la.s0);
	a[id] = la*lb.s0 + tmp*lb.s1;
}
*/

/*
// This version does multiplication by definition.

inline float2 cmult(float2 a, float2 b)
{
	return (float2)(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

__kernel void filter(__global float4* a, __global float4* b)
{
	int id0 = get_global_id(0);
	int id1 = get_global_id(1);
	int size1 = get_global_size(1);

	int id = id0*size1 + id1;

	float4 la = a[id];
	float4 lb = b[id1];

	a[id] = (float4)(cmult(la.s01, lb.s01), cmult(la.s23, lb.s23));
}
*/
