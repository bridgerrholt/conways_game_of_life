//
// Created by Bridger Holt on 8/1/18.
//

#ifndef CONWAYS_GAME_OF_LIFE_MAP_ENGINE_H
#define CONWAYS_GAME_OF_LIFE_MAP_ENGINE_H

#include <vector>
#include <set>
#include <string>
#include <lodepng.h>
#include <brh/matrix.h>
#include <iostream>

#include "digit_count.h"

template <class Int>
class Position
{
	public:
		Position() : Position(0, 0) {}
		Position(Int x, Int y) : x {x}, y {y} {}

		Int x;
		Int y;

		Position operator+(Position const & other) const {
			return {Int(x + other.x), Int(y + other.y)};
		}

		Position operator-() const {
			return {Int(-x), Int(-y)};
		}

		/// Changes the values within min and max if this object has a lower or
		/// greater value.
		void changeMinMax(Position & min, Position & max) {
			if (x < min.x)
				min.x = x;
			else if (x > max.x)
				max.x = x;

			if (y < min.y)
				min.y = y;
			else if (y > min.y)
				max.y = y;
		}

		void changeMin(Position & min) {
			if (x < min.x)
				min.x = x;

			if (y < min.y)
				min.y = y;
		}

		void changeMax(Position & max) {
			if (x > max.x)
				max.x = x;

			if (y > max.y)
				max.y = y;
		}

		bool operator<(Position const & other) const {
			if (y < other.y)
				return true;

			else if (y == other.y) {
				if (x < other.x)
					return true;
			}

			return false;
		}
};


template <class Int>
class EngineState
{
	public:
		using PosType = Position<Int>;
		using SetType = std::set<PosType>;

		// Returns true if pos wasn't already in posSet.
		bool add(PosType pos) {
			if (set_.empty()) {
				min_ = pos;
				max_ = pos;
			}
			else {
				if (inSet(pos))
					return false;

				changeMinMax(pos);
			}

			set_.insert(pos);
			return true;
		}

		EngineState update() {
			EngineState newState;

			SetType updated;

			for (auto pos : set_) {
				auto count = countNeighbors(pos);

				if (livesOn(count))
					newState.add(pos);

				updated.insert(pos);

				callOnNeighbors(pos, updateDead, std::ref(newState), std::ref(*this), std::ref(updated));
			}

			return newState;
		}

		SetType const &  posSet() const { return set_; }

		PosType minPos() const { return min_; }
		PosType maxPos() const { return max_; }


	private:
		static bool livesOn(int neighborCount) {
			return neighborCount == 2 || neighborCount == 3;
		}

		static bool getsBorn(int neighborCount) {
			return neighborCount == 3;
		}

		static bool alive(bool living, int neighborCount) {
			if (living)
				return livesOn(neighborCount);
			else
				return getsBorn(neighborCount);
		}

		void changeMinMax(PosType pos) {
			pos.changeMinMax(min_, max_);
		}

		static void updateDead(
			PosType pos, EngineState & newState,
			EngineState & state, SetType & updated)
		{
			if (inSet(updated, pos))
				return;
			if (state.inSet(pos))
				return;

			auto count = state.countNeighbors(pos);

			if (getsBorn(count))
				newState.add(pos);

			updated.insert(pos);
		}

		static void countNeighbor(PosType pos, EngineState & state, int & count) {
			if (state.inSet(pos))
				++count;
		}

		static bool inSet(SetType const & set, PosType const & pos) {
			auto it = set.find(pos);

			return it != set.end();
		}

		bool inSet(PosType const & pos) {
			return inSet(set_, pos);
		}

		template <class Function, class ... ArgType>
		void callOnNeighbors(PosType pos, Function func, ArgType ... args) {
			PosType delta;

			delta.y = -1;

			for (delta.x = -1; delta.x <= 1; ++delta.x) {
				func(pos + delta, std::forward<ArgType>(args)...);
			}

			func(pos + PosType(-1, 0), std::forward<ArgType>(args)...);
			func(pos + PosType( 1, 0), std::forward<ArgType>(args)...);

			delta.y = 1;

			for (delta.x = -1; delta.x <= 1; ++delta.x) {
				func(pos + delta, std::forward<ArgType>(args)...);
			}
		}

		int countNeighbors(PosType pos) {
			return countNeighbors(*this, pos);
		}

		static int countNeighbors(EngineState & state, PosType pos) {
			int count {0};
			state.callOnNeighbors(pos, countNeighbor, std::ref(state), std::ref(count));
			return count;
		}


		SetType set_;
		PosType min_;
		PosType max_;
};



template <class Int = short>
class MapEngine
{
	public:
		using StateType = EngineState<Int>;
		using PosType = Position<Int>;
		using SetType = std::set<PosType>;
		using PngMat = brh::Mat3<unsigned char>;

