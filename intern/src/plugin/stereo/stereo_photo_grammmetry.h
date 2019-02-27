#pragma once

#ifndef PhotoGrammmetry_H
#define PhotoGrammmetry_H

#include <vector>

#include "base/base_include_glm.h"

namespace Stereo
{
	class PhotoGrammmetry
	{
	public:
		PhotoGrammmetry();
		~PhotoGrammmetry();

	public:

	private:
		std::vector<char> Img;
		glm::mat3 K_mtx;
		glm::mat3x4 P_mtx;
	};
} // Stereo

#endif
