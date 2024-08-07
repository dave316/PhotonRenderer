#ifndef INCLUDED_FPSCAMERA
#define INCLUDED_FPSCAMERA

#pragma once

#include <glm/glm.hpp>

struct CameraUniformData
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

class FPSCamera
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
	float velocity;
	bool directions[4];
public:
	enum Direction
	{
		FORWARD,
		BACK,
		LEFT,
		RIGHT
	};

	FPSCamera();
	void init(glm::vec3 pos = glm::vec3(0.0f),
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float fov = 60.0f,
		float aspect = 4.0f / 3.0f,
		float zNear = 0.1f,
		float zFar = 500.0f);

	void setAspect(float aspect);
	void setFov(float fov);
	void setPlanes(float zNear, float zFar);
	void setSpeed(float speed);
	void setVelocity(float velocity);
	float getFov();
	float getAspect();
	float getZNear();
	float getZFar();

	glm::vec3 getPosition();
	glm::vec3 getForward();
	glm::vec3 getRight();
	glm::vec3 getUp();
	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	glm::mat4 getViewProjectionMatrix() const;

	void setPosition(glm::vec3 pos);
	void setDirection(Direction dir);
	void releaseDirection(Direction dir);
	void updateRotation(float dx, float dy);
	void updateSpeed(float dx, float dy);
	void move(float deltaTime);
	void rotate(float deltaTime);
	void writeUniformData(CameraUniformData& data);
};

#endif // INCLUDED_FPSCAMERA