#ifndef INCLUDED_PARTICLESYSTEM
#define INCLUDED_PARTICLESYSTEM

#pragma once

#include "Component.h"

#include <Graphics/Mesh.h>
#include <Graphics/Texture.h>
#include <Graphics/Shader.h>

struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 color;
	float lifeTime;
	float size;
	float type;
};

struct Generator
{
	glm::vec3 position;
	glm::vec3 minVelocity;
	glm::vec3 maxVelocity;
	glm::vec3 gravity;
	glm::vec3 color;
	float lifeMin;
	float lifeMax;
	float size;
	float generationTime;
	int numToGenerate;
};

class ParticleSystem : public Component
{
private:
	Shader::Ptr updateShader;
	Shader::Ptr renderShader;
	Texture2D::Ptr particleTexture;

	GL::VertexBuffer<Particle> particleBuffer[2];
	GL::VertexArray vao[2];
	glm::vec3 quadRight;
	glm::vec3 quadUp;

	GLuint tffBuffer;
	GLuint query;

	int numParticles;
	int currentBuffer;
	float elapsedTime;

	Generator generator;

	ParticleSystem(const ParticleSystem&) = delete;
	ParticleSystem& operator=(const ParticleSystem&) = delete;
public:

	ParticleSystem(Shader::Ptr updateShader, Shader::Ptr renderShader);
	~ParticleSystem();

	float grandf(float min, float max);
	void setGenerator(const Generator& generator);
	void setVP(const glm::mat4& VP);
	void createBillboard(glm::vec3 direction, glm::vec3 up);
	void updateTransformFeedback(float timePassed);
	void renderParticles();
	int getNumParticles();

	typedef std::shared_ptr<ParticleSystem> Ptr;
	static Ptr create(Shader::Ptr updateShader, Shader::Ptr renderShader)
	{
		return std::make_shared<ParticleSystem>(updateShader, renderShader);
	}
};

#endif // INCLUDED_PARTICLESYSTEM