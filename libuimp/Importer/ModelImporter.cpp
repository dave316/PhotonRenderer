#include "ModelImporter.h"

namespace Unity
{
	namespace YAML
	{
		Materials::Materials()
		{

		}

		void Materials::fromYAML(ryml::ConstNodeRef n)
		{
			if (n.has_child("importMaterials"))
				n[importMaterials.key] >> importMaterials.value;
			else if (n.has_child("materialImportMode"))
				n["materialImportMode"] >> importMaterials.value;
			n[materialName.key] >> materialName.value;
			n[materialSearch.key] >> materialSearch.value;
			if (materialLocation.isPresent = n.has_child(materialLocation.key))
				n[materialLocation.key] >> materialLocation.value;
		}

		void Materials::print(const std::string& prefix)
		{
			std::cout << std::endl;
			importMaterials.print(prefix);
			materialName.print(prefix);
			materialSearch.print(prefix);
			if (materialLocation.isPresent)
				std::cout << prefix << materialLocation.key << ": " << materialLocation.value;
		}

		Animations::Animations()
		{

		}

		void Animations::fromYAML(ryml::ConstNodeRef n)
		{
			n[legacyGenerateAnimations.key] >> legacyGenerateAnimations.value;
			n[bakeSimulation.key] >> bakeSimulation.value;
			if (n.has_child(resampleCurves.key))
				n[resampleCurves.key] >> resampleCurves.value;
			n[optimizeGameObjects.key] >> optimizeGameObjects.value;
			if (!n[motionNodeName.key].val_is_null())
				n[motionNodeName.key] >> motionNodeName.value;
			if (n.has_child(rigImportErrors.key) && !n[rigImportErrors.key].val_is_null())
				n[rigImportErrors.key] >> rigImportErrors.value;
			if (n.has_child(rigImportWarnings.key) && !n[rigImportWarnings.key].val_is_null())
				n[rigImportWarnings.key] >> rigImportWarnings.value;
			if (!n[animationImportErrors.key].val_is_null())
				n[animationImportErrors.key] >> animationImportErrors.value;
			if (!n[animationImportWarnings.key].val_is_null())
				n[animationImportWarnings.key] >> animationImportWarnings.value;
			if (!n[animationRetargetingWarnings.key].val_is_null())
				n[animationRetargetingWarnings.key] >> animationRetargetingWarnings.value;
			n[animationDoRetargetingWarnings.key] >> animationDoRetargetingWarnings.value;
			if (n.has_child(importAnimatedCustomProperties.key))
				n[importAnimatedCustomProperties.key] >> importAnimatedCustomProperties.value;
			n[animationCompression.key] >> animationCompression.value;
			n[animationRotationError.key] >> animationRotationError.value;
			n[animationPositionError.key] >> animationPositionError.value;
			n[animationScaleError.key] >> animationScaleError.value;
			n[animationWrapMode.key] >> animationWrapMode.value;
			n[extraExposedTransformPaths.key] >> extraExposedTransformPaths.value;
			if (n.has_child(extraUserProperties.key))
				n[extraUserProperties.key] >> extraUserProperties.value;
			n[clipAnimations.key] >> clipAnimations.value;
			n[isReadable.key] >> isReadable.value;
		}

		void Animations::print(const std::string& prefix)
		{
			std::cout << std::endl;
			legacyGenerateAnimations.print(prefix);
			bakeSimulation.print(prefix);
			resampleCurves.print(prefix);
			optimizeGameObjects.print(prefix);
			motionNodeName.print(prefix);
			rigImportErrors.print(prefix);
			rigImportWarnings.print(prefix);
			animationImportErrors.print(prefix);
			animationImportWarnings.print(prefix);
			animationRetargetingWarnings.print(prefix);
			animationDoRetargetingWarnings.print(prefix);
			importAnimatedCustomProperties.print(prefix);
			animationCompression.print(prefix);
			animationRotationError.print(prefix);
			animationPositionError.print(prefix);
			animationScaleError.print(prefix);
			animationWrapMode.print(prefix);
			std::cout << prefix << extraExposedTransformPaths.key << ":" << std::endl;
			for (auto& p : extraExposedTransformPaths.value)
				std::cout << prefix << "- " << p << std::endl;
			std::cout << prefix << extraUserProperties.key << ":" << std::endl;
			for (auto& p : extraUserProperties.value)
				std::cout << prefix << "- " << p << std::endl;
			std::cout << prefix << clipAnimations.key << ":" << std::endl;
			for (auto& p : clipAnimations.value)
				std::cout << prefix << "- " << p << std::endl;
			std::cout << prefix << isReadable.key << ": " << isReadable.value;
		}

