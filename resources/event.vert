/**
 * @brief Source code of the vertex shader used for drawing of single-channel events.
 *
 * @file
 * @include event.vert
 */
/// @cond

#ifdef GLSL_110
attribute float sampleValue;
#else
in float sampleValue;
#endif

uniform mat4 transformMatrix;
uniform float y0;
uniform int bufferOffset;
uniform float yScale;
uniform float eventWidth;
uniform int divideBy;

void main()
{
	float x = float(bufferOffset) + float(gl_VertexID)/float(divideBy);

	float y = y0 + yScale*sampleValue + eventWidth*float(1 - 2*(gl_VertexID & 1));

	gl_Position = transformMatrix*vec4(x, y, 0, 1);
}
/// @endcond
