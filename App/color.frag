#version 150 core
/**
 * @brief Source code of the fragment shader used for all the drawing modes.
 *
 * @file
 * @include color.frag
 */
/// @cond

uniform vec4 color;

out vec4 outColor;

void main()
{
	outColor = color;
}
/// @endcond
