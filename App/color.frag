/**
 * @brief Source code of the fragment shader used for all the drawing modes.
 *
 * @file
 * @include color.frag
 */
/// @cond

uniform vec4 color;

#ifndef GL_2_0
out vec4 outColor;
#endif

void main()
{
#ifdef GL_2_0
	gl_FragColor = color;
#else
	outColor = color;
#endif
}
/// @endcond
