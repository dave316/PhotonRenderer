#include "EnvironmentMap.h"
#include "MeshPrimitives.h"
#include <IO/Image/ImageDecoder.h>
#include <IO/Image/Cubemap.h>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>


EnvironmentMap::EnvironmentMap(unsigned int faceSize) :
	faceSize(faceSize)
{
	cubeMap = TextureCubeMap::create(faceSize, faceSize, GL::RGB32F);
	cubeMesh = MeshPrimitives::createCube(glm::vec3(0), 1.0f);

	setViews(glm::vec3(0));
}

void EnvironmentMap::setViews(glm::vec3 position)
{
	views.clear();

	glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f); // TODO: allow to set zNear/zFar
	views.push_back(P * glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	views.push_back(P * glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	views.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	views.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	views.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	views.push_back(P * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

	cubeMesh = MeshPrimitives::createCube(position, 1.0f);
}

void EnvironmentMap::fromPanorama(Texture2D::Ptr panorama, Shader::Ptr pano2cm)
{
	pano2cm->use();
	pano2cm->setUniform("M", glm::mat4(1.0f));
	pano2cm->setUniform("VP[0]", views);
	pano2cm->setUniform("panorama", 0);
	
	cubeMap = TextureCubeMap::create(faceSize, faceSize, GL::RGB32F); // TODO: set format depending on use
	cubeMap->generateMipmaps();

	auto envFBO = Framebuffer::create(faceSize, faceSize);
	envFBO->addRenderTexture(GL::COLOR0, cubeMap);
	envFBO->checkStatus();
	envFBO->begin();
	panorama->use(0);
	cubeMesh->draw();
	envFBO->end();

	cubeMap->generateMipmaps();
	cubeMap->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
}

void EnvironmentMap::fromFile(const std::string& filename)
{
	//cubeMap = IO::loadCubemapKTX(filename);

	auto panoImgHDR = IO::decodeHDRFromFile(filename);
	auto cm = IO::CubemapF32::create(faceSize, panoImgHDR->getChannels());
	cm->setFromEquirectangularMap(panoImgHDR);
	cubeMap = cm->upload();
	cubeMap->generateMipmaps();
	cubeMap->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
	
	//std::ifstream is(filename, std::ios::binary);
	//LinearImage linputImage = ImageDecoder::decode(is, filename);
	//uint32_t w = linputImage.getWidth();
	//uint32_t h = linputImage.getHeight();
	//uint32_t c = linputImage.getChannels();
	//auto panoImgHDR = ImageRGB32F::create(w, h);
	//panoImgHDR->setFromMemory(linputImage.getPixelRef(), w * h * c);
	//{
	//	float* data = linputImage.getPixelRef();
	//	uint32_t w = linputImage.getWidth();
	//	uint32_t h = linputImage.getHeight();
	//	uint32_t c = linputImage.getChannels();
	//	uint32_t size = w * h * c;
	//	glm::vec3 avgColor(0);
	//	for (int i = 0; i < size; i += 3)
	//	{
	//		avgColor.r += data[i];
	//		avgColor.g += data[i + 1];
	//		avgColor.b += data[i + 2];
	//	}
	//	avgColor /= (float)w * h;
	//	std::cout << "avg: " << avgColor.r << " " << avgColor.g << " " << avgColor.b << std::endl;
	//}
	// 
	// 
	//auto panoImgHDR = IO_Legacy::loadImageFromFile(filename);
	//auto cm = CubemapRGB32F::create(faceSize);
	//auto cm2 = CubemapRGB32F::create(faceSize);
	//cm->setFromEquirectangularMap(panoImgHDR);
	//cm2->mirror(cm);
	//cubeMap = cm2->upload();
	//cubeMap->generateMipmaps();
	//cubeMap->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
}

void EnvironmentMap::fromTex(TextureCubeMap::Ptr tex)
{
	//cubeMap->generateMipmaps();
	cubeMap = tex->copy();
	cubeMap->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
}

void EnvironmentMap::filter(EnvironmentMap::Ptr source, Shader::Ptr filter)
{
	filter->use();
	filter->setUniform("M", glm::mat4(1.0f));
	filter->setUniform("VP[0]", views);
	filter->setUniform("environmentMap", 0);

	auto envFBO = Framebuffer::create(faceSize, faceSize);
	envFBO->addRenderTexture(GL::COLOR0, cubeMap);
	envFBO->checkStatus();
	envFBO->begin();
	source->use(0);
	cubeMesh->draw();
	envFBO->end();
}

void EnvironmentMap::filterMips(EnvironmentMap::Ptr source, Shader::Ptr filter, unsigned int maxMipLevel)
{
	filter->use();
	filter->setUniform("M", glm::mat4(1.0f));
	filter->setUniform("VP[0]", views);
	filter->setUniform("environmentMap", 0);

	cubeMap->generateMipmaps();
	cubeMap->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);

	auto specFBO = Framebuffer::create(faceSize, faceSize);
	for (unsigned int mip = 0; mip < maxMipLevel; mip++)
	{
		unsigned int mipWidth = faceSize * std::pow(0.5, mip);
		unsigned int mipHeight = faceSize * std::pow(0.5, mip);
		float roughness = (float)mip / (float)(maxMipLevel - 1);
		filter->setUniform("roughness", roughness);

		specFBO->resize(mipWidth, mipHeight);
		specFBO->addRenderTexture(GL::COLOR0, cubeMap, mip);
		specFBO->begin();
		source->use(0);
		cubeMesh->draw();
		specFBO->end();
	}
}

void EnvironmentMap::use(GLuint unit)
{
	cubeMap->use(unit);
}

TextureCubeMap::Ptr EnvironmentMap::getCubeMap()
{
	return cubeMap;
}