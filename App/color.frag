#version 110
#extension GL_EXT_gpu_shader4 : enable
/**
 * @brief Source code of the fragment shader used for all the drawing modes.
 *
 * @file
 * @include color.frag
 */
/// @cond

uniform vec4 color;

//out vec4 outColor;

void main()
{
	//outColor = color;
	gl_FragColor = color;
}
/// @endcond
