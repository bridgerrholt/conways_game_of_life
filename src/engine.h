//
// Created by Bridger Holt on 3/31/18.
//

#ifndef CONWAYS_GAME_OF_LIFE_ENGINE_H
#define CONWAYS_GAME_OF_LIFE_ENGINE_H

#include <brh/matrix.h>

#include <lodepng.h>

namespace cgol {

template <class T>
struct PrettyArray
{
	T const & array;
};

template <class T>
PrettyArray<T> prettyArray(T const & array) { return {array}; }

template <class OS, class T>
OS & operator<<(OS & outStream, PrettyArray<T> prettyArray) {
	auto it = prettyArray.array.begin();
	outStream << "{ ";
	while (it < prettyArray.array.end()-1) {
		outStream << *it << ", ";
		++it;
	}
	outStream << *it << " }";
	return outStream;
}

class Engine
{
	public:
		using CellState = bool;
		using CellType  = CellState;
		using MatType   = brh::Mat2<CellType>;

		Engine(std::size_t width, std::size_t height) :
			Engine(MatType({width, height})) {}

		explicit Engine(MatType mat) :
			mat_     {std::move(mat)},
			matTemp_ (mat_),
			pngData_ (mat_.size() * 4) { }

		MatType const & getMat() const {
			return mat_;
		}

		/*void setMat(MatType mat) {
			mat_ = std::move(mat);
		}*/

		template <class Dist, class R>
		void populate(R & randomEngine, typename Dist::result_type aliveChance=0.5) {
			Dist range { 0.0, 1.0 };
			for (auto && i : mat_) {
				auto num   = range(randomEngine);
				bool alive = num < aliveChance;

				i = alive;
			}
		};

		void advance() {
			auto it  = mat_.sbegin();
			auto itWrite = matTemp_.begin();
			auto end = mat_.send();
			auto firstRowLast = it - 1;
			auto lastRowFirst = end;
			auto lastRowLast = end - 1;

			firstRowLast += typename MatType::axis_array {0, 1};
			lastRowFirst -= typename MatType::axis_array {0, 1};

			auto width  = mat_.axes()[0];
			auto height = mat_.axes()[1];

			updateOuterCell<0, 0>(it, itWrite);
			while (it < firstRowLast) {
				updateOuterCell<1, 0>(it, itWrite);
			}

			updateOuterCell<0, 1>(it, itWrite);

			while (it < lastRowFirst) {
				updateOuterCell<1, 1>(it, itWrite);

				for (std::size_t i {0}; i < width-2; ++i) {
					updateInnerCell(it, itWrite);
				}

				updateOuterCell<1, 2>(it, itWrite);
			}

			updateOuterCell<0, 2>(it, itWrite);

			while (it < lastRowLast) {
				updateOuterCell<1, 3>(it, itWrite);
			}

			updateOuterCell<0, 3>(it, itWrite);

			std::string fileName = "out-" + std::to_string(iteration_) + ".png";
			writeState(fileName);

			std::swap(mat_, matTemp_);
			++iteration_;
		}

		void writeState(std::string filename) {
			auto width  = mat_.axes()[0];
			auto height = mat_.axes()[1];

			std::size_t index {0};
			MatType::axis_array axes;
			for (axes[1] = 0; axes[1] < height; ++axes[1]) {
				for (axes[0] = 0; axes[0] < width; ++axes[0]) {
					auto i = mat_[axes];
					auto value = static_cast<unsigned char>(255 * !i);

					for (std::size_t j {0}; j < 3; ++j) {
						pngData_[index + j] = value;
					}

					pngData_[index + 3] = 255;

					index += 4;
				}
			}

			unsigned error = lodepng::encode(filename.c_str(), pngData_, (unsigned)width, (unsigned)height);
			if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
		}

	private:
		using DeltaAxes = MatType::diff_array;

		template <std::size_t setCount, std::size_t setSize>
		using DeltaArray = std::array<
			std::array<DeltaAxes, setSize>, setCount>;

		using DeltaPair = std::pair<DeltaArray<4, 3>, DeltaArray<4, 5>>;

		DeltaArray<4, 3> cornerDeltas_ = {{
			{{{ 1,  0}, { 0,  1}, { 1,  1}}},
			{{{-1,  0}, {-1,  1}, { 0,  1}}},
			{{{ 0, -1}, { 1, -1}, { 1,  0}}},
			{{{-1, -1}, { 0, -1}, {-1,  0}}}
		}};

