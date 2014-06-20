
#version 150

uniform vec4 color;
uniform vec4 owncolor;

uniform sampler2D texture0;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform sampler2D texture3;
uniform sampler2D shadowmap;
uniform sampler2D specularmap;
uniform sampler2D normalmap;
uniform sampler2D ownermap;

in vec4 lpos;
in vec3 light_vec;
in vec3 light_dir;

in vec2 texCoordOut0;

in vec3 normalOut;
in vec3 eyevec;

const vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

out vec4 outfrag;

void main (void)
{
	//if(elevy > maxelev)
	//	discard;

	vec4 texel0 = texture(texture0, texCoordOut0);
	//vec4 texel1 = texture(texture1, texCoordOut0);
	//vec4 texel2 = texture(texture2, texCoordOut0);
	//vec4 texel3 = texture(texture3, texCoordOut0);

	//float alph1 = texel1.w;
	//float alph2 = texel2.w;
	//float alph3 = texel3.w;

	vec4 stexel = texel0;
	//stexel = vec4(stexel.xyz * (1.0 - alph1) + texel1.xyz * alph1, 1.0);
	//stexel = vec4(stexel.xyz * (1.0 - alph2) + texel2.xyz * alph2, 1.0);
	//stexel = vec4(stexel.xyz * (1.0 - alph3) + texel3.xyz * alph3, 1.0);

	//float alph = color.w * texel0.w * elevtransp;
	float alph = texel0.w;

	if(alph < 0.5)
		discard;

	alph = color.w;

	float cosTheta = dot( normalOut, light_vec );
	float shadow_bias = 0.005 * tan(acos(cosTheta)); 
	// cosTheta is dot( n,l ), clamped between 0 and 1
	shadow_bias = clamp(shadow_bias, 0,0.01);

	vec3 smcoord = lpos.xyz / lpos.w;
	//float shadow = max(0.6, 
	//	float(smcoord.z - shadow_bias <= texture(shadowmap, smcoord.xy).x));
		//float(smcoord.z <= texture(shadowmap, smcoord.xy).x));
	float shadow = 1;

	for (int i=0;i<4;i++)
	{
  		if ( texture2D( shadowmap, smcoord.xy + poissonDisk[i]/700.0 ).z 
			<  smcoord.z - shadow_bias )
		{
    			shadow-=0.1;
  		}
	}

	vec3 bump = normalize( texture(normalmap, texCoordOut0).xyz * 2.0 - 1.0);

	//vec3 lvec = normalize(light_vec);
	//float diffuse = max(dot(-lvec, normalOut), 0.0) + 0.50;

	float distSqr = dot(light_vec, light_vec);
	vec3 lvec = light_vec * inversesqrt(distSqr);
	//float diffuse = max( dot(lvec, vec3(0,0,1)), 0.0 ) * 0.75 + 0.50;
	float diffuse = max( dot(lvec, bump), 0.0 ) * 0.75 + 0.50;

	vec3 vvec = normalize(eyevec);
	float specular = pow(clamp(dot(reflect(-lvec, bump), vvec), 0.0, 1.0), 0.7 );
	//float specular = pow(clamp(dot(reflect(-lvec, vec3(0,0,1)), vvec), 0.0, 1.0), 0.7 );
	//vec3 vspecular = vec3(0,0,0);
	vec3 vspecular = texture(specularmap, texCoordOut0).xyz * specular;

	vec4 owntexel = texture(ownermap, texCoordOut0);

	float alph1 = owntexel.w;
	stexel = vec4(stexel.xyz * (1.0 - alph1) + owncolor.xyz * alph1, 1.0);

	float minlight = min(shadow, diffuse);

	outfrag = vec4(color.xyz * stexel.xyz * minlight + vspecular, alph);
	//gl_FragColor = vec4(color.xyz * stexel.xyz * shadow * diffuse, alph);
	//gl_FragColor = vec4(1,0,0,1);
	//gl_FragColor = texel0;
	//gl_FragColor = vec4(light_vec, color.w * texel0.w);	
	//gl_FragColor = vec4(vspecular, color.w * texel0.w);
	//gl_FragColor = vec4(eyevec, color.w * texel0.w);
}

