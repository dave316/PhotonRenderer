#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(Type type, const std::string& name) : 
	type(type),
	name(name)
{
	projection = glm::perspective(yfov, aspect, zNear, zFar);
}

void Camera::computeProjection()
{
	switch (type)
	{
	case PERSPECTIVE: 
		projection = glm::perspective(yfov, aspect, zNear, zFar); 
		break;
	case ORTHOGRAPHIC: 
		projection = glm::ortho(-xmag, xmag, -ymag, ymag, zNear, zFar); 
		break;
	}
}

void Camera::setOrthoProjection(float xmag, float ymag)
{
	this->xmag = xmag;
	this->ymag = ymag;
	type = ORTHOGRAPHIC;
	computeProjection();
}

void Camera::setFieldOfView(float yfov)
{
	this->yfov = yfov;
	type = PERSPECTIVE;
	computeProjection();
}

void Camera::setAspectRatio(float aspect)
{
	this->aspect = aspect;
	computeProjection();
}

void Camera::setNearPlane(float zNear)
{
	this->zNear = zNear;
	computeProjection();
}

void Camera::setFarPlane(float zFar)
{
	this->zFar = zFar;
	computeProjection();
}

glm::mat4 Camera::getProjection() const
{
	return projection;
}

std::string Camera::getName() const
{
	return name;
}