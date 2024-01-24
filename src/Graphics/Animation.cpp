#include "Animation.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>

Animation::Animation() :
	name(""),
	currentTime(0.0f),
	duration(0.0f)
{

}

Animation::Animation(const std::string& name) :
	name(name),
	currentTime(0.0f),
	duration(0.0f)
{

}

Animation::~Animation()
{
	//std::cout << "deleted Animation " << name << std::endl;
}

void Animation::addChannel(IChannel::Ptr channel)
{
	channels.push_back(channel);
}

void Animation::setDuration(float duration)
{
	this->duration = duration;
}

std::vector<float> Animation::getWeights()
{
	return currentWeights;
}

void Animation::setCameraProperty(IChannel::Ptr iChannel, Camera::Ptr camera, AnimAttribute attribute)
{
	auto channel = std::dynamic_pointer_cast<Channel<float>>(iChannel);
	float value = channel->interpolate(currentTime);
	switch (attribute)
	{
		case AnimAttribute::CAMERA_ASPECT: camera->setAspectRatio(value); break;
		case AnimAttribute::CAMERA_YFOV: camera->setFieldOfView(glm::radians(value)); break;
		case AnimAttribute::CAMERA_ZNEAR: camera->setNearPlane(value); break;
		case AnimAttribute::CAMERA_ZFAR: camera->setFarPlane(value); break;
	}
}

void Animation::setLightProperty(IChannel::Ptr iChannel, Light::Ptr light, AnimAttribute attribute)
{
	switch (attribute)
	{
		case AnimAttribute::LIGHT_COLOR:
		{
			auto channel = std::dynamic_pointer_cast<Channel<glm::vec3>>(iChannel);
			glm::vec3 lightColor = channel->interpolate(currentTime);
			light->setColorLinear(lightColor);
			break;
		}
		case AnimAttribute::LIGHT_INTENSITY:
		{
			auto channel = std::dynamic_pointer_cast<Channel<float>>(iChannel);
			float lightIntensity = channel->interpolate(currentTime);
			light->setLuminousIntensity(lightIntensity);
			break;
		}
		case AnimAttribute::LIGHT_RANGE:
		{
			auto channel = std::dynamic_pointer_cast<Channel<float>>(iChannel);
			float lightRange = channel->interpolate(currentTime);
			light->setRange(lightRange);
			break;
		}
		case AnimAttribute::LIGHT_INNERANGLE:
		{
			auto channel = std::dynamic_pointer_cast<Channel<float>>(iChannel);
			float innerAngle = channel->interpolate(currentTime);
			light->setInnerAngle(innerAngle);
			break;
		}
		case AnimAttribute::LIGHT_OUTERANGLE:
		{
			auto channel = std::dynamic_pointer_cast<Channel<float>>(iChannel);
			float outerAngle = channel->interpolate(currentTime);
			light->setOuterAngle(outerAngle);
			break;
		}
	}
}

void Animation::setTransformProperty(IChannel::Ptr iChannel, Transform::Ptr transform, AnimAttribute attribute)
{
	switch (attribute)
	{
		case AnimAttribute::TRANSFORM_POSITION:
		{
			auto channel = std::dynamic_pointer_cast<Channel<glm::vec3>>(iChannel);
			glm::vec3 translation = channel->interpolate(currentTime);
			transform->setLocalPosition(translation);
			break;
		}
		case AnimAttribute::TRANSFORM_ROTATION:
		{
			auto channel = std::dynamic_pointer_cast<Channel<glm::quat>>(iChannel);
			glm::quat rotation = glm::normalize(channel->interpolate(currentTime));
			transform->setLocalRotation(rotation);
			break;
		}
		case AnimAttribute::TRANSFORM_SCALE:
		{
			auto channel = std::dynamic_pointer_cast<Channel<glm::vec3>>(iChannel);
			glm::vec3 scale = channel->interpolate(currentTime);
			transform->setLocalScale(scale);
			break;
		}
		case AnimAttribute::TRANSFORM_WEIGHTS:
		{
			auto channel = std::dynamic_pointer_cast<Channel<float>>(iChannel);
			currentWeights = channel->interpolateElements(currentTime);
			break;
		}
	}
}

