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
	movementSpeed = 5.0f;
	rotationSpeed = 0.1f;
	velocity = movementSpeed * 0.1f;
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

void FPSCamera::setSpeed(float speed)
{
	this->movementSpeed = speed;
}

void FPSCamera::setVelocity(float velocity)
{
	this->velocity = velocity;
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


glm::mat4 FPSCamera::getViewMatrix() const
{
	return glm::lookAt(position, position + direction, up);
}


