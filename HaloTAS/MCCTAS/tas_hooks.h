#pragma once

#include "pch.h"
#include <vector>

class tas_hooks
{
public:
	tas_hooks();
	~tas_hooks();

	void attach_all();
	void detach_all();
};

