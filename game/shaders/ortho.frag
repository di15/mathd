
#version 120


uniform vec4 color;
uniform sampler2D texture0;

void main(void)
{
	gl_FragColor = color * texture2D(texture0, gl_TexCoord[0].xy);
}
