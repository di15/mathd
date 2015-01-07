
#version 130


uniform vec4 color;

//uniform sampler2D texture0;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform sampler2D texture3;
//uniform sampler2D specularmap;
//uniform sampler2D normalmap;
uniform sampler2D shadowmap;

in vec4 lpos;
in vec3 light_vec;
in vec3 light_dir;

in vec2 texCoordOut0;

in vec3 normalOut;

in vec3 eyevec;
//varying float elevtransp;
//uniform float maxelev;
//varying float elevy;

in float sandalpha;
in float grassalpha;
in float snowalpha;
in float rockalpha;
in float crackedrockalpha;

uniform sampler2D sandtex;
uniform sampler2D grasstex;
uniform sampler2D rocktex;
uniform sampler2D rocknormtex;
uniform sampler2D snowtex;
uniform sampler2D crackedrocktex;
uniform sampler2D crackedrocknormtex;

uniform vec3 sundirection;

const float tile_tex_scale = 5.3;
const float cracked_rock_tex_scale = 5.3;
//const float shadow_bias = 0.0;
//const float shadow_bias = 0.002;

out vec4 outfrag;

void main (void)
{
	//if(elevy > maxelev)
	//	discard;

	//gl_FragColor = vec4(lpos.x, lpos.y, 0, 1);
	//gl_FragColor = vec4(0, lpos.y, 0, 1);
	//gl_FragColor = vec4(lpos.x, 0, 0, 1);
	//gl_FragColor = vec4(smcoord.x, smcoord.y, 0, 1);
	//gl_FragColor = vec4(smcoord.x, 0, 0, 1);
	//gl_FragColor = vec4(0, smcoord.y, 0, 1);
	//gl_FragColor = vec4(lpos.w/100.0, lpos.w/100.0, lpos.w/100.0, 1);
	//return;


	//vec3 bump = normalize( texture(normalmap, texCoordOut0).xyz * 2.0 - 1.0);
	vec3 bump = vec3(0,0,1);
	vec3 rockbump = normalize( texture(rocknormtex, texCoordOut0 / tile_tex_scale).xyz * 2.0 - 1.0);
	vec3 crackedrockbump = normalize( texture(crackedrocknormtex, texCoordOut0 / cracked_rock_tex_scale ).xyz * 2.0 - 1.0);

	vec3 lvec = normalize(sundirection);
	//vec3f unitnormal = normalize(normalOut);
	//float diffuse = min(1, max(dot(lvec, unitnormal), 0.0) * 0.75 + 0.50);
	float diffuse = max(dot(lvec, normalOut), 0.0) * 0.75 + 0.50;

	//float distSqr = dot(light_vec, light_vec);
	//vec3 lvec = light_vec * inversesqrt(distSqr);
	//float diffuse = max( dot(lvec, bump), 0.0 ) + 0.25;
	//float diffuse = min(1, max( dot(lvec, bump), 0.0 ));
	//float rockdiffuse = min(1, max( dot(lvec, rockbump), 0.0 ));
	//float crackedrockdiffuse = min(1, max( dot(lvec, crackedrockbump), 0.0 ) * 0.75 + 0.50);

	//vec3 vvec = normalize(eyevec);
	//float specular = pow(clamp(dot(reflect(-lvec, bump), vvec), 0.0, 1.0), 0.7 );
	//vec3 vspecular = vec3(0,0,0);
	//vec3 vspecular = texture(specularmap, texCoordOut0).xyz * specular;

	vec4 sandtxl = texture(sandtex, texCoordOut0 / tile_tex_scale);
	vec4 grasstxl = texture(grasstex, texCoordOut0 / tile_tex_scale);
	vec4 rocktxl = texture(rocktex, texCoordOut0 / tile_tex_scale);
	//vec4 snowtxl = texture(snowtex, texCoordOut0 / tile_tex_scale);
	vec4 crackedrocktxl = texture(crackedrocktex, texCoordOut0 / cracked_rock_tex_scale );

	//float sandalpha2 = sandalpha + (sanddettxl.w * sandbumpscale);	
	//float grassalpha2 = grassalpha + (grassdettxl.w * grassbumpscale);
	//float dirtalpha2 = dirtalpha + (dirtdettxl.w * dirtbumpscale);
	//float rockalpha2 = rockalpha + (rockdettxl.w * rockbumpscale);
/*
	float sandalpha2 = sandalpha + (sandtxl.w * 0.2);	
	float grassalpha2 = grassalpha + (grasstxl.w * 0.2);
	float rockalpha2 = rockalpha + (rocktxl.w * 0.2);
	float snowalpha2 = snowalpha + (snowtxl.w * 0.2);
	float crackedrockalpha2 = crackedrockalpha + (crackedrocktxl.w * 0.2);
*/
	float sandalpha2 = 0.01 + sandalpha * sandtxl.w * 0.5;
	//float sandalpha2 = sandalpha;

	//if(sandalpha > 0.0 && sandalpha2 <= 0.1)
	//	sandalpha2 = 1.0;
	
	float grassalpha2 = grassalpha * grasstxl.w;
	float rockalpha2 = rockalpha * rocktxl.w;
	//float snowalpha2 = snowalpha * snowtxl.w;
	float crackedrockalpha2 = crackedrockalpha * crackedrocktxl.w;


/*
	float alphamag = sqrt( sandalpha2*sandalpha2 
			+ grassalpha2*grassalpha2 
			+ rockalpha2*rockalpha2 
			+ snowalpha2*snowalpha2 
			+ crackedrockalpha2*crackedrockalpha2 );
*/

	float totalalpha = sandalpha2 + 
				grassalpha2 +
				rockalpha2 +
				//snowalpha2 +
				crackedrockalpha2;

/*
	if(totalalpha <= 0.0)
	{
		sandalpha2 = 1;
		grassalpha2 = 0;
		rockalpha2 = 0;
		snowalpha2 = 0;
		crackedrockalpha2 = 0;
	}
	else if(sandalpha2 <= 0
		&& grassalpha2 <= 0
		&& rockalpha2 <= 0
		&& snowalpha2 <= 0
		&& crackedrockalpha2 <= 0)
	{
		sandalpha2 = 1;
		grassalpha2 = 0;
		rockalpha2 = 0;
		snowalpha2 = 0;
		crackedrockalpha2 = 0;
	}
	else*/
	{
/*
		sandalpha2 /= alphamag;
		grassalpha2 /= alphamag;
		rockalpha2 /= alphamag;
		snowalpha2 /= alphamag;
		crackedrockalpha2 /= alphamag;
*/

		sandalpha2 /= totalalpha;
		grassalpha2 /= totalalpha;
		rockalpha2 /= totalalpha;
		//snowalpha2 /= totalalpha;
		crackedrockalpha2 /= totalalpha;

	}

/*
	if(sandalpha2 
		+ grassalpha2 
		+ rockalpha2 
		+ snowalpha2 
		+ crackedrockalpha2 <= 0.5)
	{


		if(sandalpha > 0.0)
		{
			sandalpha2 = 1;
			grassalpha2 = 0;
			rockalpha2 = 0;
			snowalpha2 = 0;
			crackedrockalpha2 = 0;
		}
		else
		{
			sandalpha2 = 0;
			grassalpha2 = 0;
			rockalpha2 = 1;
			snowalpha2 = 0;
			crackedrockalpha2 = 0;
		}
	}
*/
	//sandalpha2 = 1;

	//sandalpha2 = 0;
	//grassalpha2 = 0;
	//rockalpha2 = 1;

	vec4 stexel = vec4( vec3(sandtxl.xyz * sandalpha2) +
				vec3(grasstxl.xyz * grassalpha2) +
				vec3(rocktxl.xyz * rockalpha2) +
				//vec3(snowtxl.xyz * snowalpha2) +
				vec3(crackedrocktxl.xyz * crackedrockalpha2),
				1.0);

	//float alph = color.w * texel0.w * elevtransp;
	float alph = 1;

	//shadow = 1;

	//diffuse = diffuse * (1.0 - crackedrockalpha2) + crackedrockdiffuse * crackedrockalpha2;

	//float minlight = diffuse;
	float minlight = diffuse;
	//minlight = 1;

	//gl_FragColor = vec4(stexel.xyz * minlight, 1);
	outfrag = vec4(color.xyz * stexel.xyz * minlight, alph);
	//gl_FragColor = vec4(color.xyz * stexel.xyz * shadow, alph);
	//gl_FragColor = vec4(color.xyz * stexel.xyz * shadow * diffuse, alph);
	//gl_FragColor = vec4(color.xyz * sanddettxl.xyz * shadow * diffuse, alph);
	//gl_FragColor = vec4(grassdettxl.xyz, 1.0);
	//gl_FragColor = vec4(grassgradtxl.rgb, 1.0);
	//gl_FragColor = grasstxl;
	//gl_FragColor = vec4(1,0,0,1);
	//gl_FragColor = texel0;
	//gl_FragColor = vec4(light_vec, color.w * texel0.w);	
	//gl_FragColor = vec4(vspecular, color.w * texel0.w);
	//gl_FragColor = vec4(eyevec, color.w * texel0.w);
	//gl_FragColor = vec4(totalalpha, totalalpha, totalalpha, 1);

	//if(gl_FragColor.x + gl_FragColor.y + gl_FragColor.z < 0.5)
	//	gl_FragColor = vec4(color.xyz * sandtxl.xyz * minlight, alph);
}

