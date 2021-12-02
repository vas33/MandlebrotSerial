#pragma once
#include <array>
#include <random>

namespace mandlebrot
{
	/*const unsigned int imageWidth = 600;
	const unsigned int imageHeight = 400;*/

	const unsigned int imageWidth = 6000;
	const unsigned int imageHeight = 4000;

	const unsigned int maxIterations = 80;

	const float realPartStart = -2;
	const float realPartEnd = 1;
	const float imPartStart = -1;
	const float imPartEnd = 1;

	struct alignas(16) MandlebrotColor
	{
		unsigned int R{ 0 };
		unsigned int G{ 0 };
		unsigned int B{ 0 };
		unsigned int A{ 255 };
	};

	inline unsigned int mandlebrot(std::pair<float, float> numer)
	{
		//z(n + 1) = z(n)^2 + c.
		//so
		//z(0) = 0;
		//z(1) = c;
		//z(2) = c^2 + c;

		//c^2 = (a + bi)^2;
		//c^2 = a^2 + 2* a*bi + b^2*i^2 //aand i^2 == -1
		//c^2 = a^2 + 2*a*bi - b^2

		double real = numer.first;
		double imaginary = numer.second;

		
		unsigned int numIterations = 0;
		while (real * real + imaginary * imaginary <= 4.0 && numIterations < maxIterations)
		{
			double  newReal = real * real - imaginary * imaginary;
			double  newImaginary = 2.f * real * imaginary;

			real = numer.first + newReal;
			imaginary = numer.second + newImaginary;

			++numIterations;
		}

		return numIterations;
	};

	alignas(16) std::array<MandlebrotColor, maxIterations + 1> mandlebrodColors;

	void GenerateMandlebrodColors()
	{
		std::random_device rd;  //Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_int_distribution<> distrib(0, 255);

		for (int colorIndex = 0; colorIndex < maxIterations; ++colorIndex)
		{
			unsigned int R = distrib(gen);
			unsigned int G = distrib(gen);
			unsigned int B = distrib(gen);

			mandlebrodColors[colorIndex] = { R,G,B };
		}
		mandlebrodColors[maxIterations] = { 0,0,0 };
	}

	inline MandlebrotColor getColor(unsigned int iterations)
	{
		return mandlebrodColors[iterations];
	};
}