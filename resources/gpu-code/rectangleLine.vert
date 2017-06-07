/**
 * @brief Source code of the vertex shader used for drawing of all-channel events and lines.
 *
 * @file
 * @include rectangleLine.vert
 */
/// @cond

#ifdef GLSL_110
#extension GL_EXT_gpu_shader4 : enable
attribute vec2 inputVertex;
#else
in vec2 inputVertex;
#endif

uniform mat4 transformMatrix;

void main() {
  gl_Position = transformMatrix*vec4(inputVertex, 0, 1);
}
/// @endcond
