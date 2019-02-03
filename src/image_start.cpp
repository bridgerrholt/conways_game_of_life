#include <iostream>
#include <random>
#include <fstream>

#include <lodepng.h>

#include "engine.h"
#include "map_engine.h"

int main()
{
	using MatType = cgol::Engine::MatType;


	MatType matTest {{5, 5}};
	for (int j {0}; j < 5; ++j) {
		for (int i {0}; i < 5; ++i) {
			std::cout << i << ", " << j << ": " << matTest.index({i, j}) << '\n';
		}
	}

	std::string filename = "/Users/bridgerholt/Desktop/conway.data";
	//std::string filename = "/Users/bridgerholt/Desktop/test.data";
	std::size_t width {379}, height {192}, channels {4};

	auto size = width * height * channels;

	MatType::value_array data;
	data.resize(width * height, false);

	std::ifstream file {filename, std::ios::binary};

	assert(file.good());

	std::vector<char> buffer((
		                         std::istreambuf_iterator<char>(file)),
	                         (std::istreambuf_iterator<char>()));

	MapEngine<>::StateType state;

	char c;
	for (std::size_t i {0}; i < size;) {
		if ((i + 1) % channels == 0) {
			++i;
			continue;
		}

		//file.get(c);

		c = buffer[i];

		if (c != -1) {
			data[i / channels] = true;

			// Round up to nearest factor of 4 (channels).
			i = ((i + channels) / channels) * channels;
		}
		else
			++i;
	}


	MatType mat {{width, height}/*, data*/};

	MatType::axis_array axes {0, 0};

	std::size_t i {0};
	for (axes[1] = 0; axes[1] < height; ++axes[1]) {
		for (axes[0] = 0; axes[0] < width; ++axes[0]) {
			mat[axes] = data[i];
			++i;

			if (data[i] == true)
				state.add({(short)axes[0], (short)axes[1]});
		}
	}

	/*mat = MatType {{5, 5}};

	mat[{0, 0}] = true;
	mat[{1, 0}] = true;
	mat[{2, 0}] = true;
	mat[{3, 0}] = true;
	mat[{4, 0}] = true;*/

	MapEngine<>::StateType state2;
	state2.add({0, 0});
	state2.add({0, 1});
	state2.add({1, 1});
	state2.add({1, 0});

	MapEngine<> mapEngine;
	mapEngine.populate(state);

	mapEngine.writePng("test-");

	for (int i = 0; i < 500; ++i) {
		std::cout << i << ' ';
		if (i % 10 == 0)
			std::cout << '\n';
		mapEngine.update();
		//std::cout << outPretty(engine) << '\n';
	}

	mapEngine.writePng("map-test-");


	/*cgol::Engine engine {mat};
	engine.writeState("test.png");

	//std::cout << outPretty(engine) << '\n';

	for (int i = 0; i < 1000; ++i) {
		std::cout << i << ' ';
		if (i % 10 == 0)
			std::cout << '\n';
		engine.advance();
		//std::cout << outPretty(engine) << '\n';
	}

	std::cout << std::flush;*/

	return 0;
}