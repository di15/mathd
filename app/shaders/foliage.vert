
#version 130


in vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 normalMatrix;
uniform mat4 mvp;

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
uniform mat4 normalMat;
/*
uniform mat4 modelmats[30000];
uniform int onswitches[30000];
*/

uniform float mind;
uniform float maxd;

out float logz;
const float C = 0.1;

void main(void)
{
	//mat4 model = modelmats[gl_InstanceID];
	//mat4 model = modelmats[0];

	//vec4 vpos = (view * (model * position));
	vec4 vpos = model * position;
	vpos.w = 1;
	lpos = lightMatrix * vpos;
	lpos.w = 1;

	//float onmult = onswitches[ 0 ];
	//onmult = 1;

	//gl_Position = projection * (view * (model * position));
	gl_Position = mvp * position;

	float FC = 1.0/log(maxd*C + 1);
 
	logz = log(gl_Position.w*C + 1)*FC;
	gl_Position.z = (2*logz - 1)*gl_Position.w;

	//gl_Position.w = 1;

	elevy = position.y;
	//elevtransp = 1;

	//if(position.y > maxelev)
	//{
	//	elevtransp = 0;
	//}

	//vpos = (view * (model * position));

	//vec3 normalEyeSpace = vec3( normalMatrix * vec4(normalIn, 0.0) );
	//vec3 normalEyeSpace = mat3(normalMatrix) * normalIn;
	//mat4 normalMat = transpose( inverse( model * view ) );
	//mat4 normalMat = transpose( inverse( model ) );
	//mat4 normalMat = invModelView;
	vec3 normalEyeSpace = vec3( normalMat * vec4(normalIn, 0.0) );
	normalOut = normalize(normalEyeSpace);
/*
	vec3 n = normalOut;
	//vec3 tangentEyeSpace = vec3( normalMat * vec4(tangent, 0.0) );
	//vec3 t = normalize(tangentEyeSpace);
	//vec3 t = normalOut;

	vec3 t;
	vec3 c1 = cross( normalOut, vec3(0.0, 0.0, 1.0) ); 
	vec3 c2 = cross( normalOut, vec3(0.0, 1.0, 0.0) ); 

	if( length(c1)>length(c2) )
	{
		t = normalize(c1);	
	}
	else
	{
		t = normalize(c2);	
	}

	vec3 b = normalize(cross(n, t));
	//vec3 b = normalOut;

	vec3 vVertex = vec3(view * (model * position));

	//light_vec = vpos.xyz - lightPos;
	//vec3 tmpVec = lightPos - vVertex;
	vec3 tmpVec = sundirection;
	light_vec.x = dot(tmpVec, t);
	light_vec.y = dot(tmpVec, b);
	light_vec.z = dot(tmpVec, n);
*/
	light_vec = sundirection;

	//light_vec = n;
	//light_vec = normalIn * 0.5 + 0.5;
	//light_vec = t;
	//light_vec = t * 0.5 + 0.5;
	//light_vec = b * 0.5 + 0.5;
/*
	tmpVec = -vVertex;
	eyevec.x = dot(tmpVec, t);
	eyevec.y = dot(tmpVec, b);
	eyevec.z = dot(tmpVec, n);
*/
	texCoordOut0 = texCoordIn0;
}
