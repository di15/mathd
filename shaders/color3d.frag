
#version 150

uniform vec4 color;

//varying vec3 normalOut;

out vec4 outfrag;

void main(void)
{
	outfrag = color;
}