		Meshes::Meshes()
		{

		}

		void Meshes::fromYAML(ryml::ConstNodeRef n)
		{
			n[lODScreenPercentages.key] >> lODScreenPercentages.value;
			n[globalScale.key] >> globalScale.value;
			n[meshCompression.key] >> meshCompression.value;
			n[addColliders.key] >> addColliders.value;
			if (n.has_child(importVisibility.key))
				n[importVisibility.key] >> importVisibility.value;
			n[importBlendShapes.key] >> importBlendShapes.value;
			if (n.has_child(importCameras.key))
				n[importCameras.key] >> importCameras.value;
			if (n.has_child(importLights.key))
				n[importLights.key] >> importLights.value;
			n[swapUVChannels.key] >> swapUVChannels.value;
			n[generateSecondaryUV.key] >> generateSecondaryUV.value;
			n[useFileUnits.key] >> useFileUnits.value;
			if (n.has_child(optimizeMeshForGPU.key))
				n[optimizeMeshForGPU.key] >> optimizeMeshForGPU.value;
			n[keepQuads.key] >> keepQuads.value;
			n[weldVertices.key] >> weldVertices.value;
			if (n.has_child(preserveHierarchy.key))
				n[preserveHierarchy.key] >> preserveHierarchy.value;
			if (n.has_child(indexFormat.key))
				n[indexFormat.key] >> indexFormat.value;
			n[secondaryUVAngleDistortion.key] >> secondaryUVAngleDistortion.value;
			n[secondaryUVAreaDistortion.key] >> secondaryUVAreaDistortion.value;
			n[secondaryUVHardAngle.key] >> secondaryUVHardAngle.value;
			n[secondaryUVPackMargin.key] >> secondaryUVPackMargin.value;
			n[useFileScale.key] >> useFileScale.value;
		}

		void Meshes::print(const std::string& prefix)
		{
			std::cout << std::endl;
			std::cout << prefix << lODScreenPercentages.key << ":" << std::endl;
			for (auto& p : lODScreenPercentages.value)
				std::cout << prefix << "- " << p << std::endl;
			globalScale.print(prefix);
			meshCompression.print(prefix);
			addColliders.print(prefix);
			importVisibility.print(prefix);
			importBlendShapes.print(prefix);
			importCameras.print(prefix);
			importLights.print(prefix);
			swapUVChannels.print(prefix);
			generateSecondaryUV.print(prefix);
			useFileUnits.print(prefix);
			optimizeMeshForGPU.print(prefix);
			keepQuads.print(prefix);
			weldVertices.print(prefix);
			preserveHierarchy.print(prefix);
			indexFormat.print(prefix);
			secondaryUVAngleDistortion.print(prefix);
			secondaryUVAreaDistortion.print(prefix);
			secondaryUVHardAngle.print(prefix);
			secondaryUVPackMargin.print(prefix);
			std::cout << prefix << useFileScale.key << ": " << useFileScale.value;
		}

		TangentSpace::TangentSpace()
		{

		}

		void TangentSpace::fromYAML(ryml::ConstNodeRef n)
		{
			n[normalSmoothAngle.key] >> normalSmoothAngle.value;
			n[normalImportMode.key] >> normalImportMode.value;
			n[tangentImportMode.key] >> tangentImportMode.value;
			if (n.has_child(normalCalculationMode.key))
				n[normalCalculationMode.key] >> normalCalculationMode.value;
		}

		void TangentSpace::print(const std::string& prefix)
		{
			std::cout << std::endl;
			normalSmoothAngle.print(prefix);
			normalImportMode.print(prefix);
			tangentImportMode.print(prefix);
			std::cout << prefix << normalCalculationMode.key << ": " << normalCalculationMode.value;
		}

		HumanDescription::HumanDescription()
		{

		}

