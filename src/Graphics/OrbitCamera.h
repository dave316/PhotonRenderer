#ifndef INCLUDED_ORBITCAMERA
#define INCLUDED_ORBITCAMERA

#pragma once

#include "UserCamera.h"

class OrbitCamera : public UserCamera
{
private:
	glm::vec3 center;
	float distance = 1.0f;
	float yaw;
	float pitch;
	float rotationSpeed;
	float movementSpeed;
	float velocity;
public:

	OrbitCamera();
	void init(glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f),
		float fov = 60.0f,
		float aspect = 4.0f / 3.0f,
		float zNear = 0.01f,
		float zFar = 25.0f);

	void setCenter(glm::vec3 center);
	void setDistance(float distance);
	void setSpeed(float speed);
	void updateRotation(float dx, float dy);
	void move(float deltaTime);
	void rotate(float deltaTime);

	glm::mat4 getViewMatrix() const;
};

#endif // INCLUDED_ORBITCAMERA