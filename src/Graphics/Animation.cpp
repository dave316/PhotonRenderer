#include "Animation.h"

namespace pr
{
	Animation::Animation(const std::string& name) : name(name)
	{

	}

	void Animation::addChannel(IChannel::Ptr channel)
	{
		channels.push_back(channel);
	}

	void Animation::setDuration(float duration)
	{
		this->duration = duration;
	}

	void Animation::setMaterialProperty(IChannel::Ptr channel, Material::Ptr material, AnimAttribute attribute)
	{
		AnimAttribute attr = static_cast<AnimAttribute>(attribute & AnimAttribute::MATERIAL_MASK);
		switch (attr)
		{
			case AnimAttribute::MATERIAL_BASECOLOR: setPropertyValue<glm::vec4>(channel, material, "baseColor"); break;
			case AnimAttribute::MATERIAL_ROUGHNESS: setPropertyValue<float>(channel, material, "roughness"); break;
			case AnimAttribute::MATERIAL_METALLIC: setPropertyValue<float>(channel, material, "metallic"); break;
			case AnimAttribute::MATERIAL_EMISSIVE_FACTOR: setPropertyValue<glm::vec3>(channel, material, "material.emissiveFactor"); break;
			case AnimAttribute::MATERIAL_EMISSIVE_STRENGTH: setPropertyValue<float>(channel, material, "material.emissiveStrength"); break;
			case AnimAttribute::MATERIAL_ALPHA_CUTOFF: setPropertyValue<float>(channel, material, "alphaCutOff"); break;
			case AnimAttribute::MATERIAL_NORMAL_SCALE: setPropertyValue<float>(channel, material, "normalScale"); break;
			case AnimAttribute::MATERIAL_OCCLUSION_STRENGTH: setPropertyValue<float>(channel, material, "occlusionStrength"); break;
			case AnimAttribute::MATERIAL_IOR: setPropertyValue<float>(channel, material, "ior"); break;
			case AnimAttribute::MATERIAL_TRANSMISSION_FACTOR: setPropertyValue<float>(channel, material, "transmissionFactor"); break;
			case AnimAttribute::MATERIAL_THICKNESS_FACTOR: setPropertyValue<float>(channel, material, "thicknessFactor"); break;
			case AnimAttribute::MATERIAL_ATTENUATION_DISTANCE: setPropertyValue<float>(channel, material, "attenuationDistance"); break;
			case AnimAttribute::MATERIAL_ATTENUATION_COLOR: setPropertyValue<glm::vec3>(channel, material, "attenuationColor"); break;
			case AnimAttribute::MATERIAL_IRIDESCENCE_FACTOR: setPropertyValue<float>(channel, material, "iridescenceFactor"); break;
			case AnimAttribute::MATERIAL_IRIDESCENCE_IOR: setPropertyValue<float>(channel, material, "iridescenceIor"); break;
			case AnimAttribute::MATERIAL_IRIDESCENCE_THICKNESS_MAXIMUM: setPropertyValue<float>(channel, material, "iridescenceThicknessMax"); break;

			case AnimAttribute::MATERIAL_BASECOLOR_TEXTURE: setTextureTransform(channel, material, attribute, "baseColorTex"); break;
			case AnimAttribute::MATERIAL_METALROUGH_TEXTURE: setTextureTransform(channel, material, attribute, "metalRoughTex"); break;
			case AnimAttribute::MATERIAL_NORMAL_TEXTURE: setTextureTransform(channel, material, attribute, "normalTex"); break;
			case AnimAttribute::MATERIAL_OCCLUSION_TEXTURE: setTextureTransform(channel, material, attribute, "occlusionTex"); break;
			case AnimAttribute::MATERIAL_EMISSIVE_TEXTURE: setTextureTransform(channel, material, attribute, "emissiveTex"); break;
			case AnimAttribute::MATERIAL_SHEEN_COLOR_TEXTURE: setTextureTransform(channel, material, attribute, "sheenColorTex"); break;
			case AnimAttribute::MATERIAL_SHEEN_ROUGHNESS_TEXTURE: setTextureTransform(channel, material, attribute, "sheenRoughnessTex"); break;
			case AnimAttribute::MATERIAL_CLEARCOAT_TEXTURE: setTextureTransform(channel, material, attribute, "clearcoatTex"); break;
			case AnimAttribute::MATERIAL_CLEARCOAT_ROUGHNESS_TEXTURE: setTextureTransform(channel, material, attribute, "clearcoatRoughnessTex"); break;
			case AnimAttribute::MATERIAL_CLEARCOAT_NORMAL_TEXTURE: setTextureTransform(channel, material, attribute, "clearcoatNormalTex"); break;
			case AnimAttribute::MATERIAL_TRANSMISSION_TEXTURE: setTextureTransform(channel, material, attribute, "transmissionTex"); break;
			case AnimAttribute::MATERIAL_THICKNESS_TEXTURE: setTextureTransform(channel, material, attribute, "thicknessTex"); break;
			case AnimAttribute::MATERIAL_SPECULAR_TEXTURE: setTextureTransform(channel, material, attribute, "specularTex"); break;
			case AnimAttribute::MATERIAL_SPECULAR_COLOR_TEXTURE: setTextureTransform(channel, material, attribute, "specularColorTex"); break;
			case AnimAttribute::MATERIAL_IRIDESCENCE_TEXTURE: setTextureTransform(channel, material, attribute, "iridescenceTex"); break;
			case AnimAttribute::MATERIAL_IRIDESCENCE_THICKNESS_TEXTURE: setTextureTransform(channel, material, attribute, "iridescenceThicknessTex"); break;
			case AnimAttribute::MATERIAL_ANISOTROPY_TEXTURE: setTextureTransform(channel, material, attribute, "anisotropyTex"); break;
			case AnimAttribute::MATERIAL_DIFFUSE_TRANSMISSION_TEXTURE: setTextureTransform(channel, material, attribute, "translucencyTex"); break;
			case AnimAttribute::MATERIAL_DIFFUSE_TRANSMISSION_COLOR_TEXTURE: setTextureTransform(channel, material, attribute, "translucencyColorTex"); break;
		}
	}

