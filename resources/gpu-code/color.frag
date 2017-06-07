/**
 * @brief Source code of the fragment shader used for all the drawing modes.
 *
 * @file
 * @include color.frag
 */
/// @cond

uniform vec4 color;

#ifndef GLSL_110
out vec4 outColor;
#endif

void main() {
#ifdef GLSL_110
  gl_FragColor = color;
#else
  outColor = color;
#endif
}
/// @endcond
