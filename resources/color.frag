/**
 * @brief Source code of the fragment shader used for all the drawing modes.
 *
 * @file
 * @include color.frag
 */
/// @cond

#version 110

uniform vec4 color;

void main()
{
	gl_FragColor = color;
}
/// @endcond
