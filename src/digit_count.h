//
// Created by Bridger Holt on 8/1/18.
//

#ifndef CONWAYS_GAME_OF_LIFE_DIGIT_COUNT_H
#define CONWAYS_GAME_OF_LIFE_DIGIT_COUNT_H

template <class Int, class Out = Int>
Out digitCount(Int value, Int base = 10) {
	Out result {0};

	while (value > 0) {
		value /= base;
		++result;
	}

	return result;
}

#endif //CONWAYS_GAME_OF_LIFE_DIGIT_COUNT_H
