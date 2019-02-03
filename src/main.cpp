#include <iostream>
#include <random>

#include <lodepng.h>

#include "engine.h"

int main()
{
	std::mt19937 randomEngine;
	cgol::Engine engine {300, 300};

	engine.populate<std::uniform_real_distribution<float>>(randomEngine, 0.5);

	//std::cout << outPretty(engine) << '\n';

	for (int i = 0; i < 100; ++i) {
		std::cout << i << ' ';
		if (i % 10 == 0)
			std::cout << '\n';
		engine.advance();
		//std::cout << outPretty(engine) << '\n';
	}

	std::cout << std::flush;

	return 0;
}