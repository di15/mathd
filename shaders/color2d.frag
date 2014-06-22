
#version 130


uniform vec4 color;

//varying vec2 texCoordOut0;
//uniform sampler2D texture0;

out vec4 outfrag;

void main(void)
{
	outfrag = color;	// * texture2D(texture0, texCoordOut0);
}