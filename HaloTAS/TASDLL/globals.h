#pragma once

#include <memory>
#include "tas_input_handler.h"
#include "halo_engine.h"

using std::unique_ptr;

extern unique_ptr<halo_engine> gEngine;
extern unique_ptr<tas_input_handler> gInputHandler;