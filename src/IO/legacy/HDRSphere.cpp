#include "HDRSphere.h"

#include <algorithm>
#include <stb_image.h>
#include <stb_image_write.h>


namespace IO
{
	std::vector<float> projectToSH9Scaled(glm::vec3 dir)
	{
		std::vector<float> values(9);
		values[0] = 0.282095f;

		// Band 1
		values[1] = -0.488603f * dir.y;
		values[2] = 0.488603f * dir.z;
		values[3] = -0.488603f * dir.x;

		// Band 2
		values[4] = 1.092548f * dir.x * dir.y;
		values[5] = -1.092548f * dir.y * dir.z;
		values[6] = 0.315392f * (3.0f * dir.z * dir.z - 1.0f);
		values[7] = -1.092548f * dir.x * dir.z;
		values[8] = 0.546274f * (dir.x * dir.x - dir.y * dir.y);

		return values;
	}

	std::vector<float> projectToSH9(glm::vec3 dir)
	{
		std::vector<float> values(9);

		values[0] = 1.0f;

		// Band 1
		values[1] = -dir.y;
		values[2] = dir.z;
		values[3] = -dir.x;

		// Band 2
		values[4] = dir.x * dir.y;
		values[5] = -dir.y * dir.z;
		values[6] = (3.0f * dir.z * dir.z - 1.0f);
		values[7] = -dir.x * dir.z;
		values[8] = (dir.x * dir.x - dir.y * dir.y);

		return values;
	}

	float factorial(uint32_t n, uint32_t d)
	{
		d = glm::max(1u, d);
		n = glm::max(1u, n);
		float r = 1.0f;
		if (n == d)
		{

		}
		else if (n > d)
		{
			for (; n > d; n--)
				r *= n;
		}
		else
		{
			for (; d > n; d--)
				r *= d;
			r = 1.0f / r;
		}
		return r;
	}

	constexpr const double F_2_SQRTPI = 1.12837916709551257389615890312154517;
	constexpr const double F_SQRT2 = 1.41421356237309504880168872420969808;

	float Kml(SSIZE_T m, size_t l) {
		m = m < 0 ? -m : m;  // abs() is not constexpr
		const float K = (2 * l + 1) * factorial(size_t(l - m), size_t(l + m));
		return std::sqrt(K) * (F_2_SQRTPI * 0.25);
	}

	std::vector<float> Ki(size_t numBands) {
		const size_t numCoefs = numBands * numBands;
		std::vector<float> K(numCoefs);
		for (size_t l = 0; l < numBands; l++) {
			K[IO::SHindex(0, l)] = Kml(0, l);
			for (size_t m = 1; m <= l; m++) {
				K[SHindex(m, l)] =
					K[SHindex(-m, l)] = F_SQRT2 * Kml(m, l);
			}
		}
		return K;
	}

