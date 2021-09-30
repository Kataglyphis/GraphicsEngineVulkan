#pragma once

// Indices (locations) of Queue families (if they exist at all)
struct QueueFamilyIndices {

	int graphics_family = -1;									// location of graphics family

	//check if queue families are valid 
	bool is_valid() {
		return graphics_family >= 0;
	}

};
