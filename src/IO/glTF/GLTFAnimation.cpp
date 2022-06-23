#include "GLTFAnimation.h"

#include <sstream>

namespace IO
{
	namespace glTF
	{
		Skin::Ptr loadSkin(const json::Value& skinNode, BinaryData& data)
		{
			std::vector<int> boneJoints;
			std::vector<glm::mat4> boneMatrices;
			if (skinNode.HasMember("inverseBindMatrices"))
			{
				int accIndex = skinNode["inverseBindMatrices"].GetInt();
				data.loadData(accIndex, boneMatrices);
			}
			if (skinNode.HasMember("joints"))
			{
				for (auto& jointNode : skinNode["joints"].GetArray())
				{
					boneJoints.push_back(jointNode.GetInt());
				}
			}

			std::string name;
			if (skinNode.HasMember("name"))
				name = skinNode["name"].GetString();

			auto skin = Skin::create(name);
			for (int i = 0; i < boneJoints.size(); i++)
				skin->addJoint(boneJoints[i], boneMatrices[i]);
			if (skinNode.HasMember("skeleton"))
				skin->setSkeleton(skinNode["skeleton"].GetInt());

			return skin;
		}

		Animation::Ptr loadAnimation(const json::Value& animNode, BinaryData& data)
		{
			std::string name = "animation";
			if (animNode.HasMember("name"))
				name = animNode["name"].GetString();

			auto animation = Animation::create(name);
			float minTime = 1000.0f;
			float maxTime = 0.0f;

			std::vector<AnimationSampler> animSamplers;
			for (auto& samplerNode : animNode["samplers"].GetArray())
			{
				AnimationSampler sampler;
				sampler.input = samplerNode["input"].GetInt();
				sampler.output = samplerNode["output"].GetInt();
				if (samplerNode.HasMember("interpolation"))
				{
					std::string interpolation = samplerNode["interpolation"].GetString();
					if (interpolation.compare("STEP") == 0)
						sampler.interpolation = Interpolation::STEP;
					else if (interpolation.compare("LINEAR") == 0)
						sampler.interpolation = Interpolation::LINEAR;
					else if (interpolation.compare("CUBICSPLINE") == 0)
						sampler.interpolation = Interpolation::CUBIC;
					else
						sampler.interpolation = Interpolation::LINEAR;
				}
				animSamplers.push_back(sampler);
			}

			std::vector<AnimationChannel> animChannels;
			for (auto& channelNode : animNode["channels"].GetArray())
			{
				AnimationChannel channel;
				channel.sampler = channelNode["sampler"].GetInt();
				auto targetNode = channelNode["target"].GetObject();
				if (targetNode.HasMember("node"))
					channel.targetNode = targetNode["node"].GetInt();
				channel.targetPath = targetNode["path"].GetString();
				if (channel.targetPath.compare("pointer") == 0)
				{
					// TODO: check if present etc.
					channel.targetPointer = targetNode["extensions"]["KHR_animation_pointer"]["pointer"].GetString();
				}
				animChannels.push_back(channel);
			}

			std::vector<IChannel::Ptr> channels;
			for (auto& channel : animChannels)
			{
				AnimationSampler& sampler = animSamplers[channel.sampler];
				std::string& path = channel.targetPath;

				if (path.compare("translation") == 0)
					channels.push_back(loadChannel<glm::vec3>(data, sampler, AnimAttribute::TRANSFORM_POSITION, channel.targetNode));
				else if (path.compare("rotation") == 0)
					channels.push_back(loadChannel<glm::quat>(data, sampler, AnimAttribute::TRANSFORM_ROTATION, channel.targetNode));
				else if (path.compare("scale") == 0)
					channels.push_back(loadChannel<glm::vec3>(data, sampler, AnimAttribute::TRANSFORM_SCALE, channel.targetNode));
				else if (path.compare("weights") == 0)
					channels.push_back(loadChannel<float>(data, sampler, AnimAttribute::TRANSFORM_WEIGHTS, channel.targetNode));
				else if (path.compare("pointer") == 0)
					channels.push_back(loadPointer(data, sampler, channel));
			}

			float duration = 0.0f;
			for (auto channel : channels)
			{
				if (channel)
				{
					duration = glm::max(duration, channel->getMaxTime());
					animation->addChannel(channel);
				}
			}
			animation->setDuration(duration);
			return animation;
		}