		void HumanDescription::fromYAML(ryml::ConstNodeRef n)
		{
			if (n.has_child(serializedVersion.key))
				n[serializedVersion.key] >> serializedVersion.value;
			n[armTwist.key] >> armTwist.value;
			n[foreArmTwist.key] >> foreArmTwist.value;
			n[upperLegTwist.key] >> upperLegTwist.value;
			n[legTwist.key] >> legTwist.value;
			n[armStretch.key] >> armStretch.value;
			n[legStretch.key] >> legStretch.value;
			n[feetSpacing.key] >> feetSpacing.value;
			if (!n[rootMotionBoneName.key].val_is_null())
				n[rootMotionBoneName.key] >> rootMotionBoneName.value;
			if (n.has_child(rootMotionBoneRotation.key))
				read(n[rootMotionBoneRotation.key], rootMotionBoneRotation.value);
			n[hasTranslationDoF.key] >> hasTranslationDoF.value;
			if (n.has_child(hasExtraRoot.key))
				n[hasExtraRoot.key] >> hasExtraRoot.value;
			if (n.has_child(skeletonHasParents.key))
				n[skeletonHasParents.key] >> skeletonHasParents.value;
		}

		void HumanDescription::print(const std::string& prefix)
		{
			std::cout << std::endl;
			serializedVersion.print(prefix);
			armTwist.print(prefix);
			foreArmTwist.print(prefix);
			upperLegTwist.print(prefix);
			legTwist.print(prefix);
			armStretch.print(prefix);
			legStretch.print(prefix);
			feetSpacing.print(prefix);
			rootMotionBoneName.print(prefix);
			rootMotionBoneRotation.print(prefix);
			hasTranslationDoF.print(prefix);
			hasExtraRoot.print(prefix);
			std::cout << prefix << skeletonHasParents.key << ": " << skeletonHasParents.value;
		}

		ModelImporter::ModelImporter()
		{
		}

		void ModelImporter::fromYAML(ryml::ConstNodeRef n)
		{
			Importer::fromBaseYAML(n);
			n[serializedVersion.key] >> serializedVersion.value;
			if (n.has_child("fileIDToRecycleName"))
			{
				n[fileIDToRecycleName.key] >> fileIDToRecycleName.value;
			}				
			else if (n.has_child("internalIDToNameTable"))
			{
				auto& nameTable = n["internalIDToNameTable"];
				for (auto& nameEntry : nameTable)
				{
					int64_t id;
					std::string name;
					auto& idNode = nameEntry["first"].child(0);
					idNode >> id;
					nameEntry["second"] >> name;
					fileIDToRecycleName.value.insert(std::make_pair(id, name));
				}
			}
			n[materials.key] >> materials.value;
			n[animations.key] >> animations.value;
			n[meshes.key] >> meshes.value;
			n[tangentSpace.key] >> tangentSpace.value;
			n[importAnimation.key] >> importAnimation.value;
			if (n.has_child(copyAvatar.key))
				n[copyAvatar.key] >> copyAvatar.value;
			n[humanDescription.key] >> humanDescription.value;
			n[lastHumanDescriptionAvatarSource.key] >> lastHumanDescriptionAvatarSource.value;
			n[animationType.key] >> animationType.value;
			n[humanoidOversampling.key] >> humanoidOversampling.value;
			n[additionalBone.key] >> additionalBone.value;
		}

		void ModelImporter::print(const std::string& prefix)
		{
			std::cout << name << ":" << std::endl;
			std::string subPrefix = prefix + "  ";
			serializedVersion.print(subPrefix);
			std::cout << subPrefix << fileIDToRecycleName.key << ":" << std::endl;
			for (auto&& [id, name] : fileIDToRecycleName.value)
				std::cout << subPrefix << "  " << id << ": " << name << std::endl;
			materials.print(subPrefix);
			animations.print(subPrefix);
			meshes.print(subPrefix);
			tangentSpace.print(subPrefix);
			importAnimation.print(subPrefix);
			copyAvatar.print(subPrefix);
			humanDescription.print(subPrefix);
			lastHumanDescriptionAvatarSource.print(subPrefix);
			animationType.print(subPrefix);
			humanoidOversampling.print(subPrefix);
			additionalBone.print(subPrefix);
			Importer::print(subPrefix);
		}

		bool read(ryml::ConstNodeRef n, Materials* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, Animations* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, Meshes* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, TangentSpace* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}

		bool read(ryml::ConstNodeRef n, HumanDescription* value)
		{
			if (value)
				value->fromYAML(n);
			return true;
		}
	}
}