		class FrameRange
		{
			public:
				FrameRange(std::size_t end) :
					FrameRange(0, end) {}

				FrameRange(std::size_t begin, std::size_t end) :
					begin {begin}, end {end} {}

				std::size_t begin;
				std::size_t end;
		};

		class Rect
		{
			public:
				PosType min;
				PosType max;
		};


		void populate(StateType state) {
			auto stateMin = state.minPos();
			auto stateMax = state.maxPos();

			if (stateList_.empty()) {
				min_ = stateMin;
				max_ = stateMax;
			}
			else {
				stateMin.changeMin(min_);
				stateMax.changeMax(max_);
			}

			stateList_.emplace_back(std::move(state));
		}

		// Copied from Engine
    template <class Dist, class R>
    void populate(
      R & randomEngine, PosType dimensions,
      typename Dist::result_type aliveChance=0.5)
    {
		  StateType state;

      Dist range { 0.0, 1.0 };
      for (Int y {0}; y < dimensions.y; y++)
      {
        for (Int x {0}; x < dimensions.x; x++)
        {
          auto num = range(randomEngine);
          bool alive = num < aliveChance;

          if (alive)
            state.add({x, y});
        }
      }

      stateList_.emplace_back(std::move(state));
    };


		void update() {
			auto newState = currentState().update();
			newState.minPos().changeMin(min_);
			newState.maxPos().changeMax(max_);
			stateList_.emplace_back(std::move(newState));
		}

		void writePng(std::string const & filePrefix) {
			writePng(filePrefix, defaultRange());
		}

		void writePng(std::string const & filePrefix, FrameRange range) {
			writePng(filePrefix, range, defaultRect());
		}

		void writePng(std::string const & filePrefix, Rect rect) {
			writePng(filePrefix, defaultRange(), rect);
		}

		void writePng(
			std::string const & filePrefix, FrameRange range, Rect rect)
		{
			auto end = range.end;
			auto begin = range.begin;
			auto frames = end - begin;

			assert(begin < end);
			assert(end <= frameCount());

			auto min = rect.min;
			auto max = rect.max;


			PosType delta = -min;

			assert(min.x < max.x);
			assert(min.y < max.y);

			auto width = max.x - min.x;
			auto height = max.y - min.y;

			auto digits = digitCount(frames);

			auto fileName = filePrefix;
			auto prefixSize = fileName.size();

			PngMat mat {{4, width, height}};

			// Set alpha channel to 255.
			auto matIt = mat.begin();
			matIt += 3;
			for (; matIt != mat.end() + 3; matIt += 4) {
				*matIt = 255;
			}

			for (auto i = begin; i < end; ++i) {
				auto indexStr = std::to_string(i);
				while (indexStr.size() < digits)
					indexStr.insert(indexStr.begin(), '0');

				fileName += indexStr;
				fileName += ".png";

				writePng(fileName, stateList_[i].posSet(), rect, delta, mat);
				fileName.resize(prefixSize);
			}
		}

		std::size_t frameCount() const {
			return stateList_.size();
		}

		StateType & currentState() {
			assert(!stateList_.empty());

			return stateList_.back();
		}

    StateType & state(std::size_t index)
    {
      return stateList_[index];
    }

    StateType const & state(std::size_t index) const
    {
      return stateList_[index];
    }

    FrameRange defaultRange() const {
      return {0, frameCount()};
    }

    Rect defaultRect() const {
      return {min_, max_ + PosType(1, 1)};
    }


	private:
		using StateList = std::vector<StateType>;

		void writePng(
			std::string const & fileName, SetType const & posSet,
			Rect rect, PosType delta, PngMat & mat)
		{
			// Set RGB to 0 0 0.
			auto matIt = mat.begin();
			for (; matIt != mat.end(); matIt += 4) {
				for (std::size_t i {0}; i < 3; ++i)
					*(matIt + i) = 255;
			}

			auto min = rect.min;
			auto max = rect.max;

			for (auto pos : posSet) {
				if (pos.x >= min.x && pos.x < max.x &&
				    pos.y >= min.y && pos.y < max.y)
				{
					for (std::size_t c {0}; c < 3; ++c) {
						mat[{c, pos.x + delta.x, pos.y + delta.y}] = 0;
					}
				}
			}

			unsigned error = lodepng::encode(fileName.c_str(), mat.data(), (unsigned)(max.x-min.x), (unsigned)(max.y-min.y));
			if (error)
				std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
		};

		StateList stateList_;

		std::size_t frameCount_ {0};
		PosType min_;
		PosType max_;
};

#endif //CONWAYS_GAME_OF_LIFE_MAP_ENGINE_H
