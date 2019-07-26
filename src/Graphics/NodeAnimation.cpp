#include "NodeAnimation.h"
#include <iostream>

NodeAnimation::NodeAnimation() :
	name(""),
	interpMode(Interpolation::LINEAR),
	currentTime(0.0f),
	ticksPerSecond(0.0f),
	startTime(0.0f),
	endTime(0.0f),
	duration(0.0f),
	nodeIndex(0)
{

}

NodeAnimation::NodeAnimation(const std::string& name, Interpolation interp, float ticksPerSecond, float startTime, float endTime, float duration, unsigned int nodeIndex) :
	name(name),
	interpMode(interp),
	currentTime(0.0f),
	ticksPerSecond(ticksPerSecond),
	startTime(startTime),
	endTime(endTime),
	duration(duration),
	nodeIndex(nodeIndex)
{

}

NodeAnimation::~NodeAnimation()
{
	std::cout << "deleted NodeAnimation " << name << std::endl;
}

//void NodeAnimation::setPositions(std::vector<std::pair<float, glm::vec3>>& positions)
//{
//	this->positions = positions;
//}
//
//void NodeAnimation::setRotations(std::vector<std::pair<float, glm::quat>>& rotations)
//{
//	this->rotations = rotations;
//}
//
//void NodeAnimation::setScales(std::vector<std::pair<float, glm::vec3>>& scales)
//{
//	this->scales = scales;
//}

void NodeAnimation::setTimes(std::vector<float>& times)
{
	this->times = times;
}

void NodeAnimation::setPositions(std::vector<glm::vec3>& positions)
{
	this->positions = positions;
}

void NodeAnimation::setRotations(std::vector<glm::quat>& rotations)
{
	this->rotations = rotations;
}

void NodeAnimation::setScales(std::vector<glm::vec3>& scales)
{
	this->scales = scales;
}

unsigned int NodeAnimation::findTime(float currentTime)
{
	for (int i = 0; i < times.size() - 1; i++)
	{
		if (currentTime < times[i + 1])
		{
			return i;
		}
	}
	return 0;
}

//unsigned int NodeAnimation::findPosition(float currentTime)
//{
//	for (int i = 0; i < positions.size() - 1; i++)
//	{
//		if (currentTime < positions[i + 1].first)
//		{
//			return i;
//		}
//	}
//	return 0;
//}
//
//unsigned int NodeAnimation::findRotation(float currentTime)
//{
//	for (int i = 0; i < rotations.size() - 1; i++)
//	{
//		if (currentTime < rotations[i + 1].first)
//		{
//			return i;
//		}
//	}
//	return 0;
//}
//
//unsigned int NodeAnimation::findScaling(float currentTime)
//{
//	for (int i = 0; i < scales.size() - 1; i++)
//	{
//		if (currentTime < scales[i + 1].first)
//		{
//			return i;
//		}
//	}
//	return 0;
//}

glm::mat4 NodeAnimation::calcInterpPosition()
{
	glm::vec3 result(0.0f);
	if (positions.size() == 1)
	{
		result = positions[0];
	}
	else if(positions.size() > 1)
	{
		unsigned int positionIndex = findTime(currentTime);
		unsigned int nextPositionIndex = positionIndex + 1;
		float deltaTime = times[nextPositionIndex] - times[positionIndex];
		float factor = (currentTime - times[positionIndex]) / deltaTime;
		glm::vec3 start = positions[positionIndex];
		glm::vec3 end = positions[nextPositionIndex];
		switch (interpMode)
		{
		case Interpolation::STEP:
			if (factor < 0.5f)
				result = start;
			else
				result = end;
			break;
		case Interpolation::LINEAR:
			result = glm::mix(start, end, factor);
			break;
		case Interpolation::CUBIC:
			glm::vec3 a_k1 = positions[nextPositionIndex * 3];
			glm::vec3 v_k = positions[positionIndex * 3 + 1];
			glm::vec3 v_k1 = positions[nextPositionIndex * 3 + 1];
			glm::vec3 b_k = positions[positionIndex * 3 + 2];

			float t_k = times[positionIndex];
			float t_k1 = times[nextPositionIndex];
			float t_current = currentTime;
			float t = (t_current - t_k) / (t_k1 - t_k);
			glm::vec3 p_0 = v_k;
			glm::vec3 m_0 = (t_k1 - t_k) * b_k;
			glm::vec3 p_1 = v_k1;
			glm::vec3 m_1 = (t_k1 - t_k) * a_k1;

			float t2 = t * t;
			float t3 = t2 * t;
			glm::vec3 p = (2 * t3 - 3 * t2 + 1) * p_0 + 
				(t3 - 2 * t2 + t) * m_0 + 
				(-2 * t3 + 3 * t2) * p_1 + 
				(t3 - t2) * m_1;

			result = p;

			break;
		}		
	}
	pos = result;
	return glm::translate(glm::mat4(1.0f), result);
}

