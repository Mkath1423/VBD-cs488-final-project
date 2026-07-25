#pragma once

#include <string>
#include <filesystem>

struct Config {
	int height;
	int width;
	std::filesystem::path resources;

	Config() : 
		height{600}, 
		width{800},
		resources{"./resources"}
	{};
};

Config parse_args(int argc, char* argv[]);
