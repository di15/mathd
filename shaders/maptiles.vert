
#version 150

attribute vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 normalMatrix;

uniform mat4 lightMatrix;
uniform vec3 lightPos;
uniform vec3 lightDir;

varying vec4 lpos;
varying vec3 light_vec;
varying vec3 light_dir;

attribute vec3 normalIn;
varying vec3 normalOut;

attribute vec2 texCoordIn0;
varying vec2 texCoordOut0;

//uniform mat4 invModelView;
//uniform mat4 normalMat;

varying vec3 eyevec;
//attribute vec3 tangent;

//varying float elevy;

//uniform float sandonlyminy = -100,000;
uniform float sandonlymaxy;	// 100
uniform float sandgrassmaxy;	// 1,000
uniform float grassonlymaxy;	// 75,000
uniform float grassdirtmaxy;	// 90,000
uniform float dirtonlymaxy;	// 95,000
uniform float dirtrockmaxy;	// 99,000
//uniform float rockonlymaxy = 100,000;

varying float sandalpha;
varying float grassalpha;
varying float dirtalpha;
varying float rockalpha;

uniform vec3 sundirection;

void main(void)
{
	//vec4 vpos = (view * (model * position));
	vec4 vpos = model * position;
	//vec4 vpos = position;
	vpos.w = 1;
	lpos = lightMatrix * vpos;
	//lpos.w = 1;
	gl_Position = projection * (view * (model * position));
	gl_Position.w = 1;

	//elevy = position.y;

	if(position.y < sandonlymaxy)
	{
		sandalpha = 1;
		grassalpha = 0;
		dirtalpha = 0;
		rockalpha = 0;
	}
	else if(position.y < sandgrassmaxy)
	{
		float transition = (position.y - sandonlymaxy) / (sandgrassmaxy - sandonlymaxy);
		sandalpha = 1.0 - transition;
		grassalpha = transition;
		dirtalpha = 0;
		rockalpha = 0;
	}
	else if(position.y < grassonlymaxy)
	{
		sandalpha = 0;
		grassalpha = 1;
		dirtalpha = 0;
		rockalpha = 0;
	}
	else if(position.y < grassdirtmaxy)
	{
		float transition = (position.y - grassonlymaxy) / (grassdirtmaxy - grassonlymaxy);
		sandalpha = 0;
		grassalpha = 1.0 - transition;
		dirtalpha = transition;
		rockalpha = 0;
	}
	else if(position.y < dirtonlymaxy)
	{
		sandalpha = 0;
		grassalpha = 0;
		dirtalpha = 1;
		rockalpha = 0;
	}
	else if(position.y < dirtrockmaxy)
	{
		
		float transition = (position.y - dirtonlymaxy) / (dirtrockmaxy - dirtonlymaxy);
		sandalpha = 0;
		grassalpha = 0;
		dirtalpha = 1.0 - transition;
		rockalpha = transition;
	}
	else
	{
		sandalpha = 0;
		grassalpha = 0;
		dirtalpha = 0;
		rockalpha = 1;
	}

/*
	sandalpha = 1;
	grassalpha = 1;
	dirtalpha = 1;
	rockalpha = 1;
*/
/*
	sandalpha = 0;
	grassalpha = 0;
	dirtalpha = 1;
	rockalpha = 0;
*/

	vpos = (view * (model * position));

	//vec3 normalEyeSpace = vec3( normalMatrix * vec4(normalIn, 0.0) );
	//vec3 normalEyeSpace = mat3(normalMatrix) * normalIn;
	mat4 normalMat = transpose( inverse( model * view ) );
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

	vec3 vVertex = vec3(view * (model * position));

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

	tmpVec = -vVertex;
	eyevec.x = dot(tmpVec, t);
	eyevec.y = dot(tmpVec, b);
	eyevec.z = dot(tmpVec, n);

	texCoordOut0 = texCoordIn0;
}