glm::mat4 NodeAnimation::calcInterpRotation()
{
	glm::quat result(1.0f, 0.0f, 0.0f, 0.0f);
	if (rotations.size() == 1)
	{
		result = rotations[0];
	}
	else if(rotations.size() > 1)
	{
		unsigned int rotationIndex = findTime(currentTime);
		unsigned int nextRotationIndex = rotationIndex + 1;
		float deltaTime = times[nextRotationIndex] - times[rotationIndex];
		float factor = (currentTime - times[rotationIndex]) / deltaTime;
		const glm::quat& start = rotations[rotationIndex];
		const glm::quat& end = rotations[nextRotationIndex];
		switch (interpMode)
		{
		case Interpolation::STEP:
			if (factor < 0.5f)
				result = start;
			else
				result = end;
			break;
		case Interpolation::LINEAR:
			result = glm::slerp(start, end, factor);
			break;
		case Interpolation::CUBIC:
			glm::quat a_k1 = rotations[nextRotationIndex * 3];
			glm::quat v_k = rotations[rotationIndex * 3 + 1];
			glm::quat v_k1 = rotations[nextRotationIndex * 3 + 1];
			glm::quat b_k = rotations[rotationIndex * 3 + 2];

			float t_k = times[rotationIndex];
			float t_k1 = times[nextRotationIndex];
			float t_current = currentTime;
			float t = (t_current - t_k) / (t_k1 - t_k);
			glm::quat p_0 = v_k;
			glm::quat m_0 = (t_k1 - t_k) * b_k;
			glm::quat p_1 = v_k1;
			glm::quat m_1 = (t_k1 - t_k) * a_k1;

			float t2 = t * t;
			float t3 = t2 * t;
			glm::quat q = (2 * t3 - 3 * t2 + 1) * p_0 + 
				(t3 - 2 * t2 + t) * m_0 + 
				(-2 * t3 + 3 * t2) * p_1 + 
				(t3 - t2) * m_1;

			result = glm::normalize(q);

			break;
		}
	}
	rot = result;
	return glm::mat4_cast(result);
}

glm::mat4 NodeAnimation::calcInterpScaling()
{
	glm::vec3 result(1.0f);
	if (scales.size() == 1)
	{
		result = scales[0];
	}
	else if(scales.size() > 1)
	{
		unsigned int scaleIndex = findTime(currentTime);
		unsigned int nextScaleIndex = scaleIndex + 1;
		float deltaTime = times[nextScaleIndex] - times[scaleIndex];
		float factor = (currentTime - times[scaleIndex]) / deltaTime;
		glm::vec3 start = scales[scaleIndex];
		glm::vec3 end = scales[nextScaleIndex];
		switch (interpMode)
		{
		case Interpolation::STEP:
			if (factor < 0.5f)
				result = start;
			else
				result = end;
			break;
		case Interpolation::LINEAR:
			result = glm::mix(start, end, factor);
			break;
		case Interpolation::CUBIC:
			glm::vec3 a_k1 = scales[nextScaleIndex * 3];
			glm::vec3 v_k = scales[scaleIndex * 3 + 1];
			glm::vec3 v_k1 = scales[nextScaleIndex * 3 + 1];
			glm::vec3 b_k = scales[scaleIndex * 3 + 2];

			float t_k = times[scaleIndex];
			float t_k1 = times[nextScaleIndex];
			float t_current = currentTime;
			float t = (t_current - t_k) / (t_k1 - t_k);
			glm::vec3 p_0 = v_k;
			glm::vec3 m_0 = (t_k1 - t_k) * b_k;
			glm::vec3 p_1 = v_k1;
			glm::vec3 m_1 = (t_k1 - t_k) * a_k1;

			float t2 = t * t;
			float t3 = t2 * t;
			glm::vec3 p = (2 * t3 - 3 * t2 + 1) * p_0 + 
				(t3 - 2 * t2 + t) * m_0 + 
				(-2 * t3 + 3 * t2) * p_1 + 
				(t3 - t2) * m_1;

			result = p;

			break;
		}
	}
	scale = result;
	return glm::scale(glm::mat4(1.0f), result);
}

