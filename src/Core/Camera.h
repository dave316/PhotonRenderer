#ifndef INCLUDED_CAMERA
#define INCLUDED_CAMERA

#pragma once

#include "Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <string>

class Camera : public Component
{
private:
	float yfov = glm::radians(60.0f);
	float aspect = 4.0f / 3.0f;
	float xmag = 1.0f;
	float ymag = 1.0f;
	float zNear = 0.1f;
	float zFar = 1000.0f;
	glm::mat4 projection;
	std::string name;

	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;

	void computeProjection();
public:
	enum Type
	{
		PERSPECTIVE = 0,
		ORTHOGRAPHIC,
	};
	Type type;

	Camera(Type type, const std::string& name);
	void setOrthoProjection(float xmag, float ymag);
	void setFieldOfView(float yfov);
	void setAspectRatio(float aspect);
	void setNearPlane(float zNear);
	void setFarPlane(float zFar);
	glm::mat4 getProjection() const;
	std::string getName() const;
	typedef std::shared_ptr<Camera> Ptr;
	static Ptr create(Type type, const std::string& name)
	{
		return std::make_shared<Camera>(type, name);
	}
};

#endif // INCLUDED_CAMERA