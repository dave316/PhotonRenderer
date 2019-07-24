#include "Camera.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

Camera::Camera()
{
	init();
}

void Camera::init(glm::vec3 pos, glm::vec3 direction, glm::vec3 up, float fov, float aspect, float zNear, float zFar)
{
	this->position = pos;
	this->direction = direction;
	this->up = up;
	this->right = glm::normalize(glm::cross(direction, up));

	this->fov = fov;
	this->aspect = aspect;
	this->zNear = zNear;
	this->zFar = zFar;

	for (int i = 0; i < 4; i++)
	{
		directions[i] = false;
	}

	yaw = -90.0f;
	pitch = 0.0f;
	movementSpeed = 1.0f;
	rotationSpeed = 0.1f;
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position, position + direction, up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return glm::perspective(glm::radians(fov), aspect, zNear, zFar);
}

glm::mat4 Camera::getViewProjectionMatrix() const
{
	return getProjectionMatrix() * getViewMatrix();
}

void Camera::setDirection(Direction dir)
{
	directions[dir] = true;
}

void Camera::releaseDirection(Direction dir)
{
	directions[dir] = false;
}

void Camera::move(float deltaTime)
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

void Camera::setAspect(float aspect)
{
	this->aspect = aspect;
}

void Camera::setFov(float fov)
{
	this->fov = fov;
}

void Camera::updateRotation(float dx, float dy)
{
	yaw += dx * rotationSpeed;
	pitch += dy * rotationSpeed;
}

void Camera::rotate(float deltaTime)
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

void Camera::writeUniformData(UniformData& data)
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
