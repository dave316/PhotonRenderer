#include "ParticleSystem.h"

#include <IO/Image/ImageDecoder.h>

ParticleSystem::ParticleSystem(Shader::Ptr updateShader, Shader::Ptr renderShader) :
	updateShader(updateShader), renderShader(renderShader)
{
	const unsigned int numParticleAttributes = 6;
	const char* varyings[numParticleAttributes] = {
		"gPosition",
		"gVelocity",
		"gColor",
		"gLifeTime",
		"gSize",
		"gType"
	};

	glTransformFeedbackVaryings(updateShader->getProgramID(), numParticleAttributes, varyings, GL_INTERLEAVED_ATTRIBS);
	updateShader->link();

	glGenTransformFeedbacks(1, &tffBuffer);
	glGenQueries(1, &query);

	Particle generator;
	generator.type = 0;

	const unsigned int maxParticles = 100000;
	for (unsigned int i = 0; i < 2; i++)
	{
		particleBuffer[i].upload(NULL, maxParticles, GL_DYNAMIC_DRAW);
		particleBuffer[i].uploadSubData(&generator, 0, 1);

		vao[i].addAttrib(0, 3, particleBuffer[i], sizeof(Particle), (GLvoid*)0);
		vao[i].addAttrib(1, 3, particleBuffer[i], sizeof(Particle), (GLvoid*)offsetof(Particle, velocity));
		vao[i].addAttrib(2, 3, particleBuffer[i], sizeof(Particle), (GLvoid*)offsetof(Particle, color));
		vao[i].addAttrib(3, 1, particleBuffer[i], sizeof(Particle), (GLvoid*)offsetof(Particle, lifeTime));
		vao[i].addAttrib(4, 1, particleBuffer[i], sizeof(Particle), (GLvoid*)offsetof(Particle, size));
		vao[i].addAttrib(5, 1, particleBuffer[i], sizeof(Particle), (GLvoid*)offsetof(Particle, type));
	}

	auto img = IO::decodeFromFile("../../../../assets/textures/particle.bmp");
	particleTexture = img->upload(true);

	numParticles = 1;
	currentBuffer = 0;
}

ParticleSystem::~ParticleSystem()
{
	glDeleteTransformFeedbacks(1, &tffBuffer);
	glDeleteQueries(1, &query);
}

float ParticleSystem::grandf(float min, float add)
{
	float random = (float)(rand() % (RAND_MAX + 1)) / (float)RAND_MAX;
	return min + add * random;
}

void ParticleSystem::setGenerator(const Generator& generator)
{
	this->generator = generator;
}

void ParticleSystem::setVP(const glm::mat4& VP)
{
	renderShader->setUniform("VP", VP);
}

void ParticleSystem::createBillboard(glm::vec3 direction, glm::vec3 up)
{
	quadRight = glm::normalize(glm::cross(direction, up));
	quadUp = glm::normalize(glm::cross(quadRight, direction));

	renderShader->setUniform("quadRight", quadRight);
	renderShader->setUniform("quadUp", quadUp);
}

void ParticleSystem::updateTransformFeedback(float timePassed)
{
	updateShader->setUniform("genPosition", generator.position);
	updateShader->setUniform("genGravity", generator.gravity);
	updateShader->setUniform("genVelocityMin", generator.minVelocity);
	updateShader->setUniform("genVelocityRange", generator.maxVelocity - generator.minVelocity);
	updateShader->setUniform("genColor", generator.color);
	updateShader->setUniform("genSize", generator.size);
	updateShader->setUniform("genLifeMin", generator.lifeMin);
	updateShader->setUniform("genLifeRange", generator.lifeMax - generator.lifeMin);
	updateShader->setUniform("timePassed", timePassed);
	updateShader->setUniform("numToGenerate", 0);

	elapsedTime += timePassed;

	if (elapsedTime > generator.generationTime)
	{
		updateShader->setUniform("numToGenerate", generator.numToGenerate);
		elapsedTime -= generator.generationTime;
		glm::vec3 randomSeed = glm::vec3(grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f));
		updateShader->setUniform("randomSeed", randomSeed);
	}

	updateShader->use();

	glEnable(GL_RASTERIZER_DISCARD);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tffBuffer);

	vao[currentBuffer].bind();
	glEnableVertexAttribArray(1);

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffer[1 - currentBuffer]);

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, numParticles);

	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectiv(query, GL_QUERY_RESULT, &numParticles);

	vao[currentBuffer].unbind();

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glDisable(GL_RASTERIZER_DISCARD);

	currentBuffer = 1 - currentBuffer;
}

void ParticleSystem::renderParticles()
{
	renderShader->use();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);

	renderShader->setUniform("particleTexture", 0);
	particleTexture->use(0);

	vao[currentBuffer].bind();
	glDisableVertexAttribArray(1);

	glDrawArrays(GL_POINTS, 0, numParticles);

	vao[currentBuffer].unbind();

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

int ParticleSystem::getNumParticles()
{
	return numParticles;
}