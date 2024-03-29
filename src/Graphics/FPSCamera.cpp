#include "FPSCamera.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

FPSCamera::FPSCamera()
{
	init();
}

void FPSCamera::init(glm::vec3 pos, glm::vec3 direction, glm::vec3 up, float fov, float aspect, float zNear, float zFar)
{
	this->position = pos;
	this->direction = direction;
	this->up = up;
	this->right = glm::normalize(glm::cross(direction, up));

	this->fov = glm::radians(fov);
	this->aspect = aspect;
	this->zNear = zNear;
	this->zFar = zFar;

	for (int i = 0; i < 4; i++)
	{
		directions[i] = false;
	}

	yaw = -90.0f;
	pitch = 0.0f;
	movementSpeed = 10.0f;
	rotationSpeed = 0.1f;
	velocity = movementSpeed * 0.1f;
}

glm::vec3 FPSCamera::getPosition()
{
	return position;
}

glm::vec3 FPSCamera::getForward()
{
	return direction;
}

glm::vec3 FPSCamera::getRight()
{
	return right;
}

glm::vec3 FPSCamera::getUp()
{
	return up;
}

glm::mat4 FPSCamera::getViewMatrix() const
{
	return glm::lookAt(position, position + direction, up);
}

glm::mat4 FPSCamera::getProjectionMatrix() const
{
	return glm::perspective(fov, aspect, zNear, zFar);
}

glm::mat4 FPSCamera::getViewProjectionMatrix() const
{
	return getProjectionMatrix() * getViewMatrix();
}

void FPSCamera::setPosition(glm::vec3 pos)
{
	this->position = pos;
}

void FPSCamera::setDirection(Direction dir)
{
	directions[dir] = true;
}

void FPSCamera::releaseDirection(Direction dir)
{
	directions[dir] = false;
}

void FPSCamera::move(float deltaTime)
{
	for (int i = 0; i < 4; i++)
	{
		if (directions[i])
		{
			switch (i)
			{
			case FORWARD:
				position += direction * deltaTime * movementSpeed;
				break;
			case BACK:
				position -= direction * deltaTime * movementSpeed;
				break;
			case LEFT:
				position -= right * deltaTime * movementSpeed;
				break;
			case RIGHT:
				position += right * deltaTime * movementSpeed;
				break;
			}
		}
	}
}

void FPSCamera::setAspect(float aspect)
{
	this->aspect = aspect;
}

void FPSCamera::setFov(float fov)
{
	this->fov = glm::radians(fov);
}

void FPSCamera::setPlanes(float zNear, float zFar)
{
	this->zNear = zNear;
	this->zFar = zFar;
}

void FPSCamera::setSpeed(float speed)
{
	this->movementSpeed = speed;
}

void FPSCamera::setVelocity(float velocity)
{
	this->velocity = velocity;
}

float FPSCamera::getFov()
{
	return fov;
}

float FPSCamera::getAspect() 
{
	return aspect;
}

float FPSCamera::getZNear()
{
	return zNear;
}

float FPSCamera::getZFar()
{
	return zFar;
}

void FPSCamera::updateRotation(float dx, float dy)
{
	yaw += dx * rotationSpeed;
	pitch += dy * rotationSpeed;
}

void FPSCamera::updateSpeed(float dx, float dy)
{
	movementSpeed += dy * velocity;
	movementSpeed = std::max(velocity, movementSpeed);
}

void FPSCamera::rotate(float deltaTime)
{
	if (pitch > 89.0f)
		pitch = 89.0f;

	if (pitch < -89.0f)
		pitch = -89.0f;

	direction = glm::vec3(
		glm::cos(glm::radians(pitch)) * glm::cos(glm::radians(yaw)),
		glm::sin(glm::radians(pitch)),
		glm::cos(glm::radians(pitch)) * glm::sin(glm::radians(yaw))
	);

	direction = glm::normalize(direction);
	right = glm::normalize(glm::cross(direction, up));
}

void FPSCamera::writeUniformData(CameraUniformData& data)
{
	data.V = getViewMatrix();
	data.V_I = glm::inverse(data.V);
	data.P = getProjectionMatrix();
	data.P_I = glm::inverse(data.P);
	data.VP = data.P * data.V;
	data.VP_I = glm::inverse(data.VP);
	data.position = glm::vec4(position, 0.0f);
	data.direction = direction;
	data.zNear = zNear;
	data.zFar = zFar;
}
