// MandlebrotSerial.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <assert.h>
#include <iostream>
#include <map>
#include <random>

#include "mandlebrot.h"
#include "utill.h"
#include "ch1.h"

#include <dvec.h>
#include <pmmintrin.h>

void MandlebrotSerial()
{
	mandlebrot::GenerateMandlebrodColors();

	MeasureExecution ex("MandlebrotSerial");

	std::cout << "Generating mandlebrot image(serial)..\n";

	auto image = std::make_shared<ch01::Image>("mandlebrot",
		mandlebrot::imageWidth, mandlebrot::imageHeight);
	{
		for (unsigned int xCoord = 0; xCoord < mandlebrot::imageWidth; ++xCoord)
		{
			for (unsigned int yCoord = 0; yCoord < mandlebrot::imageHeight; ++yCoord)
			{
				std::pair<float, float> cNumber{ 0.f, 0.f };
				cNumber.first = mandlebrot::realPartStart + ((float)xCoord / (float)mandlebrot::imageWidth) * (mandlebrot::realPartEnd - mandlebrot::realPartStart);
				cNumber.second = mandlebrot::imPartStart + ((float)yCoord / (float)mandlebrot::imageHeight) * (mandlebrot::imPartEnd - mandlebrot::imPartStart);

				auto numIterations = mandlebrot::mandlebrot(cNumber);

				auto color = mandlebrot::getColor(numIterations);

				image->fill(color.R, color.G, color.B, yCoord, xCoord);
			}
		}

		//save original image image
		image->write("./mandlebrot.png");
	}


	std::cout << "Generating mandlebrot image(serial).. Done\n";
}

