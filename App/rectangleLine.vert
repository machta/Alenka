#version 150 core
/**
 * @brief Source code of the vertex shader used for drawing of all-channel events and lines.
 *
 * @file
 * @include rectangleLine.vert
 */
/// @cond

/*layout(location = 0) */in vec2 inputVertex;

uniform mat4 transformMatrix;

void main()
{
	gl_Position = transformMatrix*vec4(inputVertex, 0, 1);
}
/// @endcond
