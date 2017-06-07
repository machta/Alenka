/**
 * @brief Source code of the vertex shader used for drawing of the signal tracks.
 *
 * @file
 * @include signal.vert
 */
/// @cond

#ifdef GLSL_110
#extension GL_EXT_gpu_shader4 : enable
attribute float sampleValue1;
#else
in float sampleValue1;
#endif

uniform mat4 transformMatrix;
uniform float y0;
uniform int bufferOffset;
uniform float yScale;

void main() {
	float x = float(bufferOffset + gl_VertexID);
	float y = y0 + sampleValue1*yScale;

	gl_Position = transformMatrix*vec4(x, y, 0, 1);
}
/// @endcond
