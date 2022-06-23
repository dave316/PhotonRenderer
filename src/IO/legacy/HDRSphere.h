#ifndef INCLUDED_HDRSPHERE_LEGACY
#define INCLUDED_HDRSPHERE_LEGACY

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <basetsd.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace IO
{
	struct SphericalSample
	{
		glm::vec3 dir;
		std::vector<float> values;
	};
	typedef std::vector<SphericalSample> UniformSamples;
	UniformSamples generateRandomSamples(int numSamples, int seed);
	std::vector<float> projectToSH9(glm::vec3 dir);
	std::vector<float> projectToSH9Scaled(glm::vec3 dir);
	float factorial(uint32_t n, uint32_t d = 1);
	float Kml(SSIZE_T m, size_t l);
	std::vector<float> Ki(size_t numBands);
	void computeShBasis(
		float* SHb,
		size_t numBands,
		const glm::vec3 s);
	class SphericalLightingProbe
	{
	public:
		std::vector<glm::vec3> coeffs;
		SphericalLightingProbe() {}
		SphericalLightingProbe(const SphericalLightingProbe& rhd)
		{
			coeffs = rhd.coeffs;
		}
		SphericalLightingProbe(int numCoeffs, float value = 0.0f)
		{
			coeffs.resize(numCoeffs, glm::vec3(value));
		}
		SphericalLightingProbe operator*(const glm::vec3& rhd)
		{
			SphericalLightingProbe probe(coeffs.size());
			for (int i = 0; i < coeffs.size(); i++)
			{
				probe.coeffs[i] = coeffs[i] * rhd;
			}
			return probe;
		}
		SphericalLightingProbe operator*(const float& value)
		{
			SphericalLightingProbe probe(coeffs.size());
			for (int i = 0; i < coeffs.size(); i++)
			{
				probe.coeffs[i] = coeffs[i] * value;
			}
			return probe;
		}
		SphericalLightingProbe operator+(const SphericalLightingProbe& rhd)
		{
			SphericalLightingProbe probe(coeffs.size());
			for (int i = 0; i < coeffs.size(); i++)
			{
				probe.coeffs[i] = coeffs[i] + rhd.coeffs[i];
			}
			return probe;
		}
		glm::vec3 dot(const SphericalLightingProbe& rhd) const
		{
			glm::vec3 dotProd = glm::vec3(0.0f);
			for (int i = 0; i < coeffs.size(); i++)
			{
				dotProd.x += coeffs[i].x * rhd.coeffs[i].x;
				dotProd.y += coeffs[i].y * rhd.coeffs[i].y;
				dotProd.z += coeffs[i].z * rhd.coeffs[i].z;
			}
			return dotProd;
		}
	};
	static inline constexpr size_t SHindex(SSIZE_T m, size_t l) {
		return l * (l + 1) + m;
	}

	struct SHBasisFunction
	{
		float K(int l, int m)
		{
			const float kConstantFactors[4][4] =
			{
				{ 0.282095f, 0, 0, 0 },
				{ 0.488603f, 0.345494f, 0, 0 },
				{ 0.630783f, 0.257516f, 0.128758f, 0 },
				{ 0.746353f, 0.215453f, 0.0681324f, 0.0278149f }
			};

			return kConstantFactors[l][m];
		}

		float P(int l, int m, float x)
		{
			float pmm = 1.0f;
			if (m > 0)
			{
				float somx2 = glm::sqrt((1 - x) * (1 + x));
				float fact = 1;
				for (int i = 1; i <= m; i++)
				{
					pmm *= -fact * somx2;
					fact += 2;
				}
			}

			if (l == m)
				return pmm;

			float pmmp1 = x * (2.0f * m + 1.0f) * pmm;
			if (l == m + 1)
				return pmmp1;

			float pll = 0.0f;
			for (int ll = m + 2; ll <= l; ll++)
			{
				pll = ((2 * ll - 1) * x * pmmp1 - (ll + m - 1) * pmm) / (ll - m);
				pmm = pmmp1;
				pmmp1 = pll;
			}

			return pll;
		}

		float Y(int l, int m, float theta, float phi)
		{
			const float sqrt2 = glm::root_two<float>();
			if (m == 0)
				return K(l, 0) * P(l, m, glm::cos(theta));
			else if (m > 0)
				return sqrt2 * K(l, m) * glm::cos(m * phi) * P(l, m, glm::cos(theta));
			else
				return sqrt2 * K(l, -m) * glm::sin(-m * phi) * P(l, -m, glm::cos(theta));
		}
	};

	class HDRSphere
	{
	private:
		std::unique_ptr<float> data;
		unsigned int width;
		unsigned int height;
		unsigned int channels;

	public:
		HDRSphere(const std::string& filename);
		void saveImg(const std::string& filename);
		void convertCylindricalToSpherical(int size);
		glm::vec3 getColor(glm::vec3 normal);
		SphericalLightingProbe createEnvironmentProbe(std::vector<SphericalSample> spSamples);
	};
}

#endif // INCLUDED_HDRSPHERE_LEGACY