#pragma once
#include <cstdint>
class VectorField
{
public:

	VectorField(uint32_t width, uint32_t height, uint32_t depth);

	~VectorField();

private:

	struct VectorFieldDimensions {

		uint32_t x;
		uint32_t y;
		uint32_t z;
		
	} dimensions;

};

