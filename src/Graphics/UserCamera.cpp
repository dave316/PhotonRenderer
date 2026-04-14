#include "UserCamera.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

UserCamera::UserCamera()
{
	init();
}

void UserCamera::init(glm::vec3 pos, glm::vec3 direction, glm::vec3 up, float fov, float aspect, float zNear, float zFar)
{
	this->position = pos;
	this->direction = direction;
	this->up = up;
	this->right = glm::normalize(glm::cross(direction, up));

	this->fov = glm::radians(fov);
	this->aspect = aspect;
	this->zNear = zNear;
	this->zFar = zFar;
}

glm::vec3 UserCamera::getPosition()
{
	return position;
}

glm::vec3 UserCamera::getForward()
{
	return direction;
}

glm::vec3 UserCamera::getRight()
{
	return right;
}

glm::vec3 UserCamera::getUp()
{
	return up;
}

glm::mat4 UserCamera::getProjectionMatrix() const
{
	return glm::perspective(fov, aspect, zNear, zFar);
}

glm::mat4 UserCamera::getViewProjectionMatrix() const
{
	return getProjectionMatrix() * getViewMatrix();
}

void UserCamera::setPosition(glm::vec3 pos)
{
	this->position = pos;
}

void UserCamera::setAspect(float aspect)
{
	if (aspect == 0)
		return;
	this->aspect = aspect;
}

void UserCamera::setFov(float fov)
{
	this->fov = glm::radians(fov);
}

void UserCamera::setPlanes(float zNear, float zFar)
{
	this->zNear = zNear;
	this->zFar = zFar;
}

float UserCamera::getFov()
{
	return fov;
}

float UserCamera::getAspect() 
{
	return aspect;
}

float UserCamera::getZNear()
{
	return zNear;
}

float UserCamera::getZFar()
{
	return zFar;
}

void UserCamera::writeUniformData(CameraUniformData& data)
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
