#pragma once
#include "halo_types.h"

class halo2_engine
{
private:
	static halo2_engine& get() {
		static halo2_engine instance;
		return instance;
	}

	halo2_engine() = default;
	~halo2_engine() = default;

public:
};
