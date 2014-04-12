attribute vec4 position;
attribute vec4 vertcolors;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
//uniform mat4 mvpmat;

//attribute vec3 normalIn;
//varying vec3 normalOut;
varying vec4 vertcolorout;

void main(void)
{
	gl_Position = projection * (view * (model * position));
	//gl_Position = projection * view * model * position;
	//gl_Position = mvpmat * position;
	//gl_Position = position * mvpmat;
	//gl_Position = position * model * view * projection;
	//normalOut = normalIn;
	//gl_Position.w = 1;
	vertcolorout = vertcolors;
}