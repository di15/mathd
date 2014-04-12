uniform vec4 color;

//varying vec3 normalOut;
varying vec4 vertcolorout;

void main(void)
{
	gl_FragColor = color * vertcolorout;
}