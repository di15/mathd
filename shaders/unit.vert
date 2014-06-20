
#version 150

in vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 normalMatrix;

uniform mat4 lightMatrix;
uniform vec3 lightPos;
uniform vec3 lightDir;

out vec4 lpos;
out vec3 light_vec;
out vec3 light_dir;

in vec3 normalIn;
out vec3 normalOut;

in vec2 texCoordIn0;
out vec2 texCoordOut0;

//uniform mat4 invModelView;
//uniform mat4 normalMat;

out vec3 eyevec;
//attribute vec3 tangent;

uniform float maxelev;
//varying float elevtransp;
out float elevy;

uniform vec3 sundirection;

uniform float mind;
uniform float maxd;
out float logz;
const float C = 0.1;

void main(void)
{
	vec4 vpos = model * position;
	lpos = lightMatrix * vpos;
	lpos.w = 1;
	gl_Position = projection * (view * (model * position));
	//gl_Position.w = 1;

	float FC = 1.0/log(maxd*C + 1);
 
	logz = log(gl_Position.w*C + 1)*FC;
	gl_Position.z = (2*logz - 1)*gl_Position.w;

	mat4 normalMat = transpose( inverse( model ) );
	//mat4 normalMat = invModelView;
	vec3 normalEyeSpace = vec3( normalMat * vec4(normalIn, 0.0) );
	normalOut = normalize(normalEyeSpace);

	vec3 vVertex = vec3(view * (model * position));

	light_vec = sundirection;

	eyevec = -vVertex;

	texCoordOut0 = texCoordIn0;
}
