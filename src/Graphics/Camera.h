#ifndef INCLUDED_CAMERA
#define INCLUDED_CAMERA

#pragma once

#include <glm/glm.hpp>

class Camera
{
private:
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 right;
	glm::vec3 up;
	float fov;
	float aspect;
	float zNear;
	float zFar;
	float yaw;
	float pitch;
	float rotationSpeed;
	float movementSpeed;
	bool directions[4];
public:
	enum Direction
	{
		FORWARD,
		BACK,
		LEFT,
		RIGHT
	};

	struct UniformData
	{
		glm::mat4 V;
		glm::mat4 V_I;
		glm::mat4 P;
		glm::mat4 P_I;
		glm::mat4 VP;
		glm::mat4 VP_I;
		glm::vec4 position;
		glm::vec3 direction;
		float zNear;
		float zFar;
	};

	Camera();
	void init(glm::vec3 pos = glm::vec3(0.0f),
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float fov = 60.0f,
		float aspect = 4.0f / 3.0f,
		float zNear = 0.01f,
		float zFar = 1000.0f);

	void setAspect(float aspect);
	void setFov(float fov);

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	glm::mat4 getViewProjectionMatrix() const;

	void setDirection(Direction dir);
	void releaseDirection(Direction dir);
	void updateRotation(float dx, float dy);
	void move(float deltaTime);
	void rotate(float deltaTime);
	void writeUniformData(UniformData& data);
};

#endif // INCLUDED_CAMERA