	void Animation::setTextureTransform(IChannel::Ptr iChannel, Material::Ptr material, AnimAttribute attribute, std::string texUniform)
	{
		AnimAttribute texAttr = static_cast<AnimAttribute>(attribute & AnimAttribute::TEXTURE_MASK);
		switch (texAttr)
		{
			case AnimAttribute::TEXTURE_OFFSET:
			{
				auto ch = std::dynamic_pointer_cast<Channel<glm::vec2>>(iChannel);
				glm::vec2 offset = ch->interpolate(currentTime);
				material->setTextureOffset(texUniform, offset);
				break;
			}
			case AnimAttribute::TEXTURE_ROTATION:
			{
				auto ch = std::dynamic_pointer_cast<Channel<float>>(iChannel);
				float rotation = ch->interpolate(currentTime);
				material->setTextureRotation(texUniform, rotation);
				break;
			}
			case AnimAttribute::TEXTURE_SCALE:
			{
				auto ch = std::dynamic_pointer_cast<Channel<glm::vec2>>(iChannel);
				glm::vec2 scale = ch->interpolate(currentTime);
				material->setTextureScale(texUniform, scale);
				break;
			}
		}
	}

	void Animation::update(float dt, std::map<uint32, pr::Component::Ptr> components)
	{
		currentTime += dt;
		if (currentTime > duration)
		{
			currentTime = 0.0f;
			return;
		}

		for (auto ch : channels)
		{
			uint32 targetIndex = ch->getTargetIndex();
			auto animAttr = ch->getAttributeType();
			//auto transform = nodes[targetIndex]->getComponent<Transform>();
			if (animAttr & pr::AnimAttribute::TRANSFORM_MASK)
			{
				auto transform = std::dynamic_pointer_cast<Transform>(components[targetIndex]);
				switch (animAttr)
				{
					case AnimAttribute::TRANSFORM_POSITION:
					{
						auto channel = std::dynamic_pointer_cast<Channel<glm::vec3>>(ch);
						glm::vec3 position = channel->interpolate(currentTime);
						transform->setLocalPosition(position);
						break;
					}
					case AnimAttribute::TRANSFORM_ROTATION:
					{
						auto channel = std::dynamic_pointer_cast<Channel<glm::quat>>(ch);
						glm::quat rotation = channel->interpolate(currentTime);
						transform->setLocalRotation(rotation);
						break;
					}
					case AnimAttribute::TRANSFORM_SCALE:
					{
						auto channel = std::dynamic_pointer_cast<Channel<glm::vec3>>(ch);
						glm::vec3 scale = channel->interpolate(currentTime);
						transform->setLocalScale(scale);
						break;
					}
					case AnimAttribute::TRANSFORM_WEIGHTS:
					{
						auto channel = std::dynamic_pointer_cast<Channel<float>>(ch);
						currentWeights = channel->interpolateElements(currentTime);
						break;
					}
				}
			}
			else if (animAttr & pr::AnimAttribute::MATERIAL_MASK)
			{
				auto material = std::dynamic_pointer_cast<Material>(components[targetIndex]);
				setMaterialProperty(ch, material, animAttr);
				material->update();
			}
		}
	}

	std::vector<float> Animation::getCurrentWeights()
	{
		return currentWeights;
	}
}
