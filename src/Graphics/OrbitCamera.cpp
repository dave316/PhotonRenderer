#include "OrbitCamera.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

OrbitCamera::OrbitCamera()
{
	init();
}

void OrbitCamera::init(glm::vec3 center, float fov, float aspect, float zNear, float zFar)
{
	this->center = center;

	this->fov = glm::radians(fov);
	this->aspect = aspect;
	this->zNear = zNear;
	this->zFar = zFar;

	yaw = 90.0f;
	pitch = 0.0f;
	movementSpeed = 10.0f;
	rotationSpeed = 0.1f;
	velocity = movementSpeed * 0.1f;
}

void OrbitCamera::setCenter(glm::vec3 center)
{
	this->center = center;
}

void OrbitCamera::setDistance(float distance)
{
	this->distance = distance;
}

void OrbitCamera::setSpeed(float speed)
{
	this->movementSpeed = speed;
}

void OrbitCamera::updateRotation(float dx, float dy)
{
	yaw += dx * rotationSpeed;
	pitch += dy * rotationSpeed;
}

void OrbitCamera::rotate(float deltaTime)
{
	if (pitch > 89.0f)
		pitch = 89.0f;

	if (pitch < -89.0f)
		pitch = -89.0f;

	position = center + distance * glm::vec3(
		glm::cos(glm::radians(pitch)) * glm::cos(glm::radians(yaw)),
		glm::sin(glm::radians(pitch)),
		glm::cos(glm::radians(pitch)) * glm::sin(glm::radians(yaw))
	);
}

void OrbitCamera::move(float deltaTime)
{
	distance += -deltaTime * movementSpeed;
	distance = glm::max(distance, 0.1f);
}

glm::mat4 OrbitCamera::getViewMatrix() const
{
	return glm::lookAt(position, center, up);
}

