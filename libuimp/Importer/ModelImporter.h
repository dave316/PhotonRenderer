#ifndef INCLUDED_MODELIMPORTER
#define INCLUDED_MODELIMPORTER

#pragma once

#include "Importer.h"

namespace Unity
{
	namespace YAML
	{
		struct Materials
		{
		public:
			Materials();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, Materials& value)
			{
				value.print("    ");
				return stream;
			}

		private:
			YamlField<uint32> importMaterials = { "importMaterials", 0 };
			YamlField<uint32> materialName = { "materialName", 0 };
			YamlField<uint32> materialSearch = { "materialSearch", 0 };
			YamlField<uint32> materialLocation = { "materialLocation", 0 };
		};

		struct Animations
		{
		public:
			Animations();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, Animations& value)
			{
				value.print("    ");
				return stream;
			}

		private:
			YamlField<uint32> legacyGenerateAnimations = { "legacyGenerateAnimations", 0 };
			YamlField<bool> bakeSimulation = { "bakeSimulation", false };
			YamlField<bool> resampleCurves = { "resampleCurves", false };
			YamlField<bool> optimizeGameObjects = { "optimizeGameObjects", false };
			YamlField<std::string> motionNodeName = { "motionNodeName", "" };
			YamlField<std::string> rigImportErrors = { "rigImportErrors", "" };
			YamlField<std::string> rigImportWarnings = { "rigImportWarnings", "" };
			YamlField<std::string> animationImportErrors = { "animationImportErrors", "" };
			YamlField<std::string> animationImportWarnings = { "animationImportWarnings", "" };
			YamlField<std::string> animationRetargetingWarnings = { "animationRetargetingWarnings", "" };
			YamlField<bool> animationDoRetargetingWarnings = { "animationDoRetargetingWarnings", false };
			YamlField<bool> importAnimatedCustomProperties = { "importAnimatedCustomProperties", false };
			YamlField<uint32> animationCompression = { "animationCompression", 0 };
			YamlField<float> animationRotationError = { "animationRotationError", 0.0f };
			YamlField<float> animationPositionError = { "animationPositionError", 0.0f };
			YamlField<float> animationScaleError = { "animationScaleError", 0.0f };
			YamlField<uint32> animationWrapMode = { "animationWrapMode", 0 };
			YamlField<std::vector<std::string>> extraExposedTransformPaths = { "extraExposedTransformPaths", std::vector<std::string>() };
			YamlField<std::vector<std::string>> extraUserProperties = { "extraUserProperties", std::vector<std::string>() };
			YamlField<std::vector<std::string>> clipAnimations = { "clipAnimations", std::vector<std::string>() };
			YamlField<bool> isReadable = { "isReadable", false };
		};

		struct Meshes
		{
		public:
			Meshes();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, Meshes& value)
			{
				value.print("    ");
				return stream;
			}

		//private:
			YamlField<std::vector<float>> lODScreenPercentages = { "lODScreenPercentages", std::vector<float>() };
			YamlField<float> globalScale = { "globalScale", 0.0f };
			YamlField<uint32> meshCompression = { "meshCompression", 0 };
			YamlField<bool> addColliders = { "addColliders", false };
			YamlField<bool> importVisibility = { "importVisibility", false };
			YamlField<bool> importBlendShapes = { "importBlendShapes", false };
			YamlField<bool> importCameras = { "importCameras", false };
			YamlField<bool> importLights = { "importLights", false };
			YamlField<bool> swapUVChannels = { "swapUVChannels", false };
			YamlField<bool> generateSecondaryUV = { "generateSecondaryUV", false };
			YamlField<bool> useFileUnits = { "useFileUnits", false };
			YamlField<bool> optimizeMeshForGPU = { "optimizeMeshForGPU", false };
			YamlField<bool> keepQuads = { "keepQuads", false };
			YamlField<bool> weldVertices = { "weldVertices", false };
			YamlField<bool> preserveHierarchy = { "preserveHierarchy", false };
			YamlField<uint32> indexFormat = { "indexFormat", 0 };
			YamlField<uint32> secondaryUVAngleDistortion = { "secondaryUVAngleDistortion", 0 };
			YamlField<float> secondaryUVAreaDistortion = { "secondaryUVAreaDistortion", 0.0f };
			YamlField<uint32> secondaryUVHardAngle = { "secondaryUVHardAngle", 0 };
			YamlField<uint32> secondaryUVPackMargin = { "secondaryUVPackMargin", 0 };
			YamlField<bool> useFileScale = { "useFileScale", false };
		};

		struct TangentSpace
		{
		public:
			TangentSpace();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, TangentSpace& value)
			{
				value.print("    ");
				return stream;
			}

		private:
			YamlField<float> normalSmoothAngle = { "normalSmoothAngle", 0.0f };
			YamlField<uint32> normalImportMode = { "normalImportMode", 0 };
			YamlField<uint32> tangentImportMode = { "tangentImportMode", 0 };
			YamlField<uint32> normalCalculationMode = { "normalCalculationMode", 0 };
		};

		struct HumanDescription
		{
		public:
			HumanDescription();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);

			friend std::ostream& operator<< (std::ostream& stream, HumanDescription& value)
			{
				value.print("    ");
				return stream;
			}

		private:
			YamlField<uint32> serializedVersion = { "serializedVersion", 0 };
			// TODO: missing human and skeleton array.. need to figure out types
			YamlField<float> armTwist = { "armTwist", 0.0f };
			YamlField<float> foreArmTwist = { "foreArmTwist", 0.0f };
			YamlField<float> upperLegTwist = { "upperLegTwist", 0.0f };
			YamlField<float> legTwist = { "legTwist", 0.0f };
			YamlField<float> armStretch = { "armStretch", 0.0f };
			YamlField<float> legStretch = { "legStretch", 0.0f };
			YamlField<float> feetSpacing = { "feetSpacing", 0.0f };
			YamlField<std::string> rootMotionBoneName = { "rootMotionBoneName", "" };
			YamlField<glm::quat> rootMotionBoneRotation = { "rootMotionBoneRotation", glm::quat(1,0,0,0) };
			YamlField<bool> hasTranslationDoF = { "hasTranslationDoF", false };
			YamlField<bool> hasExtraRoot = { "hasExtraRoot", false };
			YamlField<bool> skeletonHasParents = { "skeletonHasParents", false };
		};

		class ModelImporter : public Importer
		{
		public:
			ModelImporter();
			void fromYAML(ryml::ConstNodeRef n);
			void print(const std::string& prefix);
		//private:
			const std::string name = "ModelImporter";
			YamlField<uint32> serializedVersion = { "serializedVersion", 0 };
			YamlField<std::map<int64, std::string>> fileIDToRecycleName = { "fileIDToRecycleName", std::map<int64, std::string>() };
			YamlField<Materials> materials = { "materials", {} };
			YamlField<Animations> animations = { "animations", {} };
			YamlField<Meshes> meshes = { "meshes", {} };
			YamlField<TangentSpace> tangentSpace = { "tangentSpace", {} };
			YamlField<bool> importAnimation = { "importAnimation", false };
			YamlField<bool> copyAvatar = { "copyAvatar", false };
			YamlField<HumanDescription> humanDescription = { "humanDescription", {} };
			YamlField<InstanceID> lastHumanDescriptionAvatarSource = { "lastHumanDescriptionAvatarSource", 0 };
			YamlField<uint32> animationType = { "animationType", 0 };
			YamlField<uint32> humanoidOversampling = { "humanoidOversampling", 0 };
			YamlField<uint32> additionalBone = { "additionalBone", 0 };
		};

		bool read(ryml::ConstNodeRef n, Materials* value);
		bool read(ryml::ConstNodeRef n, Animations* value);
		bool read(ryml::ConstNodeRef n, Meshes* value);
		bool read(ryml::ConstNodeRef n, TangentSpace* value);
		bool read(ryml::ConstNodeRef n, HumanDescription* value);
	}
}

#endif // INCLUDED_MODELIMPORTER