	void computeShBasis(
		float* SHb,
		size_t numBands,
		const glm::vec3 s)
	{
#if 0
		// Reference implementation
		float phi = atan2(s.x, s.y);
		for (size_t l = 0; l < numBands; l++) {
			SHb[SHindex(0, l)] = Legendre(l, 0, s.z);
			for (size_t m = 1; m <= l; m++) {
				float p = Legendre(l, m, s.z);
				SHb[SHindex(-m, l)] = std::sin(m * phi) * p;
				SHb[SHindex(m, l)] = std::cos(m * phi) * p;
			}
		}
#endif

		/*
		 * TODO: all the Legendre computation below is identical for all faces, so it
		 * might make sense to pre-compute it once. Also note that there is
		 * a fair amount of symmetry within a face (which we could take advantage of
		 * to reduce the pre-compute table).
		 */

		 /*
		  * Below, we compute the associated Legendre polynomials using recursion.
		  * see: http://mathworld.wolfram.com/AssociatedLegendrePolynomial.html
		  *
		  * Note [0]: s.z == cos(theta) ==> we only need to compute P(s.z)
		  *
		  * Note [1]: We in fact compute P(s.z) / sin(theta)^|m|, by removing
		  * the "sqrt(1 - s.z*s.z)" [i.e.: sin(theta)] factor from the recursion.
		  * This is later corrected in the ( cos(m*phi), sin(m*phi) ) recursion.
		  */

		  // s = (x, y, z) = (sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta))

		  // handle m=0 separately, since it produces only one coefficient
		float Pml_2 = 0;
		float Pml_1 = 1;
		SHb[0] = Pml_1;
		for (size_t l = 1; l < numBands; l++) {
			float Pml = ((2 * l - 1.0f) * Pml_1 * s.z - (l - 1.0f) * Pml_2) / l;
			Pml_2 = Pml_1;
			Pml_1 = Pml;
			SHb[SHindex(0, l)] = Pml;
		}
		float Pmm = 1;
		for (size_t m = 1; m < numBands; m++) {
			Pmm = (1.0f - 2 * m) * Pmm;      // See [1], divide by sqrt(1 - s.z*s.z);
			Pml_2 = Pmm;
			Pml_1 = (2 * m + 1.0f) * Pmm * s.z;
			// l == m
			SHb[SHindex(-m, m)] = Pml_2;
			SHb[SHindex(m, m)] = Pml_2;
			if (m + 1 < numBands) {
				// l == m+1
				SHb[SHindex(-m, m + 1)] = Pml_1;
				SHb[SHindex(m, m + 1)] = Pml_1;
				for (size_t l = m + 2; l < numBands; l++) {
					float Pml = ((2 * l - 1.0f) * Pml_1 * s.z - (l + m - 1.0f) * Pml_2) / (l - m);
					Pml_2 = Pml_1;
					Pml_1 = Pml;
					SHb[SHindex(-m, l)] = Pml;
					SHb[SHindex(m, l)] = Pml;
				}
			}
		}

		// At this point, SHb contains the associated Legendre polynomials divided
		// by sin(theta)^|m|. Below we compute the SH basis.
		//
		// ( cos(m*phi), sin(m*phi) ) recursion:
		// cos(m*phi + phi) == cos(m*phi)*cos(phi) - sin(m*phi)*sin(phi)
		// sin(m*phi + phi) == sin(m*phi)*cos(phi) + cos(m*phi)*sin(phi)
		// cos[m+1] == cos[m]*s.x - sin[m]*s.y
		// sin[m+1] == sin[m]*s.x + cos[m]*s.y
		//
		// Note that (d.x, d.y) == (cos(phi), sin(phi)) * sin(theta), so the
		// code below actually evaluates:
		//      (cos((m*phi), sin(m*phi)) * sin(theta)^|m|
		float Cm = s.x;
		float Sm = s.y;
		for (size_t m = 1; m <= numBands; m++) {
			for (size_t l = m; l < numBands; l++) {
				SHb[SHindex(-m, l)] *= Sm;
				SHb[SHindex(m, l)] *= Cm;
			}
			float Cm1 = Cm * s.x - Sm * s.y;
			float Sm1 = Sm * s.x + Cm * s.y;
			Cm = Cm1;
			Sm = Sm1;
		}
	}

	UniformSamples generateRandomSamples(int numSamples, int seed)
	{
		int numSamplesSquared = numSamples * numSamples;
		int numBands = 3;
		int numFunctions = numBands * numBands;

		UniformSamples samples(numSamplesSquared);
		for (auto& sample : samples)
			sample.values.resize(numFunctions);
		srand(seed);
		int index = 0;
		for (int i = 0; i < numSamples; i++)
		{
			for (int j = 0; j < numSamples; j++)
			{
				float x = (i + ((float)rand() / RAND_MAX)) / numSamples;
				float y = (j + ((float)rand() / RAND_MAX)) / numSamples;
				float theta = 2.0f * glm::acos(glm::sqrt(1.0f - x));
				float phi = glm::two_pi<float>() * y;

				samples[index].dir.x = glm::sin(theta) * glm::cos(phi);
				samples[index].dir.y = glm::sin(theta) * glm::sin(phi);
				samples[index].dir.z = glm::cos(theta);
				samples[index].dir = glm::normalize(samples[index].dir);
				samples[index].values = projectToSH9(samples[index].dir);
				index++;
			}
		}

		return samples;
	}

