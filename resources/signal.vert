/**
 * @brief Source code of the vertex shader used for drawing of the signal tracks.
 *
 * @file
 * @include signal.vert
 */
/// @cond

#version 110
#extension GL_EXT_gpu_shader4 : enable

attribute float sampleValue1;

uniform mat4 transformMatrix;
uniform float y0;
uniform int bufferOffset;
uniform float yScale;

void main()
{
	float x = float(bufferOffset) + float(gl_VertexID);

	float y = y0 + yScale*sampleValue1;

	gl_Position = transformMatrix*vec4(x, y, 0, 1);
}
/// @endcond