void MandlebrotSerial_SSE()
{
	mandlebrot::GenerateMandlebrodColors();

	MeasureExecution ex("MandlebrotSerialSSE2");

	std::cout << "Generating mandlebrot image SSE2(serial)..\n";

	alignas(64) auto image = std::make_shared<ch01::Image>("mandlebrotsse2",
		mandlebrot::imageWidth, mandlebrot::imageHeight);

	{
		const auto realDifference = _mm_set_ps1(mandlebrot::realPartEnd - mandlebrot::realPartStart);
		const auto imDifference = _mm_set_ps1(mandlebrot::imPartEnd - mandlebrot::imPartStart);

		const auto realPartStart = _mm_set_ps1(mandlebrot::realPartStart);
		const auto imPartStart = _mm_set_ps1(mandlebrot::imPartStart);
		const auto imageWidth = _mm_set_ps1((float)mandlebrot::imageWidth);
		const auto imageHeight = _mm_set_ps1((float)mandlebrot::imageHeight);
		const auto stableThresh = _mm_set_ps1(4.0f);
		const auto two = _mm_set_ps1(2.0f);
		const __m128 xIndicesIncrement = {3.f, 2.f, 1.f,  0.f};

		alignas(64) static const __m128i iterationsIncrement[16]
		{
			{_mm_set_epi32(0 , 0, 0, 0)},
			{_mm_set_epi32(1 , 0, 0, 0)},
			{_mm_set_epi32(0, 1, 0, 0)},
			{_mm_set_epi32(1, 1, 0, 0)},
			{_mm_set_epi32(0, 0, 1, 0)},
			{_mm_set_epi32(1, 0, 1, 0)},
			{_mm_set_epi32(0, 0, 1, 1)},
			{_mm_set_epi32(1, 1, 1, 0)},
			{_mm_set_epi32(0, 0, 0, 1)},
			{_mm_set_epi32(1, 0, 0, 1)},
			 {_mm_set_epi32(0, 1, 0, 1)},
			 {_mm_set_epi32(1, 1, 0, 1)},
			 {_mm_set_epi32(0, 0, 1, 1)},
			 {_mm_set_epi32(1, 0, 1, 1)},
			 {_mm_set_epi32(0, 1, 1, 1)},
			 {_mm_set_epi32(1, 1, 1, 1)}
		};

		//iterate row wise than column wise
		for (unsigned int yCoord = 0; yCoord < mandlebrot::imageHeight; ++yCoord)
		{
			//by 4
			for (unsigned int xCoord = 0; xCoord < (mandlebrot::imageWidth >> 2); ++xCoord)
			{
				const auto xCoordBy4 = xCoord << 2;
				std::pair<float, float> cNumber{ 0.f, 0.f };

				auto yCoordS = _mm_set_ps1((float)yCoord);
				
				auto xCoords = _mm_set_ps1((float)xCoordBy4);
				xCoords = _mm_add_ps(xCoords, xIndicesIncrement);

				//realPart = mandlebrot::realPartStart + ((float)xCoord / (float)mandlebrot::imageWidth) * (mandlebrot::realPartEnd - mandlebrot::realPartStart);
				const auto realPartsIncrement = _mm_add_ps(realPartStart,
					_mm_mul_ps(
						_mm_div_ps(xCoords, imageWidth), realDifference));

				//impart = mandlebrot::imPartStart + ((float)yCoord / (float)mandlebrot::imageHeight) * (mandlebrot::imPartEnd - mandlebrot::imPartStart);
				const auto imPartsIncrement = _mm_add_ps(imPartStart,
					_mm_mul_ps(
						_mm_div_ps(yCoordS, imageHeight), imDifference));

				//Uroll mandlebrot by 4
				auto numIterations = _mm_set1_epi32(0);

				auto realParts = realPartsIncrement;
				auto imParts = imPartsIncrement;

				for (int i = 0; i < mandlebrot::maxIterations; ++i)
				{
					//real* real + imaginary * imaginary
					const auto realPartsSquare = _mm_mul_ps(realParts, realParts);
					const auto imPartsSquare = _mm_mul_ps(imParts, imParts);
					const auto added = _mm_add_ps(realPartsSquare, imPartsSquare);

					//real* real + imaginary * imaginary <= 4.0
					const auto threshMask = _mm_cmple_ps(added, stableThresh);
					const auto intMask = _mm_movemask_ps(threshMask);
					if (intMask == 0)
					{
						break;
					}
					//increment num iterations if less than threshhold
					const auto incrementer = iterationsIncrement[intMask];
					numIterations = _mm_add_epi32(numIterations, incrementer);

					//double  newReal = real * real - imaginary * imaginary;
					const auto newReal = _mm_sub_ps(realPartsSquare, imPartsSquare);
					//double  newImaginary = 2.f * real * imaginary;
					const auto newImaginary = _mm_mul_ps(two, _mm_mul_ps(realParts, imParts));

					//real = numer.first + newReal;
					realParts = _mm_add_ps(realPartsIncrement, newReal);
					//imaginary = numer.second + newImaginary;
					imParts = _mm_add_ps(imPartsIncrement, newImaginary);
				}
				///end unroll mandlebrod

				uint32_t val[4];
				memcpy(val, &numIterations, sizeof(val));

				const auto color1 = mandlebrot::getColor(val[0]);
				image->fill(color1.R, color1.G, color1.B, yCoord, xCoordBy4);

				const auto color2 = mandlebrot::getColor(val[1]);
				image->fill(color2.R, color2.G, color2.B, yCoord, xCoordBy4 + 1);

				const auto color3 = mandlebrot::getColor(val[2]);
				image->fill(color3.R, color3.G, color3.B, yCoord, xCoordBy4 + 2);

				const auto color4 = mandlebrot::getColor(val[3]);

				image->fill(color4.R, color4.G, color4.B, yCoord, xCoordBy4 + 3);
			}
		}

		//save original image image
		image->write("./mandlebrotsse2.png");

	}
	std::cout << "Generating mandlebrot image SSE2(serial).. Done\n";
}

int main()
{
	MandlebrotSerial(); //Slow 
	MandlebrotSerial_SSE();
}
