#version 130


in vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

//attribute vec3 normalIn;
//varying vec3 normalOut;

in vec2 texCoordIn0;
out vec2 texCoordOut0;

uniform float mind;
uniform float maxd;

out float logz;
const float C = 0.1;

void main(void)
{
	//gl_Position = projection * (view * (model * position));	//why doesn't this work?
	gl_Position = projection * view * model * position;
	//gl_Position = projection * ((position));
	//gl_Position = projection * ((model * position));
	//gl_Position = projection * (view * (position));
	//gl_Position = (model * position);
	//normalOut = normalIn;
	texCoordOut0 = texCoordIn0;
	//gl_Position.w = 1;

	//float FC = 1.0/log(maxd*C + 1);
	 
	//logz = log(gl_Position.w*C + 1)*FC;
	//gl_Position.z = (2*logz - 1)*gl_Position.w;

	//gl_Position.x = clamp(position.x, -1, 1);
	//gl_Position.x = clamp(gl_Position.x, -1, 1);
	//gl_Position.y = clamp(position.y, -1, 1);
	//gl_Position.z = 0.5;
	//gl_Position.z = clamp(position.z, -1, 10);
	//gl_Position.z = 100;
	//gl_Position.z = clamp(gl_Position.z, -10, 1);
	//gl_Position.w = 1;
}