void Animation::setMaterialProperty(IChannel::Ptr channel, Material::Ptr material, AnimAttribute attribute)
{
	AnimAttribute attr = static_cast<AnimAttribute>(attribute & AnimAttribute::MATERIAL_MASK);
	switch (attr)
	{
		case AnimAttribute::MATERIAL_BASECOLOR: setPropertyValue<glm::vec4>(channel, material, "material.baseColorFactor"); break;
		case AnimAttribute::MATERIAL_ROUGHNESS: setPropertyValue<float>(channel, material, "material.roughnessFactor"); break;
		case AnimAttribute::MATERIAL_METALLIC: setPropertyValue<float>(channel, material, "material.metallicFactor"); break;
		case AnimAttribute::MATERIAL_EMISSIVE_FACTOR: setPropertyValue<glm::vec3>(channel, material, "material.emissiveFactor"); break;
		case AnimAttribute::MATERIAL_EMISSIVE_STRENGTH: setPropertyValue<float>(channel, material, "material.emissiveStrength"); break;
		case AnimAttribute::MATERIAL_ALPHA_CUTOFF: setPropertyValue<float>(channel, material, "material.alphaCutOff"); break;
		case AnimAttribute::MATERIAL_BASECOLORTEXTURE: setTextureTransform(channel, material, attribute, "baseColorTex"); break;
		case AnimAttribute::MATERIAL_EMISSIVETEXTURE: setTextureTransform(channel, material, attribute, "emissiveTex"); break;
		case AnimAttribute::MATERIAL_NORMAL_SCALE: setPropertyValue<float>(channel, material, "material.normalScale"); break;
		case AnimAttribute::MATERIAL_OCCLUSION_STRENGTH: setPropertyValue<float>(channel, material, "material.occlusionStrength"); break;
		case AnimAttribute::MATERIAL_IOR: setPropertyValue<float>(channel, material, "material.ior"); break;
		case AnimAttribute::MATERIAL_TRANSMISSION_FACTOR: setPropertyValue<float>(channel, material, "material.transmissionFactor"); break;
		case AnimAttribute::MATERIAL_THICKNESS_FACTOR: setPropertyValue<float>(channel, material, "material.thicknessFactor"); break;
		case AnimAttribute::MATERIAL_ATTENUATION_DISTANCE: setPropertyValue<float>(channel, material, "material.attenuationDistance"); break;
		case AnimAttribute::MATERIAL_ATTENUATION_COLOR: setPropertyValue<glm::vec3>(channel, material, "material.attenuationColor"); break;
		case AnimAttribute::MATERIAL_IRIDESCENCE_FACTOR: setPropertyValue<float>(channel, material, "material.iridescenceFactor"); break;
		case AnimAttribute::MATERIAL_IRIDESCENCE_IOR: setPropertyValue<float>(channel, material, "material.iridescenceIor"); break;
		case AnimAttribute::MATERIAL_IRIDESCENCE_THICKNESS_MAXIMUM: setPropertyValue<float>(channel, material, "material.iridescenceThicknessMax"); break;
	}
}

void Animation::setTextureTransform(IChannel::Ptr iChannel, Material::Ptr material, AnimAttribute attribute, std::string texUniform)
{
	AnimAttribute texAttr = static_cast<AnimAttribute>(attribute & AnimAttribute::TEXTURE_MASK);
	switch (texAttr)
	{
		case AnimAttribute::TEXTURE_OFFSET:
		{
			auto channel = std::dynamic_pointer_cast<Channel<glm::vec2>>(iChannel);
			glm::vec2 offset = channel->interpolate(currentTime);

			auto tex = material->getTexture(texUniform);
			tex->setOffset(offset);
			material->addProperty(texUniform + ".uvTransform", tex->getUVTransform());
			break;
		}
		case AnimAttribute::TEXTURE_SCALE:
		{
			auto ch = std::dynamic_pointer_cast<Channel<glm::vec2>>(iChannel);
			glm::vec2 scale = ch->interpolate(currentTime);

			auto tex = material->getTexture(texUniform);
			tex->setScale(scale);
			material->addProperty(texUniform + ".uvTransform", tex->getUVTransform());
			break;
		}
	}
}

// TODO: more generic way to get the component to be animated
// TODO: add all specified animateable properties from KHR_animation_pointer
void Animation::update(float dt, std::vector<Entity::Ptr>& nodes, std::vector<Material::Ptr>& materials, std::vector<Light::Ptr>& lights, std::vector<Camera::Ptr>& cameras)
{
	//std::cout << "duration: " << duration << std::endl;
	// TODO: update the animation to the last keyframe
	currentTime += dt;
	if (currentTime > duration)
	{
		finished = true;
		return;
	}
	for (auto channel : channels)
	{
		unsigned int index = channel->getTargetIndex();
		auto attribType = channel->getAttributeType();
		auto transform = nodes[index]->getComponent<Transform>();

		if (attribType & AnimAttribute::CAMERA_MASK)
			setCameraProperty(channel, cameras[index], attribType);
		else if (attribType & AnimAttribute::MATERIAL_MASK)
			setMaterialProperty(channel, materials[index], attribType);
		else if (attribType & AnimAttribute::LIGHT_MASK)
			setLightProperty(channel, lights[index], attribType);
		else if (attribType & AnimAttribute::TRANSFORM_MASK)
			setTransformProperty(channel, transform, attribType);
	}
}

void Animation::reset()
{
	currentTime = offset;
	finished = false;
}

bool Animation::isFinished()
{
	return finished;
}

void Animation::print()
{
	std::cout << "duration: " << duration << std::endl;
	//for (auto&& [idx, ch] : channels)
	//	std::cout << "target node index " << idx
	//	<< " pos keys " << ch.positions.size()
	//	<< " rot keys " << ch.rotations.size()
	//	<< " scale keys " << ch.scales.size()
	//	<< std::endl;
}

std::string Animation::getName()
{
	return name;
}