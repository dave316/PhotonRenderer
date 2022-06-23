#ifndef INCLUDED_CUBEMAP_LEGACY
#define INCLUDED_CUBEMAP_LEGACY

#pragma once

#include "Image.h"
#include "HDRSphere.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>

template<typename DataType, uint32_t Channels>
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

	typedef Cubemap<DataType, Channels> CubemapType;
	typedef Image<DataType, Channels> ImageType;

private:
	std::shared_ptr<ImageType> faces[NUM_FACES];
	uint32_t size;
public:
	Cubemap(uint32_t size) :
		size(size)
	{
		for(int i = 0; i < NUM_FACES; i++)
			faces[i] = ImageType::create(size, size);
	}

	glm::vec3 getCubeDirection(Face f, float x, float y)
	{
		float scale = 2.0f / size;
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
		float x = glm::atan(dir.x, dir.z) * one_pi;
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
			for (uint32_t y = 0; y < size; y++)
			{
				for (uint32_t x = 0; x < size; x++)
				{
					glm::vec2 pos0 = direction2rectilinear(getCubeDirection(Face(f), x + 0.0f, y + 0.0f), image->getSize());
					glm::vec2 pos1 = direction2rectilinear(getCubeDirection(Face(f), x + 1.0f, y + 0.0f), image->getSize());
					glm::vec2 pos2 = direction2rectilinear(getCubeDirection(Face(f), x + 0.0f, y + 1.0f), image->getSize());
					glm::vec2 pos3 = direction2rectilinear(getCubeDirection(Face(f), x + 1.0f, y + 1.0f), image->getSize());

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
						glm::vec2 uv = direction2rectilinear(dir, image->getSize());
						pixel += image->getPixel(uv.x, uv.y);
					}
					pixel *= iN;
					faces[f]->setPixel(pixel, x, y);
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
		float upperBound = std::nextafter((float)size, 0.0f);
		// TODO: bilinear interpolation
		for (int f = 0; f < (int)Face::NUM_FACES; f++)
		{
			for (uint32_t y = 0; y < size; y++)
			{
				for (uint32_t x = 0; x < size; x++)
				{
					glm::vec3 dir = getCubeDirection(Face(f), x + 0.5f, y + 0.5f);
					dir.x = -dir.x;
					Address addr = getAddressFor(dir);
					const uint32_t cx = std::min(uint32_t(addr.s * size), size - 1);
					const uint32_t cy = std::min(uint32_t(addr.t * size), size - 1);
					auto faceImg = cm->getFaceImage(Face(addr.face));
					auto pixel = faceImg->getPixel(cx, cy);
					faces[f]->setPixel(pixel, x, y);
				}
			}
		}
	}

	float sphereQuadrantArea(float x, float y)
	{
		return glm::atan(x * y, glm::length(glm::vec3(x, y, 1)));
	}

	float solidAngle(uint32_t size, uint32_t x, uint32_t y)
	{
		float invSize = 1.0f / size;
		float s = ((x + 0.5f) * 2.0f * invSize) - 1.0f;
		float t = ((y + 0.5f) * 2.0f * invSize) - 1.0f;
		float x0 = s - invSize;
		float y0 = t - invSize;
		float x1 = s + invSize;
		float y1 = t + invSize;
		float area00 = sphereQuadrantArea(x0, y0);
		float area01 = sphereQuadrantArea(x0, y1);
		float area10 = sphereQuadrantArea(x1, y0);
		float area11 = sphereQuadrantArea(x1, y1);
		return area00 - area01 - area10 + area11;
	}

	//float truncatedCosSH9(uint32_t l)
	//{
	//	if (l == 0)
	//		return glm::pi<float>();
	//	else if (l == 1)
	//		return glm::two_pi<float>() / 3.0f;

	//	float A0 = -1.0f / 4.0f;
	//	float A1 = IO::factorial(l) / (1.0f * (1 << l));
	//	return 2.0f * glm::pi<float>() * A0 * A1;
	//}

	constexpr float computeTruncatedCosSh(size_t l) {
		if (l == 0) {
			return glm::pi<float>();
		}
		else if (l == 1) {
			return 2 * glm::pi<float>() / 3;
		}
		else if (l & 1u) {
			return 0;
		}
		const size_t l_2 = l / 2;
		float A0 = ((l_2 & 1u) ? 1.0f : -1.0f) / ((l + 2) * (l - 1));
		float A1 = IO::factorial(l, l_2) / (IO::factorial(l_2) * (1 << l));
		return 2 * glm::pi<float>() * A0 * A1;
	}


	IO::SphericalLightingProbe computeSH9()
	{
		IO::SphericalLightingProbe probe(9);
		for (int f = 0; f < (int)Face::NUM_FACES; f++)
		{
			for (uint32_t y = 0; y < size; y++)
			{
				for (uint32_t x = 0; x < size; x++)
				{
					glm::vec3 dir = getCubeDirection(Face(f), x + 0.5f, y + 0.5f);
					glm::vec3 color = faces[f]->getPixel(x, y);
					color *= solidAngle(size, x, y);
					//auto sh9 = IO::projectToSH9(dir);
					float sh9[9];
					IO::computeShBasis(sh9, 3, dir);
					for (int i = 0; i < 9; i++)
						probe.coeffs[i] += color * sh9[i];
				}
			}
		}

		auto K = IO::Ki(3);
		for (auto k : K)
			std::cout << k << std::endl;
		for (auto coeffs : probe.coeffs)
			std::cout << coeffs.x << " " << coeffs.y << " " << coeffs.z << std::endl;
		std::cout << "-------------" << std::endl;
		constexpr const float one_pi = glm::one_over_pi<float>();

		//for (size_t l = 0; l < numBands; l++) {
		//	const float truncatedCosSh = computeTruncatedCosSh(size_t(l));
		//	K[SHindex(0, l)] *= truncatedCosSh;
		//	for (size_t m = 1; m <= l; m++) {
		//		K[SHindex(-m, l)] *= truncatedCosSh;
		//		K[SHindex(m, l)] *= truncatedCosSh;
		//	}
		//}

		for (size_t l = 0; l < 3; l++) {
			const float truncatedCosSh = computeTruncatedCosSh(size_t(l));
			K[IO::SHindex(0, l)] *= truncatedCosSh;
			for (size_t m = 1; m <= l; m++) {
				K[IO::SHindex(-m, l)] *= truncatedCosSh;
				K[IO::SHindex(m, l)] *= truncatedCosSh;
			}
		}

		for (int i = 0; i < 9; i++)
			probe.coeffs[i] *= K[i];

		for (auto coeffs : probe.coeffs)
			std::cout << coeffs.x << " " << coeffs.y << " " << coeffs.z << std::endl;
		std::cout << "-------------" << std::endl;
		//probe.coeffs[0] *= 0.282095f * (ks[0] * computeTruncatedCosSh(0) * one_pi);
		//probe.coeffs[1] *= -0.488603f * (ks[1] * computeTruncatedCosSh(1) * one_pi);
		//probe.coeffs[2] *= 0.488603f * (ks[2] * computeTruncatedCosSh(1) * one_pi);
		//probe.coeffs[3] *= -0.488603f * (ks[3] * computeTruncatedCosSh(1) * one_pi);
		//probe.coeffs[4] *= 1.092548f * (ks[4] * computeTruncatedCosSh(2) * one_pi);
		//probe.coeffs[5] *= -1.092548f * (ks[5] * computeTruncatedCosSh(2) * one_pi);
		//probe.coeffs[6] *= 0.315392f * (ks[6] * computeTruncatedCosSh(2) * one_pi);
		//probe.coeffs[7] *= -1.092548f * (ks[7] * computeTruncatedCosSh(2) * one_pi);
		//probe.coeffs[8] *= 0.546274f * (ks[8] * computeTruncatedCosSh(2) * one_pi);

		constexpr float M_SQRT_PI = 1.7724538509f;
		constexpr float M_SQRT_3 = 1.7320508076f;
		constexpr float M_SQRT_5 = 2.2360679775f;
		constexpr float M_SQRT_15 = 3.8729833462f;
		constexpr float A[9] = {
					  1.0f / (2.0f * M_SQRT_PI),    // 0  0
				-M_SQRT_3 / (2.0f * M_SQRT_PI),    // 1 -1
				 M_SQRT_3 / (2.0f * M_SQRT_PI),    // 1  0
				-M_SQRT_3 / (2.0f * M_SQRT_PI),    // 1  1
				 M_SQRT_15 / (2.0f * M_SQRT_PI),    // 2 -2
				-M_SQRT_15 / (2.0f * M_SQRT_PI),    // 3 -1
				 M_SQRT_5 / (4.0f * M_SQRT_PI),    // 3  0
				-M_SQRT_15 / (2.0f * M_SQRT_PI),    // 3  1
				 M_SQRT_15 / (4.0f * M_SQRT_PI)     // 3  2
		};

		for (int i = 0; i < 9; i++)
		{
			std::cout << A[i] << std::endl;
			probe.coeffs[i] *= A[i] * one_pi;
		}
			

		//probe.coeffs[0] *= 0.282095f * one_pi;
		//probe.coeffs[1] *= -0.488603f * one_pi;
		//probe.coeffs[2] *= 0.488603f * one_pi;
		//probe.coeffs[3] *= -0.488603f * one_pi;
		//probe.coeffs[4] *= 1.092548f * one_pi;
		//probe.coeffs[5] *= -1.092548f * one_pi;
		//probe.coeffs[6] *= 0.315392f * one_pi;
		//probe.coeffs[7] *= -1.092548f * one_pi;
		//probe.coeffs[8] *= 0.546274f * one_pi;

		for (auto coeffs : probe.coeffs)
			std::cout << coeffs.x << " " << coeffs.y << " " << coeffs.z << std::endl;

		return probe;
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

		auto cm = TextureCubeMap::create(size, size, GL::RGB32F);
		for (int f = POS_X; f < NUM_FACES; f++)
			cm->uploadFace(GL::CupeMapFace(f), faces[f]->getRawPtr());
		return cm;
	}

	typedef std::shared_ptr<CubemapType> Ptr;
	static Ptr create(uint32_t size)
	{
		return Ptr(new CubemapType(size));
	}
};

typedef Cubemap<float, 3> CubemapRGB32F;

#endif // INCLUDED_CUBEMAP_LEGACY