		DeltaArray<4, 5> edgeDeltas_ {{
			{{{-1,  0}, { 0,  1}, {-1,  1}, { 0,  1}, { 1,  1}}}, // Top
			{{{ 0, -1}, { 1, -1}, { 1,  0}, { 0,  1}, { 1,  1}}}, // Left
			{{{ 0, -1}, {-1, -1}, {-1,  0}, { 0,  1}, {-1,  1}}}, // Right
			{{{-1,  0}, { 0, -1}, {-1, -1}, { 0, -1}, {-1, -1}}}  // Bottom
		}};

		static constexpr DeltaPair deltaPair() {
			constexpr DeltaPair pair {
				{{ // Corners
					{{{ 1,  0}, { 0,  1}, { 1,  1}}},
					{{{-1,  0}, {-1,  1}, { 0,  1}}},
					{{{ 0, -1}, { 1, -1}, { 1,  0}}},
					{{{-1, -1}, { 0, -1}, {-1,  0}}}
				}},

				{{ // Edges
					{{{-1,  0}, { 1,  0}, {-1,  1}, { 0,  1}, { 1,  1}}}, // Top
					{{{ 0, -1}, { 1, -1}, { 1,  0}, { 0,  1}, { 1,  1}}}, // Left
					{{{-1, -1}, { 0, -1}, {-1,  0}, {-1,  1}, { 0,  1}}}, // Right
					{{{-1, -1}, { 0, -1}, { 1, -1}, {-1,  0}, { 1,  0}}}  // Bottom
				}}};


			/*// Corners
			std::get<0>(pair) = {{
				{{{ 1,  0}, { 0,  1}, { 1,  1}}},
				{{{-1,  0}, {-1,  1}, { 0,  1}}},
				{{{ 0, -1}, { 1, -1}, { 1,  0}}},
				{{{-1, -1}, { 0, -1}, {-1,  0}}}
			}};

			// Edges
			std::get<1>(pair) = {{
				{{{-1,  0}, { 0,  1}, {-1,  1}, { 0,  1}, { 1,  1}}}, // Top
				{{{ 0, -1}, { 1, -1}, { 1,  0}, { 0,  1}, { 1,  1}}}, // Left
				{{{ 0, -1}, {-1, -1}, {-1,  0}, { 0,  1}, {-1,  1}}}, // Right
				{{{-1,  0}, { 0, -1}, {-1, -1}, { 0, -1}, {-1, -1}}}  // Bottom
			}};*/

			return pair;
		}


		template <class It>
		static bool calcCellSurvives(It it, unsigned char sum) {
			if (*it) {
				if (sum < 2 || sum > 3)
					return false;
			}
			else if (sum == 3)
				return true;

			return *it;
		}

		template <class SIt, class It>
		void advanceIts(SIt & it, It & itWrite) {
			++it;
			++itWrite;
		}

		template <class SIt, class It>
		void updateInnerCell(SIt & it, It & itWrite) {
			*itWrite = calcInnerCellSurvives(it);
			advanceIts(it, itWrite);
		}

		template <std::size_t N, std::size_t M, class SIt, class It>
		void updateOuterCell(SIt & it, It & itWrite) {
			*itWrite = calcOuterCellSurvives<N, M>(it);
			advanceIts(it, itWrite);
		}

		template <std::size_t N, class SIt, class It>
		void updateCornerCell(SIt & it, It & itWrite) {
			*itWrite = calcCornerCellSurvives<N>(it);
			advanceIts(it, itWrite);
		}

		template <std::size_t N, class SIt, class It>
		void updateEdgeCell(SIt const & it, It & itWrite) {
			*itWrite = calcEdgeCellSurvives<N>(it);
			advanceIts(it, itWrite);
		}

		template <class SIt, class It>
		void updateInnerCell(SIt const & it, It & itWrite) {
			*itWrite = calcInnerCellSurvives(it);
			advanceIts(it, itWrite);
		}

		template <std::size_t N, std::size_t M, class It>
		bool calcOuterCellSurvives(It const & it) {
			auto sum = countOuterCellNeighbors<N, M>(it);
			return calcCellSurvives(it, sum);
		}

