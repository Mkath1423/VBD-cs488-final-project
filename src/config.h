#pragma once

#include <string>
#include <filesystem>

struct Config {
	int height;
	int width;
	std::filesystem::path resources;
	int convergence;
	int max_iterations;

	Config() : 
		height{600}, 
		width{800},
		resources{"./resources"},
		convergence{0},
		max_iterations{ 50 }
	{};
};

Config parse_args(int argc, char* argv[]);
