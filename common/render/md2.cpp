#include "md2.h"
#include "main.h"
#include "3dmath.h"
#include "model.h"

vec3_t anorms_table[162] = {
#include <anorms.h>
};

int CModel::ReadMD2Model(const char *filename, struct md2_model_t *mdl, Vec3f scale, Vec3f translate)
{
  FILE *fp;
  int i;

  g_log<<filename<<"\n\r";

  fp = fopen (filename, "rb");
  if (!fp)
    {
      fprintf (stderr, "Error: couldn't open \"%s\"!\n", filename);
      return 0;
    }

  /* Read header */
  fread (&mdl->header, 1, sizeof (struct md2_header_t), fp);

  if ((mdl->header.ident != 844121161) ||
      (mdl->header.version != 8))
    {
      /* Error! */
      fprintf (stderr, "Error: bad version or identifier\n");
      fclose (fp);
      return 0;
    }

  /* Memory allocations */
  mdl->skins = (struct md2_skin_t *)
    malloc (sizeof (struct md2_skin_t) * mdl->header.num_skins);
  mdl->texcoords = (struct md2_texCoord_t *)
    malloc (sizeof (struct md2_texCoord_t) * mdl->header.num_st);
  mdl->triangles = (struct md2_triangle_t *)
    malloc (sizeof (struct md2_triangle_t) * mdl->header.num_tris);
  mdl->frames = (struct md2_frame_t *)
    malloc (sizeof (struct md2_frame_t) * mdl->header.num_frames);
  mdl->glcmds = (int *)malloc (sizeof (int) * mdl->header.num_glcmds);

  /* Read model data */
  fseek (fp, mdl->header.offset_skins, SEEK_SET);
  fread (mdl->skins, sizeof (struct md2_skin_t),
	 mdl->header.num_skins, fp);

  fseek (fp, mdl->header.offset_st, SEEK_SET);
  fread (mdl->texcoords, sizeof (struct md2_texCoord_t),
	 mdl->header.num_st, fp);

  fseek (fp, mdl->header.offset_tris, SEEK_SET);
  fread (mdl->triangles, sizeof (struct md2_triangle_t),
	 mdl->header.num_tris, fp);

  fseek (fp, mdl->header.offset_glcmds, SEEK_SET);
  fread (mdl->glcmds, sizeof (int), mdl->header.num_glcmds, fp);

  /* Read frames */
  fseek (fp, mdl->header.offset_frames, SEEK_SET);
  for (i = 0; i < mdl->header.num_frames; ++i)
    {
      /* Memory allocation for vertices of this frame */
      mdl->frames[i].verts = (struct md2_vertex_t *)
	malloc (sizeof (struct md2_vertex_t) * mdl->header.num_vertices);

      /* Read frame data */
      fread (mdl->frames[i].scale, sizeof (vec3_t), 1, fp);
      fread (mdl->frames[i].translate, sizeof (vec3_t), 1, fp);
      fread (mdl->frames[i].name, sizeof (char), 16, fp);
      fread (mdl->frames[i].verts, sizeof (struct md2_vertex_t),
	     mdl->header.num_vertices, fp);
    }

  fclose (fp);

  vertexArrays = new CVertexArray[ md2.header.num_frames ];

	struct md2_frame_t *pframe;
	struct md2_vertex_t *pvert;
	int j;
	int index;

	Vec3f* vertices;
	Vec2f* texcoords;
	Vec3f* normals;

	int numverts = md2.header.num_tris * 3;
	vector<Vec3f>* normalweights;
	normalweights = new vector<Vec3f>[numverts];

	for(int n=0; n<md2.header.num_frames; n++)
	{
		vertexArrays[n].numverts = md2.header.num_tris * 3;
		vertexArrays[n].vertices = new Vec3f[ md2.header.num_tris * 3 ];
		vertexArrays[n].texcoords = new Vec2f[ md2.header.num_tris * 3 ];
		vertexArrays[n].normals = new Vec3f[ md2.header.num_tris * 3 ];

		vertices = vertexArrays[n].vertices;
		texcoords = vertexArrays[n].texcoords;
		normals = vertexArrays[n].normals;

		for (i = 0; i < md2.header.num_tris; ++i)
		{
			for (j = 0; j < 3; ++j)
			{
				index = i*3 + j;

				pframe = &md2.frames[n];
				pvert = &pframe->verts[md2.triangles[i].vertex[j]];

				// Reverse vertex order
				if(j == 1)
					index += 1;
				else if(j == 2)
					index -= 1;

				// Compute texture coordinates
				texcoords[index].x = (GLfloat)md2.texcoords[md2.triangles[i].st[j]].s / md2.header.skinwidth;
				texcoords[index].y = (GLfloat)md2.texcoords[md2.triangles[i].st[j]].t / md2.header.skinheight;

				// Normal vector
				//glNormal3fv (anorms_table[pvert->normalIndex]);
				normals[index].x = anorms_table[pvert->normalIndex][0];
				normals[index].y = anorms_table[pvert->normalIndex][2];
				normals[index].z = -anorms_table[pvert->normalIndex][1];

				// Calculate vertex position
				vertices[index].x = (pframe->scale[0] * pvert->v[0]) + pframe->translate[0];
				vertices[index].y = (pframe->scale[2] * pvert->v[2]) + pframe->translate[2];
				vertices[index].z = -((pframe->scale[1] * pvert->v[1]) + pframe->translate[1]);
			
				vertices[index].x *= scale.x;
				vertices[index].y *= scale.y;
				vertices[index].z *= scale.z;
			
				vertices[index].x += translate.x;
				vertices[index].y += translate.y;
				vertices[index].z += translate.z;
			}

			
			Vec3f normal;
			Vec3f tri[3];
			tri[0] = vertices[index-3];
			tri[1] = vertices[index-3+1];
			tri[2] = vertices[index-3+2];
			normal = Normal2(tri);
			//normals[i] = normal;
			//normals[i+1] = normal;
			//normals[i+2] = normal;
				
			for(int j = 0; j < 3; j++)
			{
				int sharedvert = md2.triangles[i].vertex[j];
				normalweights[sharedvert].push_back(normal);
			}
		}

		for (i = 0; i < md2.header.num_tris; ++i)
		{
			for (j = 0; j < 3; ++j)
			{
				index = i*3 + j;
				int sharedvert = md2.triangles[i].vertex[j];

				Vec3f weighsum(0, 0, 0);

				for(int l=0; l<normalweights[sharedvert].size(); l++)
				{
					weighsum = weighsum + normalweights[sharedvert][l] / (float)normalweights[sharedvert].size();
				}
					
				normals[index] = weighsum;
				//normals[index+1] = weighsum;
				//normals[index+2] = weighsum;
			}
		}
	}

	delete [] normalweights;

  return 1;
}

void FreeModel(struct md2_model_t *mdl)
{
  int i;

  if (mdl->skins)
    {
      free (mdl->skins);
      mdl->skins = NULL;
    }

  if (mdl->texcoords)
    {
      free (mdl->texcoords);
      mdl->texcoords = NULL;
    }

  if (mdl->triangles)
    {
      free (mdl->triangles);
      mdl->triangles = NULL;
    }

  if (mdl->glcmds)
    {
      free (mdl->glcmds);
      mdl->glcmds = NULL;
    }

  if (mdl->frames)
    {
      for (i = 0; i < mdl->header.num_frames; ++i)
	{
	  free (mdl->frames[i].verts);
	  mdl->frames[i].verts = NULL;
	}

      free (mdl->frames);
      mdl->frames = NULL;
    }
}