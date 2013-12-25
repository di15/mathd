
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
varying float snowalpha;
varying float rockalpha;

uniform sampler2D sandtex;
uniform sampler2D grasstex;
uniform sampler2D rocktex;
uniform sampler2D snowtex;

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

	vec4 sandtxl = texture(sandtex, texCoordOut0 / 1.3);
	vec4 grasstxl = texture(grasstex, texCoordOut0 / 1.3);
	vec4 rocktxl = texture(rocktex, texCoordOut0 / 1.3);
	vec4 snowtxl = texture(snowtex, texCoordOut0 / 1.3);

	//float sandalpha2 = sandalpha + (sanddettxl.w * sandbumpscale);	
	//float grassalpha2 = grassalpha + (grassdettxl.w * grassbumpscale);
	//float dirtalpha2 = dirtalpha + (dirtdettxl.w * dirtbumpscale);
	//float rockalpha2 = rockalpha + (rockdettxl.w * rockbumpscale);

	float sandalpha2 = sandalpha + (sandtxl.w * 0.2);	
	float grassalpha2 = grassalpha + (grasstxl.w * 0.2);
	float rockalpha2 = rockalpha + (rocktxl.w * 0.2);
	float snowalpha2 = snowalpha + (snowtxl.w * 0.2);

	float minalph = 0.25;
	float maxalph = 0.75;
	float arange = maxalph - minalph;

	sandalpha2 = (max(minalph, min(maxalph, sandalpha2)) - minalph) / arange;
	grassalpha2 = (max(minalph, min(maxalph, grassalpha2)) - minalph) / arange;
	rockalpha2 = (max(minalph, min(maxalph, rockalpha2)) - minalph) / arange;
	snowalpha2 = (max(minalph, min(maxalph, snowalpha2)) - minalph) / arange;

	float totalalpha = sandalpha2 + grassalpha2 + rockalpha2 + snowalpha2;
	sandalpha2 = sandalpha2 / totalalpha;
	grassalpha2 = grassalpha2 / totalalpha;
	rockalpha2 = rockalpha2 / totalalpha;
	snowalpha2 = snowalpha2 / totalalpha;

	//sandalpha2 = 1;

	//sandalpha2 = 0;
	//grassalpha2 = 0;
	//rockalpha2 = 1;

	vec4 stexel = vec4( vec3(sandtxl.xyz * sandalpha2) +
				vec3(grasstxl.xyz * grassalpha2) +
				vec3(rocktxl.xyz * rockalpha2) +
				vec3(snowtxl.xyz * snowalpha2),
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

