#ifndef INCLUDED_CUBEMAP
#define INCLUDED_CUBEMAP

#pragma once

#include "Image.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace IO
{
	template<typename DataType>
	class Cubemap
	{
	public:
		enum Face
		{
			POS_X = 0,
			NEG_X,
			POS_Y,
			NEG_Y,
			POS_Z,
			NEG_Z,
			NUM_FACES
		};

		typedef Image<DataType> ImageType;
		typedef Cubemap<DataType> CubemapType;

	private:

		uint32 faceSize;
		uint32 channels;
		std::shared_ptr<ImageType> faces[NUM_FACES];

	public:
		Cubemap(uint32 faceSize, uint32 channels) :
			faceSize(faceSize),
			channels(channels)
		{
			for (int i = 0; i < NUM_FACES; i++)
				faces[i] = ImageType::create(faceSize, faceSize, channels);
		}

		glm::vec3 getCubeDirection(Face f, float x, float y)
		{
			float scale = 2.0f / faceSize;
			float cx = (x * scale) - 1.0f;
			float cy = 1.0f - (y * scale);
			//glm::vec2 c(x * scale - 1.0f, 1.0f - y * scale);
			glm::vec3 dir;
			const float l = std::sqrt(cx * cx + cy * cy + 1);
			switch (f)
			{
			case Face::POS_X: dir = glm::vec3(1, cy, -cx); break;
			case Face::NEG_X: dir = glm::vec3(-1, cy, cx); break;
			case Face::POS_Y: dir = glm::vec3(cx, 1, -cy); break;
			case Face::NEG_Y: dir = glm::vec3(cx, -1, cy); break;
			case Face::POS_Z: dir = glm::vec3(cx, cy, 1); break;
			case Face::NEG_Z: dir = glm::vec3(-cx, cy, -1); break;
			}
			return dir * (1.0f / l);
		}

		glm::vec2 direction2rectilinear(glm::vec3 dir, glm::vec2 size)
		{
			constexpr const float one_pi = glm::one_over_pi<float>();
			float x = glm::atan(-dir.x, dir.z) * one_pi;
			float y = glm::asin(dir.y) * (2.0f * one_pi);
			x = (x + 1.0f) * 0.5f * (size.x - 1.0f);
			y = (1.0f - y) * 0.5f * (size.y - 1.0f);
			return glm::vec2(x, y);
		}

		glm::vec2 hammersley(unsigned int i, float iN)
		{
			constexpr float tof = 0.5f / 0x80000000U;
			uint32_t bits = i;
			bits = (bits << 16u) | (bits >> 16u);
			bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
			bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
			bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
			bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
			return glm::vec2(i * iN, bits * tof);
		}

		void setFromEquirectangularMap(std::shared_ptr<ImageType> image)
		{
			for (int f = 0; f < (int)Face::NUM_FACES; f++)
			{
				for (uint32_t y = 0; y < faceSize; y++)
				{
					for (uint32_t x = 0; x < faceSize; x++)
					{
						glm::vec2 size = glm::vec2(image->getWidth(), image->getHeight());
						glm::vec2 pos0 = direction2rectilinear(getCubeDirection(Face(f), x + 0.0f, y + 0.0f), size);
						glm::vec2 pos1 = direction2rectilinear(getCubeDirection(Face(f), x + 1.0f, y + 0.0f), size);
						glm::vec2 pos2 = direction2rectilinear(getCubeDirection(Face(f), x + 0.0f, y + 1.0f), size);
						glm::vec2 pos3 = direction2rectilinear(getCubeDirection(Face(f), x + 1.0f, y + 1.0f), size);

						glm::vec2 minPos = glm::min(glm::min(pos0, pos1), glm::min(pos2, pos3));
						glm::vec2 maxPos = glm::max(glm::max(pos0, pos1), glm::max(pos2, pos3));
						float dx = glm::max(1.0f, maxPos.x - minPos.x);
						float dy = glm::max(1.0f, maxPos.y - minPos.y);
						uint32_t numSamples = dx * dy;

						float iN = 1.0f / numSamples;
						glm::vec3 pixel = glm::vec3(0);
						for (uint32_t sample = 0; sample < numSamples; sample++)
						{
							glm::vec2 h = hammersley(sample, iN);
							glm::vec3 dir = getCubeDirection(Face(f), x + h.x, y + h.y);
							//glm::vec2 size = glm::vec2(image->getWidth(), image->getHeight());
							glm::vec2 uv = direction2rectilinear(dir, size);
							auto pixelData = image->getPixel(uv.x, uv.y);
							glm::vec3 color;
							for (int i = 0; i < pixelData.size(); i++)
								color[i] = pixelData[i];
							pixel += color;
						}
						pixel *= iN;
						std::vector<DataType> values(channels);
						for (int i = 0; i < values.size(); i++)
							values[i] = pixel[i];
						faces[f]->setPixel(values, x, y);
					}
				}
			}
		}

		void setFromFaces(std::shared_ptr<ImageType> image)
		{
			for (int f = 0; f < (int)Face::NUM_FACES; f++)
			{
				for (uint32_t y = 0; y < faceSize; y++)
				{
					for (uint32_t x = 0; x < faceSize; x++)
					{
						auto rgba = image->getPixel(f * faceSize + x, y);
						std::vector<float> color;
						for (int i = 0; i < 3; i++)
							color.push_back(rgba[i]);
						faces[f]->setPixel(color, x, y);
					}
				}
			}
		}

		struct Address {
			Face face;
			float s = 0;
			float t = 0;
		};

		Address getAddressFor(glm::vec3 r) {
			Address addr;
			float sc, tc, ma;
			const float rx = std::abs(r.x);
			const float ry = std::abs(r.y);
			const float rz = std::abs(r.z);
			if (rx >= ry && rx >= rz) {
				ma = 1.0f / rx;
				if (r.x >= 0) {
					addr.face = Face::POS_X;
					sc = -r.z;
					tc = -r.y;
				}
				else {
					addr.face = Face::NEG_X;
					sc = r.z;
					tc = -r.y;
				}
			}
			else if (ry >= rx && ry >= rz) {
				ma = 1.0f / ry;
				if (r.y >= 0) {
					addr.face = Face::POS_Y;
					sc = r.x;
					tc = r.z;
				}
				else {
					addr.face = Face::NEG_Y;
					sc = r.x;
					tc = -r.z;
				}
			}
			else {
				ma = 1.0f / rz;
				if (r.z >= 0) {
					addr.face = Face::POS_Z;
					sc = r.x;
					tc = -r.y;
				}
				else {
					addr.face = Face::NEG_Z;
					sc = -r.x;
					tc = -r.y;
				}
			}
			// ma is guaranteed to be >= sc and tc
			addr.s = (sc * ma + 1.0f) * 0.5f;
			addr.t = (tc * ma + 1.0f) * 0.5f;
			return addr;
		}

		void mirror(std::shared_ptr<CubemapType> cm)
		{
			float upperBound = std::nextafter((float)faceSize, 0.0f);
			// TODO: bilinear interpolation
			for (int f = 0; f < (int)Face::NUM_FACES; f++)
			{
				for (uint32_t y = 0; y < faceSize; y++)
				{
					for (uint32_t x = 0; x < faceSize; x++)
					{
						glm::vec3 dir = getCubeDirection(Face(f), x + 0.5f, y + 0.5f);
						dir.x = -dir.x;
						Address addr = getAddressFor(dir);
						const uint32_t cx = std::min(uint32_t(addr.s * faceSize), faceSize - 1);
						const uint32_t cy = std::min(uint32_t(addr.t * faceSize), faceSize - 1);
						auto faceImg = cm->getFaceImage(Face(addr.face));
						auto pixel = faceImg->getPixel(cx, cy);
						faces[f]->setPixel(pixel, x, y);
					}
				}
			}
		}

		std::shared_ptr<ImageType> getFaceImage(Face f)
		{
			return faces[f];
		}

		TextureCubeMap::Ptr upload()
		{
			//GL::TextureFormat format = GL::RGB8;
			//switch (Channels)
			//{
			//case 1: format = GL::R8; break;
			//case 2: format = GL::RG8; break;
			//case 3: format = GL::RGB8; break;
			//case 4: format = GL::RGBA8; break;
			//default:
			//	std::cout << "error: no format for " << Channels << " channels" << std::endl;
			//	return nullptr;
			//	break;
			//}

			auto cm = TextureCubeMap::create(faceSize, faceSize, GL::RGB16F);
			for (int f = POS_X; f < NUM_FACES; f++)
				cm->uploadFace(GL::CupeMapFace(f), faces[f]->getRawPtr());
			return cm;
		}

		typedef std::shared_ptr<CubemapType> Ptr;
		static Ptr create(uint32 faceSize, uint32 channels)
		{
			return std::make_shared<CubemapType>(faceSize, channels);
		}
	};

	typedef Cubemap<uint8> CubemapUI8;
	typedef Cubemap<uint16> CubemapUI16;
	typedef Cubemap<uint32> CubemapUI32;
	typedef Cubemap<float> CubemapF32;
	//typedef Cubemap<half> CubemapF16;
}

#endif // INCLUDED_CUBEMAP