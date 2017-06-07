/**
 * @brief Source code of the vertex shader used for drawing of single-channel events.
 *
 * @file
 * @include event.vert
 */
/// @cond

#ifdef GLSL_110
#extension GL_EXT_gpu_shader4 : enable
//attribute float sampleValue0;
attribute float sampleValue1;
//attribute float sampleValue2;
#else
//in float sampleValue0;
in float sampleValue1;
//in float sampleValue2;
#endif

uniform mat4 transformMatrix;
uniform float y0;
uniform int bufferOffset;
uniform float yScale;
uniform float eventWidth;

void main() {
  float x = float(bufferOffset + gl_VertexID/2);
  float y = y0 + sampleValue1*yScale +
      eventWidth*float(1 - 2*(gl_VertexID & 1));

  gl_Position = transformMatrix*vec4(x, y, 0, 1);
}
/// @endcond
