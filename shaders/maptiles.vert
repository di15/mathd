
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
uniform float grassrockmaxy;	// 90,000
//uniform float rockonlymaxy = 100,000;
uniform float mapminz;
uniform float mapmaxz;

varying float sandalpha;
varying float grassalpha;
varying float rockalpha;
varying float snowalpha;
varying float crackedrockalpha;

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
	//gl_Position.w = 1;

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

	float mapzspan = mapmaxz - mapminz;
	float mapz10percent = mapzspan * 0.1;

	float snowupperz = mapmaxz - mapz10percent*2.0;
	float snowlowerz = mapminz + mapz10percent*2.0;

	float snowtransition = 0;

	if(position.z > snowupperz)
	{
		snowtransition = (position.z - snowupperz) / (mapz10percent*2.0);
	}
	else if(position.z < snowlowerz)
	{
		snowtransition = 1.0 - (position.z - mapminz) / (mapz10percent*2.0);
	}

	if(snowtransition > 0.0)
	{
		float otheralpha = sandalpha + grassalpha + rockalpha;
		float alphascale = (1.0 - snowtransition) / otheralpha;

		sandalpha *= alphascale;
		grassalpha *= alphascale;
		rockalpha *= alphascale;
		snowalpha = snowtransition;
	}
	else
	{
		float mapcenterz = mapminz + mapzspan/2.0;
		float equatorlower = mapcenterz - mapz10percent*2.0;
		float equatorupper = mapcenterz + mapz10percent*2.0;

		float sandtransition = 0;

		if(position.z > equatorlower && position.z <= mapcenterz)
		{
			sandtransition = (position.z - equatorlower) / (mapz10percent*2.0);
		}
		else if(position.z >= mapcenterz && position.z < equatorupper)
		{
			sandtransition = (equatorupper - position.z) / (mapz10percent*2.0);
		}

		if(sandtransition > 0.0 && sandtransition > sandalpha)
		{
			float otheralpha = snowalpha + grassalpha + rockalpha;

			if(otheralpha > 0.0)
			{
				float alphascale = (1.0 - sandtransition) / otheralpha;
				
				snowalpha *= alphascale;
				grassalpha *= alphascale;
				rockalpha *= alphascale;
				sandalpha = sandtransition;
			}
		}
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
	mat4 normalMat = transpose( inverse( model * view ) );
	//mat4 normalMat = invModelView;
	vec3 normalEyeSpace = vec3( normalMat * vec4(normalIn, 0.0) );
	normalOut = normalize(normalEyeSpace);

	vec3 n = normalIn;
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