		IChannel::Ptr loadTexTransform(BinaryData& data, AnimationSampler& sampler, AnimAttribute texAttribute, std::string texTransform, int matIndex)
		{
			if (texTransform.compare("offset") == 0)
				return loadChannel<glm::vec2>(data, sampler, static_cast<AnimAttribute>(texAttribute | AnimAttribute::TEXTURE_OFFSET), matIndex);
			else if (texTransform.compare("rotation") == 0)
				return loadChannel<float>(data, sampler, static_cast<AnimAttribute>(texAttribute | AnimAttribute::TEXTURE_ROTATION), matIndex);
			else if (texTransform.compare("scale") == 0)
				return loadChannel<glm::vec2>(data, sampler, static_cast<AnimAttribute>(texAttribute | AnimAttribute::TEXTURE_SCALE), matIndex);
		}

		IChannel::Ptr loadPointer(BinaryData& data, AnimationSampler& sampler, AnimationChannel& channel)
		{
			std::stringstream ss(channel.targetPointer);
			std::string element;
			std::getline(ss, element, '/');
			std::vector<std::string> elements;
			while (std::getline(ss, element, '/'))
				elements.push_back(element);

			// TODO: better error checking/parsing of pointer

			std::string group = elements[0];
			if (group.compare("cameras") == 0)
			{
				int camIndex = std::stoi(elements[1]);
				std::string camType = elements[2];
				if (camType.compare("perspective") == 0)
				{
					std::string attribute = elements[3];
					if (attribute.compare("aspect") == 0)
						return loadChannel<float>(data, sampler, AnimAttribute::CAMERA_ASPECT, camIndex);
					else if (attribute.compare("yfov") == 0)
						return loadChannel<float>(data, sampler, AnimAttribute::CAMERA_YFOV, camIndex);
					else if (attribute.compare("znear") == 0)
						return loadChannel<float>(data, sampler, AnimAttribute::CAMERA_ZNEAR, camIndex);
					else if (attribute.compare("zfar") == 0)
						return loadChannel<float>(data, sampler, AnimAttribute::CAMERA_ZFAR, camIndex);
				}
				else if (camType.compare("orthographic") == 0)
				{
					// TODO: add orthographic camera animation
				}
			}
			else if (group.compare("extensions") == 0)
			{
				std::string extension = elements[1];
				if (extension.compare("KHR_lights_punctual") == 0)
				{
					int lightIndex = std::stoi(elements[3]);
					std::string attribute = elements[4];
					if (attribute.compare("color") == 0) // TODO: colors are always vec4???
						return loadChannel<glm::vec4>(data, sampler, AnimAttribute::LIGHT_COLOR, lightIndex);
					else if (attribute.compare("intensity") == 0)
						return loadChannel<float>(data, sampler, AnimAttribute::LIGHT_INTENSITY, lightIndex);
					else if (attribute.compare("range") == 0)
						return loadChannel<float>(data, sampler, AnimAttribute::LIGHT_RANGE, lightIndex);
					else if (attribute.compare("spot") == 0)
					{
						std::string angleAttrib = elements[5];
						if (angleAttrib.compare("innerConeAngle"))
							return loadChannel<float>(data, sampler, AnimAttribute::LIGHT_INNERANGLE, lightIndex);
						else if (angleAttrib.compare("outerConeAngle"))
							return loadChannel<float>(data, sampler, AnimAttribute::LIGHT_OUTERANGLE, lightIndex);
					}
				}
			}
			else if (group.compare("materials") == 0)
			{
				int matIndex = std::stoi(elements[1]);
				std::string matType = elements[2];
				if (matType.compare("extensions") == 0)
				{
					std::string extension = elements[3];
					std::string extAttribute = elements[4];
					if (extension.compare("KHR_materials_emissive_strength") == 0)
					{
						if (extAttribute.compare("emissiveStrength") == 0)
							return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_EMISSIVE_STRENGTH, matIndex);
					}
					else if (extension.compare("KHR_materials_transmission") == 0)
					{
						if (extAttribute.compare("transmissionFactor") == 0)
							return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_TRANSMISSION_FACTOR, matIndex);
					}
					else if (extension.compare("KHR_materials_ior") == 0)
					{
						if (extAttribute.compare("ior") == 0)
							return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_IOR, matIndex);
					}
					else if (extension.compare("KHR_materials_volume") == 0)
					{
						if (extAttribute.compare("thicknessFactor") == 0)
							return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_THICKNESS_FACTOR, matIndex);
						else if (extAttribute.compare("attenuationDistance") == 0)
							return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_ATTENUATION_DISTANCE, matIndex);
						else if (extAttribute.compare("attenuationColor") == 0)
							return loadChannel<glm::vec4>(data, sampler, AnimAttribute::MATERIAL_ATTENUATION_COLOR, matIndex);
					}
					else if (extension.compare("KHR_materials_iridescence") == 0)
					{
						if (extAttribute.compare("iridescenceFactor") == 0)
							return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_IRIDESCENCE_FACTOR, matIndex);
						else if (extAttribute.compare("iridescenceIor") == 0)
							return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_IRIDESCENCE_IOR, matIndex);
						else if (extAttribute.compare("iridescenceThicknessMaximum") == 0)
							return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_IRIDESCENCE_THICKNESS_MAXIMUM, matIndex);
					}
				}
				else if (matType.compare("pbrMetallicRoughness") == 0)
				{
					std::string attribute = elements[3];
					if (attribute.compare("baseColorFactor") == 0)
						return loadChannel<glm::vec4>(data, sampler, AnimAttribute::MATERIAL_BASECOLOR, matIndex);
					else if (attribute.compare("roughnessFactor") == 0)
						return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_ROUGHNESS, matIndex);
					else if (attribute.compare("metallicFactor") == 0)
						return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_METALLIC, matIndex);
					else if (attribute.compare("baseColorTexture") == 0)
					{
						std::string extension = elements[5];
						if (extension.compare("KHR_texture_transform") == 0)
						{
							AnimAttribute attr = AnimAttribute::MATERIAL_BASECOLORTEXTURE;
							std::string texTransform = elements[6];
							return loadTexTransform(data, sampler, attr, texTransform, matIndex);
						}
					}

				}
				else if (matType.compare("alphaCutoff") == 0)
					return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_ALPHA_CUTOFF, matIndex);
				else if (matType.compare("emissiveFactor") == 0)
					return loadChannel<glm::vec4>(data, sampler, AnimAttribute::MATERIAL_EMISSIVE_FACTOR, matIndex);
				else if (matType.compare("emissiveTexture") == 0)
				{
					std::string extension = elements[4];
					if (extension.compare("KHR_texture_transform") == 0)
					{
						AnimAttribute attr = AnimAttribute::MATERIAL_EMISSIVETEXTURE;
						std::string texTransform = elements[5];
						return loadTexTransform(data, sampler, attr, texTransform, matIndex);
					}
				}
				else if (matType.compare("normalTexture") == 0)
				{
					std::string attribute = elements[3];
					if (attribute.compare("extensions") == 0)
					{
						std::string extension = elements[4];
						if (extension.compare("KHR_texture_transform") == 0)
						{
							AnimAttribute attr = AnimAttribute::MATERIAL_NORMALTEXTURE;
							std::string texTransform = elements[5];
							return loadTexTransform(data, sampler, attr, texTransform, matIndex);
						}
					}
					else if (attribute.compare("scale") == 0)
						return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_NORMAL_SCALE, matIndex);
				}
				else if (matType.compare("occlusionTexture") == 0)
				{
					std::string attribute = elements[3];
					if (attribute.compare("extensions") == 0)
					{
						std::string extension = elements[4];
						if (extension.compare("KHR_texture_transform") == 0)
						{
							AnimAttribute attr = AnimAttribute::MATERIAL_OCCLUSIONTEXTURE;
							std::string texTransform = elements[5];
							return loadTexTransform(data, sampler, attr, texTransform, matIndex);
						}
					}
					else if (attribute.compare("strength") == 0)
						return loadChannel<float>(data, sampler, AnimAttribute::MATERIAL_OCCLUSION_STRENGTH, matIndex);
				}
			}
			else if (group.compare("nodes") == 0)
			{
				int nodeIndex = std::stoi(elements[1]);
				std::string attribute = elements[2];
				if (attribute.compare("translation") == 0)
					return loadChannel<glm::vec3>(data, sampler, AnimAttribute::TRANSFORM_POSITION, nodeIndex);
				else if (attribute.compare("rotation") == 0)
					return loadChannel<glm::quat>(data, sampler, AnimAttribute::TRANSFORM_ROTATION, nodeIndex);
				else if (attribute.compare("scale") == 0)
					return loadChannel<glm::vec3>(data, sampler, AnimAttribute::TRANSFORM_SCALE, nodeIndex);
				else if (attribute.compare("weights") == 0)
					return loadChannel<float>(data, sampler, AnimAttribute::TRANSFORM_WEIGHTS, nodeIndex);
			}

			std::cout << "pointer " << channel.targetPointer << " not supported!" << std::endl;

			return nullptr;
		}
	}
}
