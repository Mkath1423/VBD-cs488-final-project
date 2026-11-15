#include "config.h"
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <map>

namespace {
	/*
		positional args
		 map from position to their effect

		keyword args --name val
		 map from name to effect

		short args -s val
		 map from s to name

		flag args
		 what kwargs are flags and what are their default values
	*/


	struct ArgInfo {
		std::string name;
		std::string description; 
		bool isflag;

		void (*parser)(Config&, std::string);

		ArgInfo(void (*fn)(Config&, std::string), std::string name) : 
			parser{fn}, 
			name { name },
			isflag{ false } {
		}
	};

	const std::vector<ArgInfo> positionalArgs = {
		 ArgInfo([](Config& c, std::string v) {c.resources = v; }, "resources")
	};

	const std::map<std::string, ArgInfo> optionalArgs = {
		{"height", ArgInfo([](Config& c, std::string v) {c.height = std::stoi(std::string(v)); }, "height") },
		{ "width", ArgInfo([](Config& c, std::string v) {c.width = std::stoi(std::string(v)); }, "width") },
		{ "convergence", ArgInfo([](Config& c, std::string v) {c.convergence = std::stoi(std::string(v)); }, "convergence") },
		{ "iterations", ArgInfo([](Config& c, std::string v) {c.max_iterations = std::stoi(std::string(v)); }, "iterations") }
	};

	const std::map<char, std::string> shorthands = {
		{ 'h', "height"},
		{ 'w', "width"},
		{ 'c', "convergence"},
		{ 'i', "iterations"}
	};
}


bool parse_named(Config& config, std::string name, int& i, int argc, char* argv[]) {
	auto it = optionalArgs.find(name);
	if (it == optionalArgs.end()) return false;
	
	const ArgInfo& info = it->second;

	if (info.isflag) {
		// the flag parse function will just do whatever default behavior it needs
		// it does not need a value from the command line
		info.parser(config, std::string());
		return true;
	}

	else if (i + 1 < argc) {
		info.parser(config, std::string(argv[i + 1]));
		i += 1;
		return true;
	}

	return false;
}


Config parse_args(int argc, char* argv[]) {
	Config config;
	
	int i = 1;

	// positional args
	while (i - 1 < positionalArgs.size() && i < argc) {
		std::string val(argv[i]);

		// no longer specifying positional args
		if (val.starts_with('-')) break;

		positionalArgs.at(i - 1).parser(config, val);

		i += 1;
	}

	// Optional args
	for (; i < argc; ++i) {
		std::string arg(argv[i]);

		if (arg.starts_with("--")) {
			parse_named(config, arg.substr(2), i, argc, argv);
		}
		else if (arg.starts_with('-')) {

			for (char c : arg.substr(1)) {
				auto it = shorthands.find(c);
				if (it == shorthands.end()) continue;

				// If the user multi shorthands a value arg then so be it.
				// I also dont care if they try to specify it multiple times, both are UB
				parse_named(config, it->second, i, argc, argv);
			}
		}
	}

	return config;
}