		template <std::size_t N, class It>
		bool calcCornerCellSurvives(It const & it) {
			auto sum = countCornerCellNeighbors<N>(it);
			return calcCellSurvives(it, sum);
		}

		template <std::size_t N, class It>
		bool calcEdgeCellSurvives(It const & it) {
			auto sum = countEdgeCellNeighbors<N>(it);
			return calcCellSurvives(it, sum);
		}

		template <class It>
		bool calcInnerCellSurvives(It const & it) {
			//std::cout << it.pos()[0] << ", " << it.pos()[1] << '\n';
			auto sum = countInnerCellNeighbors(it);
			return calcCellSurvives(it, sum);
		}

		template <std::size_t N, class It>
		unsigned char countEdgeCellNeighbors(It const & it) {
			return countOuterCellNeighbors(it, edgeDeltas_[N]);
		}


		template <std::size_t N, class It>
		unsigned char countCornerCellNeighbors(It const & it) {
			return countOuterCellNeighbors(it, cornerDeltas_[N]);
		}

		template <std::size_t N, std::size_t M, class It>
		unsigned char countOuterCellNeighbors(It const & it) {
			return countOuterCellNeighbors(it, std::get<N>(deltaPair())[M]);
		}

		template <class It, class DeltaArr>
		unsigned char countOuterCellNeighbors(It it, DeltaArr const & deltas) {
			/*unsigned char sum {0};
			auto itPos = it.pos();
			for (auto const & i : deltas) {
				//std::cout << it.pos()[0] << ", " << it.pos()[1] << '\n';
				sum += *(it + i);
			}

			auto s2 = countCellNeighbors(it);
			//std::cout << prettyArray(it.pos()) << " : " << (int)sum << ", " << (int)s2 << '\n';
			assert(sum == s2);
			return sum;*/

			return countCellNeighbors(it);
		}

		template <class It>
		unsigned char countInnerCellNeighbors(It it) {
			/*unsigned char sum {0};
			DeltaAxes delta;
			for (delta[1] = -1; delta[1] < 2; ++delta[1]) {
				for (delta[0] = -1; delta[0] < 2; ++delta[0]) {
					if (delta[0] != 0 || delta[1] != 0)
						sum += *(it + delta);
				}
			}

			auto s2 = countCellNeighbors(it);
			//std::cout << prettyArray(it.pos()) << " : " << (int)sum << ", " << (int)s2 << '\n';
			assert(sum == s2);
			return sum;*/

			return countCellNeighbors(it);
		}

		template <class It>
		unsigned char countCellNeighbors(It it) {
			unsigned char sum {0};
			DeltaAxes delta;
			for (delta[1] = -1; delta[1] < 2; ++delta[1]) {
				for (delta[0] = -1; delta[0] < 2; ++delta[0]) {
					if (delta[0] != 0 || delta[1] != 0) {
						auto pos = it.pos();
						auto outOfBounds = false;

						for (std::size_t i {0}; i < 2; ++i) {
							if (delta[i] == -1 && pos[i] == 0) {
								outOfBounds = true;
								break;
							}
							else if (delta[i] == 1 && pos[i] == mat_.axes()[i]-1) {
								outOfBounds = true;
								break;
							}
						}

						if (!outOfBounds)
							sum += *(it + delta);
					}
				}
			}

			return sum;
		}

		std::size_t matWidth()  { return mat_.axes()[0]; }
		std::size_t matHeight() { return mat_.axes()[1]; }

		MatType mat_;
		MatType matTemp_;
		std::vector<unsigned char> pngData_;
		std::size_t iteration_ {0};
};

struct EngineOutputPretty { Engine const & engine; };

EngineOutputPretty outPretty(Engine const & engine) { return {engine}; }

template <class OS>
OS & operator<<(OS & outStream, EngineOutputPretty output) {
	auto const & mat   = output.engine.getMat();
	auto         width = mat.axes()[0];

	auto it = mat.begin();
	while (it < mat.end()) {
		for (decltype(width) i {0}; i < width; ++i) {
			outStream << *it;
			++it;
		}
		outStream << '\n';
	}

	return outStream;
}

}

#endif // CONWAYS_GAME_OF_LIFE_ENGINE_H
