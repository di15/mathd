
#version 130

in vec4 position;

//uniform mat4 projection;
uniform mat4 model;
uniform mat4 modelview;
//uniform mat4 view;
uniform mat4 mvp;

uniform mat4 lightMatrix;
uniform vec3 lightPos;

out vec4 lpos;
out vec3 light_vec;
out vec3 light_dir;

in vec3 normalIn;
out vec3 normalOut;

in vec2 texCoordIn0;
out vec2 texCoordOut0;
out vec2 phasetexCoordOut;

//uniform mat4 invModelView;
//uniform mat4 normalMat;

out vec3 eyevec;
//attribute vec3 tangent;

uniform float maxelev;
//varying float elevtransp;
out float elevy;

uniform vec3 sundirection;
uniform mat4 normalMat;

uniform float mind;
uniform float maxd;

out float logz;
const float C = 0.1;

uniform int wavephase;

void main(void)
{
	//vec4 vpos = (view * (model * position));
	vec4 vpos = model * position;
	//vec4 vpos = position;
	vpos.w = 1;
	lpos = lightMatrix * vpos;
/*
	lpos.xy /= 2.0;
	lpos.xy += vec2(0.5, 0.5);
*/
	lpos.w = 1;
	//gl_Position = projection * (view * (model * position));
	gl_Position = mvp * position;
	//gl_Position.w = 1;

	//gl_Position.z = 2.0*log(gl_Position.w/mind)/log(maxd/mind) - 1; 
    	//gl_Position.z *= gl_Position.w;

	float FC = 1.0/log(maxd*C + 1);
 
	logz = log(gl_Position.w*C + 1)*FC;
	gl_Position.z = (2*logz - 1)*gl_Position.w;

	elevy = position.y;
	//elevtransp = 1;

	//if(position.y > maxelev)
	//{
	//	elevtransp = 0;
	//}

	//vpos = (view * (model * position));	//130 note
	vpos = modelview * position;

	//vec3 normalEyeSpace = vec3( normalMatrix * vec4(normalIn, 0.0) );
	//vec3 normalEyeSpace = mat3(normalMatrix) * normalIn;
	//mat4 normalMat = transpose( inverse( model * view ) );
	//mat4 normalMat = transpose( inverse( model ) );
	//mat4 normalMat = invModelView;
	vec3 normalEyeSpace = vec3( normalMat * vec4(normalIn, 0.0) );
	normalOut = normalize(normalEyeSpace);

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

	//vec3 vVertex = vec3(view * (model * position));
	//vec3 vVertex = vec3(modelview * position);
	//vec3 vVertex = vec3(view * model * position);
	vec3 vVertex = vec3(vpos);

	//light_vec = vpos.xyz - lightPos;
	//vec3 tmpVec = lightPos - vVertex;
	vec3 tmpVec = sundirection;
	light_vec.x = dot(tmpVec, t);
	light_vec.y = dot(tmpVec, b);
	light_vec.z = dot(tmpVec, n);

	//light_vec = n;
	//light_vec = normalIn * 0.5 + 0.5;
	//light_vec = t;
	//light_vec = t * 0.5 + 0.5;
	//light_vec = b * 0.5 + 0.5;

	eyevec = -vVertex;

	texCoordOut0 = texCoordIn0;
	vec2 phasetexc;
	phasetexc.x = texCoordIn0.x + wavephase/200.0;
	phasetexc.y = texCoordIn0.y + wavephase/100.0;
	phasetexCoordOut = phasetexc;
}
