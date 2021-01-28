#pragma once
#include <string>

#include "eef.h"

class Arguments {
public:
	EEF_Config eef_cfg;
	bool divisibles = false;
	bool analyze = false;
	std::string alloc_file;
};

Arguments read_arguments(int argc, char**argv);
