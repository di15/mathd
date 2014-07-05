
#version 130



in vec4 position;

uniform float width;
uniform float height;

in vec2 texCoordIn0;
out vec2 texCoordOut0;

void main(void)
{
	gl_Position = vec4(position.x * 2.0 / width - 1.0,
		position.y * -2.0 / height + 1.0,
		position.z, 
		1.0);
                     
	texCoordOut0 = texCoordIn0;
}