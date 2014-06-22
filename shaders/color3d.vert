
#version 130


in vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
//uniform mat4 mvpmat;

//attribute vec3 normalIn;
//varying vec3 normalOut;

uniform float mind;
uniform float maxd;

out float logz;
const float C = 0.1;

void main(void)
{
	//gl_Position = projection * (view * (model * position));	//why doesn't this work?
	gl_Position = projection * view * model * position;
	//gl_Position = projection * view * model * position;
	//gl_Position = mvpmat * position;
	//gl_Position = position * mvpmat;
	//gl_Position = position * model * view * projection;
	//normalOut = normalIn;
	//gl_Position.w = 1;

	float FC = 1.0/log(maxd*C + 1);
 
	logz = log(gl_Position.w*C + 1)*FC;
	gl_Position.z = (2*logz - 1)*gl_Position.w;
}
