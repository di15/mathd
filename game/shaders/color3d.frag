
#version 130


uniform vec4 color;

//varying vec3 normalOut;

out vec4 outfrag;
in float logz;

void main(void)
{
	gl_FragDepth = logz;

	outfrag = color;
}
