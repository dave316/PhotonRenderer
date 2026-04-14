#ifndef INCLUDED_FPSCAMERA
#define INCLUDED_FPSCAMERA

#pragma once

#include "UserCamera.h"

class FPSCamera : public UserCamera
{
private:
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
		float zFar = 25.0f);

	void setSpeed(float speed);
	void setVelocity(float velocity);
	void setDirection(Direction dir);
	void releaseDirection(Direction dir);
	void updateRotation(float dx, float dy);
	void updateSpeed(float dx, float dy);
	void move(float deltaTime);
	void rotate(float deltaTime);

	glm::mat4 getViewMatrix() const;
};

#endif // INCLUDED_FPSCAMERA