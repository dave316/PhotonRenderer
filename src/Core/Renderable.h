#ifndef INCLUDED_RENDERABLE
#define INCLUDED_RENDERABLE

#pragma once

#include "Component.h"

#include <Graphics/Material.h>
#include <Graphics/Mesh.h>
#include <Graphics/Skin.h>
#include <Graphics/Shader.h>

struct ModelUniformData
{
	glm::mat4 M;
	glm::mat4 N;
	int animMode = 0;
	//glm::vec3 padding;
};

struct IBLUniformData
{
	int diffuseMode;
	int specularProbeIndex;
	int lightMapIndex;
	int padding;
	glm::vec4 lightMapST;
	glm::vec4 sh[9];
};

enum RenderType
{
	OPAQUE = 0,
	TRANSPARENT = 1
};

class Renderable : public Component
{
	Mesh::Ptr mesh;
	Skin::Ptr skin;
	bool skinnedMesh = false;
	bool morphTagets = false;
	bool enabled = true;

	int bufferOffset = 0;
	int diffuseMode = 0; // 0 - lightprobe(cubemap), 1 - lightprobe(SH), 2 - lightmap
	int lightMapIndex = -1;
	glm::vec2 lightMapOffset = glm::vec2(0);
	glm::vec2 lightMapScale = glm::vec2(0);

	std::string reflName = "";
	int specularProbeIndex = 0;
	std::vector<glm::vec3> sh9;

	RenderType type;
	unsigned int renderPriority;

public:
	Renderable(RenderType type = RenderType::OPAQUE) : type(type), renderPriority(0) {}
	~Renderable();
	void setMesh(Mesh::Ptr mesh);
	void render(Shader::Ptr shader, bool useShader = false);
	void switchMaterial(int materialIndex);
	void setSkin(Skin::Ptr skin);
	void setEnabled(bool enabled);
	bool isSkinnedMesh();
	bool useMorphTargets();
	bool isEnabled();
	void setPriority(unsigned int priority);
	void setType(RenderType type);
	void setDiffuseMode(int mode);
	void setLightMapIndex(int mode);
	void setLightMapST(glm::vec2 offsect, glm::vec2 scale);
	void setReflectionProbe(std::string name, int index);
	void setProbeSH9(std::vector<glm::vec3>& sh9);
	void setOffset(int offset);
	void writeUniformData(IBLUniformData& data);
	std::vector<float> getWeights();
	void computeJoints(std::vector<Entity::Ptr>& nodes);
	Mesh::Ptr getMesh();
	Skin::Ptr getSkin();
	Box getBoundingBox();
	glm::vec2 getLMOffset();
	glm::vec2 getLMScale();
	int getLMIndex();
	int getRPIndex();
	int getDiffuseMode();
	std::string getReflName();
	std::vector<glm::vec3> getSH9();
	unsigned int getPriority();
	RenderType  getType();
	typedef std::shared_ptr<Renderable> Ptr;
	static Ptr create()
	{
		return std::make_shared<Renderable>();
	}

};

#endif // INCLUDED_RENDERABLE