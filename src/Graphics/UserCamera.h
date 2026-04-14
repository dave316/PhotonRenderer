#ifndef INCLUDED_USERCAMERA
#define INCLUDED_USERCAMERA

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

class UserCamera
{
protected:
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 right;
	glm::vec3 up;
	float fov;
	float aspect;
	float zNear;
	float zFar;
public:
	UserCamera();
	virtual ~UserCamera() {}
	void init(glm::vec3 pos = glm::vec3(0.0f),
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float fov = 60.0f,
		float aspect = 4.0f / 3.0f,
		float zNear = 0.01f,
		float zFar = 25.0f);

	void setAspect(float aspect);
	void setFov(float fov);
	void setPlanes(float zNear, float zFar);
	float getFov();
	float getAspect();
	float getZNear();
	float getZFar();

	glm::vec3 getPosition();
	glm::vec3 getForward();
	glm::vec3 getRight();
	glm::vec3 getUp();
	virtual glm::mat4 getViewMatrix() const = 0;
	glm::mat4 getProjectionMatrix() const;
	glm::mat4 getViewProjectionMatrix() const;

	void setPosition(glm::vec3 pos);
	void writeUniformData(CameraUniformData& data);
};

#endif // INCLUDED_USERCAMERA