	HDRSphere::HDRSphere(const std::string& filename)
	{
		int w, h, c;
		stbi_set_flip_vertically_on_load(false);
		data = std::unique_ptr<float>(stbi_loadf(filename.c_str(), &w, &h, &c, 0));
		width = w;
		height = h;
		channels = c;
	}

	void HDRSphere::saveImg(const std::string& filename)
	{
		stbi_write_hdr(filename.c_str(), width, height, channels, data.get());
	}

	void HDRSphere::convertCylindricalToSpherical(int size)
	{
		float* dst = new float[size * size * channels];

		for (int x = 0; x < size; x++)
		{
			for (int y = 0; y < size; y++)
			{
				float u = ((float)x / (float)size - 0.5f) / 0.5f;
				float v = ((float)y / (float)size - 0.5f) / 0.5f;

				if (u * u + v * v > 1.0f)
					continue;

				float pi = glm::pi<float>();
				float phi = glm::atan(v, u);
				float theta = pi * sqrt(u * u + v * v);

				glm::vec3 normal;
				normal.x = glm::sin(theta) * glm::cos(phi);
				normal.y = -glm::sin(theta) * glm::sin(phi);
				normal.z = glm::cos(theta);

				float r = (1.0f / pi) * glm::acos(normal.y) / glm::sqrt(normal.x * normal.x + normal.z * normal.z);

				u = normal.x * r;
				v = normal.z * r;

				theta = glm::atan(v, u);
				phi = pi * glm::sqrt(u * u + v * v);

				int x_src = int(((theta + pi) / (2.0f * pi)) * width);
				int y_src = int((phi / pi) * height);

				int idx = 3 * (size * y + x);
				int idx_src = 3 * (width * y_src + x_src);

				dst[idx] = data.get()[idx_src];
				dst[idx + 1] = data.get()[idx_src + 1];
				dst[idx + 2] = data.get()[idx_src + 2];
			}
		}

		data.reset(dst);
		width = size;
		height = size;
	}

	glm::vec3 HDRSphere::getColor(glm::vec3 normal)
	{
		glm::vec3 D = normal;
		D.y = -D.y;

		float r = (1.0f / glm::pi<float>()) * glm::acos(D.z) / glm::sqrt(D.x * D.x + D.y * D.y);
		float u = D.x * r;
		float v = D.y * r;

		int x = int((u * 0.5f + 0.5f) * width);
		int y = int((v * 0.5f + 0.5f) * height);
		int idx = glm::clamp(0.0f, (float)width * height - 1, (float)width * y + x);

		glm::vec3 color;
		color.x = data.get()[idx * 3];
		color.y = data.get()[idx * 3 + 1];
		color.z = data.get()[idx * 3 + 2];

		return color;
	}

	SphericalLightingProbe HDRSphere::createEnvironmentProbe(std::vector<SphericalSample> spSamples)
	{
		int numBands = 3;
		int numFunctions = numBands * numBands;
		int numSamples = spSamples.size();
		SphericalLightingProbe probe(numFunctions);
		for (int k = 0; k < numSamples; k++)
		{
			glm::vec3 color = getColor(spSamples[k].dir);
			for (int l = 0; l < numFunctions; l++)
			{
				probe.coeffs[l] += color * spSamples[k].values[l];
			}
		}

		float factor = 4.0f * glm::pi<float>() / numSamples;
		for (int k = 0; k < numFunctions; k++)
		{
			probe.coeffs[k] *= factor;
		}

		return probe;
	}
}
