
#version 150

uniform vec4 color;

//uniform sampler2D texture0;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform sampler2D texture3;
//uniform sampler2D specularmap;
//uniform sampler2D normalmap;
uniform sampler2D shadowmap;

varying vec4 lpos;
varying vec3 light_vec;
varying vec3 light_dir;

varying vec2 texCoordOut0;

varying vec3 normalOut;

varying vec3 eyevec;
//varying float elevtransp;
//uniform float maxelev;
//varying float elevy;

varying float sandalpha;
varying float grassalpha;
varying float dirtalpha;
varying float rockalpha;

uniform sampler2D sandgrad;
uniform sampler2D sanddet;
uniform sampler2D grassgrad;
uniform sampler2D grassdet;
uniform sampler2D dirtgrad;
uniform sampler2D dirtdet;
uniform sampler2D rockgrad;
uniform sampler2D rockdet;

void main (void)
{
	//if(elevy > maxelev)
	//	discard;

	vec3 smcoord = lpos.xyz / lpos.w;
	float shadow = max(0.6, float(smcoord.z <= texture(shadowmap, smcoord.xy).x));
	//float shadow = 1;

	//vec3 bump = normalize( texture(normalmap, texCoordOut0).xyz * 2.0 - 1.0);
	vec3 bump = vec3(0,0,1);

	//vec3 lvec = normalize(light_vec);
	//float diffuse = max(dot(-lvec, normalOut), 0.0) + 0.50;

	float distSqr = dot(light_vec, light_vec);
	vec3 lvec = light_vec * inversesqrt(distSqr);
	float diffuse = max( dot(lvec, bump), 0.0 ) * 0.75 + 0.50;

	//vec3 vvec = normalize(eyevec);
	//float specular = pow(clamp(dot(reflect(-lvec, bump), vvec), 0.0, 1.0), 0.7 );
	//vec3 vspecular = vec3(0,0,0);
	//vec3 vspecular = texture(specularmap, texCoordOut0).xyz * specular;

	vec4 sandgradtxl = texture(sandgrad, texCoordOut0 / 100);
	vec4 sanddettxl = texture(sanddet, texCoordOut0 / 1);
	vec4 grassgradtxl = texture(grassgrad, texCoordOut0 / 100);
	vec4 grassdettxl = texture(grassdet, texCoordOut0 / 1);
	vec4 dirtgradtxl = texture(dirtgrad, texCoordOut0 / 100);
	vec4 dirtdettxl = texture(dirtdet, texCoordOut0 / 1);
	vec4 rockgradtxl = texture(rockgrad, texCoordOut0 / 100);
	vec4 rockdettxl = texture(rockdet, texCoordOut0 / 1);

	//float sandalpha2 = sandalpha + (sanddettxl.w * sandbumpscale);	
	//float grassalpha2 = grassalpha + (grassdettxl.w * grassbumpscale);
	//float dirtalpha2 = dirtalpha + (dirtdettxl.w * dirtbumpscale);
	//float rockalpha2 = rockalpha + (rockdettxl.w * rockbumpscale);

	float sandalpha2 = sandalpha + (sanddettxl.w * 0.2);	
	float grassalpha2 = grassalpha + (grassdettxl.w * 0.2);
	float dirtalpha2 = dirtalpha + (dirtdettxl.w * 0.2);
	float rockalpha2 = rockalpha + (rockdettxl.w * 0.2);

	float minalph = 0.25;
	float maxalph = 0.75;
	float arange = maxalph - minalph;

	sandalpha2 = (max(minalph, min(maxalph, sandalpha2)) - minalph) / arange;
	grassalpha2 = (max(minalph, min(maxalph, grassalpha2)) - minalph) / arange;
	dirtalpha2 = (max(minalph, min(maxalph, dirtalpha2)) - minalph) / arange;
	rockalpha2 = (max(minalph, min(maxalph, rockalpha2)) - minalph) / arange;

	float totalalpha = sandalpha2 + grassalpha2 + dirtalpha2 + rockalpha2;
	sandalpha2 = sandalpha2 / totalalpha;
	grassalpha2 = grassalpha2 / totalalpha;
	dirtalpha2 = dirtalpha2 / totalalpha;
	rockalpha2 = rockalpha2 / totalalpha;

	//sandalpha2 = 1;


	//sandalpha2 = 0;
	//grassalpha2 = 0;
	//dirtalpha2 = 0;
	//rockalpha2 = 1;


	vec4 stexel = vec4( vec3(sandgradtxl.xyz * sanddettxl.xyz * sandalpha2) +
				vec3(grassgradtxl.xyz * grassdettxl.xyz * grassalpha2) +
				vec3(dirtgradtxl.xyz * dirtdettxl.xyz * dirtalpha2) +
				vec3(rockgradtxl.xyz * rockdettxl.xyz * rockalpha2),
				1.0);

	//float alph = color.w * texel0.w * elevtransp;
	float alph = color.w * stexel.w;

	//shadow = 1;

	//gl_FragColor = vec4(color.xyz * stexel.xyz * shadow * diffuse + vspecular, alph);
	//gl_FragColor = vec4(color.xyz * stexel.xyz * shadow, alph);
	gl_FragColor = vec4(color.xyz * stexel.xyz * shadow * diffuse, alph);
	//gl_FragColor = vec4(color.xyz * sanddettxl.xyz * shadow * diffuse, alph);
	//gl_FragColor = vec4(grassdettxl.xyz, 1.0);
	//gl_FragColor = vec4(grassgradtxl.rgb, 1.0);
	//gl_FragColor = grassdettxl;
	//gl_FragColor = vec4(1,0,0,1);
	//gl_FragColor = texel0;
	//gl_FragColor = vec4(light_vec, color.w * texel0.w);	
	//gl_FragColor = vec4(vspecular, color.w * texel0.w);
	//gl_FragColor = vec4(eyevec, color.w * texel0.w);
}

