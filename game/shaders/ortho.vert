
#version 120


uniform float width;
uniform float height;

void main(void)
{
	gl_Position = vec4(gl_Position.x * 2.0 / width - 1.0,
		gl_Position.y * -2.0 / height + 1.0,
		0, 
		1.0);
                     
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