//glm::mat4 NodeAnimation::calcInterpPosition()
//{
//	glm::vec3 result(0.0f);
//	if (positions.size() == 1)
//	{
//		result = positions[0].second;
//	}
//	else if(positions.size() > 1)
//	{
//		unsigned int positionIndex = findPosition(currentTime);
//		unsigned int nextPositionIndex = positionIndex + 1;
//		float deltaTime = positions[nextPositionIndex].first - positions[positionIndex].first;
//		float factor = (currentTime - positions[positionIndex].first) / deltaTime;
//		glm::vec3 start = positions[positionIndex].second;
//		glm::vec3 end = positions[nextPositionIndex].second;
//		result = glm::mix(start, end, factor);
//	}
//	pos = result;
//	return glm::translate(glm::mat4(1.0f), result);
//}
//
//glm::mat4 NodeAnimation::calcInterpRotation()
//{
//	glm::quat result(1.0f, 0.0f, 0.0f, 0.0f);
//	if (rotations.size() == 1)
//	{
//		result = rotations[0].second;
//	}
//	else if(rotations.size() > 1)
//	{
//		unsigned int rotationIndex = findRotation(currentTime);
//		unsigned int nextRotationIndex = rotationIndex + 1;
//		float deltaTime = rotations[nextRotationIndex].first - rotations[rotationIndex].first;
//		float factor = (currentTime - rotations[rotationIndex].first) / deltaTime;
//		const glm::quat& start = rotations[rotationIndex].second;
//		const glm::quat& end = rotations[nextRotationIndex].second;
//		result = glm::slerp(start, end, factor);
//	}
//	rot = result;
//	return glm::mat4_cast(result);
//}
//
//glm::mat4 NodeAnimation::calcInterpScaling()
//{
//	glm::vec3 result(1.0f);
//	if (scales.size() == 1)
//	{
//		result = scales[0].second;
//	}
//	else if(scales.size() > 1)
//	{
//		unsigned int scaleIndex = findScaling(currentTime);
//		unsigned int nextScaleIndex = scaleIndex + 1;
//		float deltaTime = scales[nextScaleIndex].first - scales[scaleIndex].first;
//		float factor = (currentTime - scales[scaleIndex].first) / deltaTime;
//		glm::vec3 start = scales[scaleIndex].second;
//		glm::vec3 end = scales[nextScaleIndex].second;
//		result = glm::mix(start, end, factor);
//	}
//	scale = result;
//	return glm::scale(glm::mat4(1.0f), result);
//}

void NodeAnimation::update(float time)
{
	currentTime = fmodf(time, duration);
	if (currentTime < startTime || currentTime > endTime)
		currentTime = 0.0f;
	glm::mat4 translation = calcInterpPosition();
	glm::mat4 rotation = calcInterpRotation();
	glm::mat4 scale = calcInterpScaling();
	//currentTransform = translation * rotation * scale;
}

bool NodeAnimation::hasPositions()
{
	return (!positions.empty());
}

bool NodeAnimation::hasRotations()
{
	return (!rotations.empty());
}

bool NodeAnimation::hasScale()
{
	return (!scales.empty());
}

glm::vec3 NodeAnimation::getPos()
{
	return pos;
}

glm::quat NodeAnimation::getRot()
{
	return rot;
}

glm::vec3 NodeAnimation::getScale()
{
	return scale;
}

std::string NodeAnimation::getName()
{
	return name;
}

unsigned int NodeAnimation::getNodeIndex()
{
	return nodeIndex;
}

//glm::mat4 NodeAnimation::getTransform()
//{
//	return currentTransform;
//}