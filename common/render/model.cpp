
#include "../platform.h"
#include "../math/3dmath.h"
#include "model.h"
#include "texture.h"
#include "../utils.h"
#include "../platform.h"
#include "../gui/gui.h"
#include "shader.h"
#include "../debug.h"
#include "vertexarray.h"

Model g_model[MODELS];
vector<ModelToLoad> g_modelsToLoad;

Model::~Model()
{
	if(m_va == NULL)
		return;

	delete [] m_va;
	m_va = NULL;
}

int NewModel()
{
	for(int i=0; i<MODELS; i++)
		if(!g_model[i].m_on)
			return i;

	return -1;
}

void QueueModel(int* id, const char* filepath, Vec3f scale, Vec3f translate)
{
	ModelToLoad toLoad;
	toLoad.id = id;
	strcpy(toLoad.filepath, filepath);
	toLoad.scale = scale;
	toLoad.translate = translate;

	g_modelsToLoad.push_back(toLoad);
}

bool Load1Model()
{
	static int last = -1;

	if(last+1 < g_modelsToLoad.size())
		Status(g_modelsToLoad[last+1].filepath);

	if(last >= 0)
	{
		int id = NewModel();
		g_model[id].m_on = true;
		g_model[id].load(g_modelsToLoad[last].filepath, g_modelsToLoad[last].scale, g_modelsToLoad[last].translate);
		(*g_modelsToLoad[last].id) = id;
	}

	last ++;

	if(last >= g_modelsToLoad.size())
	{
		g_modelsToLoad.clear();
		return false;	// Done loading all models
	}

	return true;	// Not finished loading models
}

void DrawVA(VertexArray* va, Vec3f pos)
{	
	Shader* s = &g_shader[g_curS];

	Matrix modelmat;
    modelmat.setTranslation((const float*)&pos);
    glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

    glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
    glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
    glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}

void Model::usetex()
{	
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_diffusem ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);
	/*
	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_specularm ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SPECULARMAP], 1);
	
	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_normalm ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_NORMALMAP], 2);
	*/
	glActiveTextureARB(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_ownerm ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_OWNERMAP], 3);
}

void Model::draw(int frame, Vec3f pos, float yaw)
{
	Shader* s = &g_shader[g_curS];

	//yaw = 110;	// (int)(rand()%360);

	float pitch = 0;
	Matrix modelmat;
    float radians[] = {DEGTORAD(pitch), DEGTORAD(yaw), 0};
    modelmat.setTranslation((const float*)&pos);
    Matrix rotation;
    rotation.setRotationRadians(radians);
    modelmat.postMultiply(rotation);
    glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	VertexArray* va = &m_va[frame];

	usetex();
    
    glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
    glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
    glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}

void Model::load(const char* filepath, Vec3f scale, Vec3f translate)
{
	m_diffusem = 0;
	m_specularm = 0;
	m_normalm = 0;
	m_ownerm = 0;

	bool result = m_ms3d.load(filepath);

	if(result)
	{
		m_ms3d.queuetex(m_diffusem, m_specularm, m_normalm, m_ownerm);
		m_ms3d.genva(&m_va, scale, translate, filepath);
	}
	
	/*
	if(result)
	{
		//CreateTexture(spectex, specfile);
		//QueueTexture(&spectex, specfile, true);
		CorrectNormals();
	}*/
}

void BeginVertexArrays()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glActiveTextureARB(GL_TEXTURE3_ARB);
	glEnable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
}

void EndVertexArrays()
{
	glActiveTextureARB(GL_TEXTURE3_ARB);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

bool PlayAnimation(float& frame, int first, int last, bool loop, float rate)
{
    if(frame < first || frame > last+1)
    {
        frame = first;
        return false;
    }
    
    frame += rate;
    
    if(frame > last)
    {
        if(loop)
            frame = first;
		else
			frame = last;
        
        return true;
    }
    
    return false;
}

//Play animation backwards
bool PlayAnimationB(float& frame, int first, int last, bool loop, float rate)
{
    if(frame < first-1 || frame > last)
    {
        frame = last;
        return false;
    }
    
    frame -= rate;
    
    if(frame < first)
    {
        if(loop)
            frame = last;
		else
			frame = first;
        
        return true;
    }
    
    return false;
}