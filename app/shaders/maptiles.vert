
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

out vec3 eyevec;
//attribute vec3 tangent;

//varying float elevy;

//uniform float sandonlyminy = -100,000;
uniform float sandonlymaxy;	// 100
uniform float sandgrassmaxy;	// 1,000
uniform float grassonlymaxy;	// 75,000
uniform float grassrockmaxy;	// 90,000
//uniform float rockonlymaxy = 100,000;
uniform float mapminz;
uniform float mapmaxz;

out float sandalpha;
out float grassalpha;
out float rockalpha;
out float snowalpha;
out float crackedrockalpha;

uniform vec3 sundirection;
uniform mat4 normalMat;

uniform float mind;
uniform float maxd;

out float logz;
const float C = 0.1;

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

	//elevy = position.y;

	if(position.y < sandonlymaxy)
	{
		sandalpha = 1;
		grassalpha = 0;
		rockalpha = 0;
		snowalpha = 0;
	}
	else if(position.y < sandgrassmaxy)
	{
		float transition = (position.y - sandonlymaxy) / (sandgrassmaxy - sandonlymaxy);
		sandalpha = 1.0 - transition;
		grassalpha = transition;
		rockalpha = 0;
		snowalpha = 0;
	}
	else if(position.y < grassonlymaxy)
	{
		sandalpha = 0;
		grassalpha = 1;
		rockalpha = 0;
		snowalpha = 0;
	}
	else if(position.y < grassrockmaxy)
	{
		float transition = (position.y - grassonlymaxy) / (grassrockmaxy - grassonlymaxy);
		sandalpha = 0;
		grassalpha = 1.0 - transition;
		snowalpha = 0;
		rockalpha = transition;
	}
	else
	{
		sandalpha = 0;
		grassalpha = 0;
		snowalpha = 0;
		rockalpha = 1;
	}

	// Make cracked rock ridges appear at more horizontal-facing polygons.
	// Higher normal.y means the polygon is more upward-facing.
	crackedrockalpha = min(1, 
				max(0, 
					1.0 - (normalIn.y - 0.2)/0.6
				)
				);

	// We don't want sandy beaches with steep inclines to look like rock.
	if(position.y < sandonlymaxy)
		crackedrockalpha = 0;

	float otheralpha = snowalpha + grassalpha + rockalpha + sandalpha;
	float alphascale = (1.0 - crackedrockalpha) / otheralpha;
	
	snowalpha *= alphascale;
	grassalpha *= alphascale;
	rockalpha *= alphascale;
	sandalpha *= alphascale;

	const float minalph = 0.25;
	const float maxalph = 0.75;
	const float arange = maxalph - minalph;

	sandalpha = (max(minalph, min(maxalph, sandalpha)) - minalph) / arange;
	grassalpha = (max(minalph, min(maxalph, grassalpha)) - minalph) / arange;
	rockalpha = (max(minalph, min(maxalph, rockalpha)) - minalph) / arange;
	snowalpha = (max(minalph, min(maxalph, snowalpha)) - minalph) / arange;
	crackedrockalpha = (max(minalph, min(maxalph, crackedrockalpha)) - minalph) / arange;

	float totalalpha = sandalpha + grassalpha + rockalpha + snowalpha + crackedrockalpha;
	sandalpha /= totalalpha;
	grassalpha /= totalalpha;
	rockalpha /= totalalpha;
	snowalpha /= totalalpha;
	crackedrockalpha /= totalalpha;

/*
	sandalpha = 1;
	grassalpha = 1;
	snowalpha = 1;
	rockalpha = 1;
*/
/*
	sandalpha = 0;
	grassalpha = 0;
	snowalpha = 1;
	rockalpha = 0;
*/

	vpos = (view * (model * position));

	//vec3 normalEyeSpace = vec3( normalMatrix * vec4(normalIn, 0.0) );
	//vec3 normalEyeSpace = mat3(normalMatrix) * normalIn;
	//mat4 normalMat = transpose( inverse( model * view ) );
	//mat4 normalMat = invModelView;
	vec3 normalEyeSpace = vec3( normalMat * vec4(normalIn, 0.0) );
	normalOut = normalize(normalEyeSpace);
	//normalOut = normalIn;
/*
	vec3 n = normalIn;
	//vec3 tangentEyeSpace = vec3( normalMat * vec4(tangent, 0.0) );
	//vec3 t = normalize(tangentEyeSpace);
	//vec3 t = normalOut;

	vec3 t;
	//vec3 c1 = cross( normalOut, vec3(0.0, 0.0, 1.0) ); 
	//vec3 c2 = cross( normalOut, vec3(0.0, 1.0, 0.0) );
	vec3 c1 = cross( normalIn, vec3(0.0, 0.0, 1.0) ); 
	vec3 c2 = cross( normalIn, vec3(0.0, 1.0, 0.0) ); 

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
