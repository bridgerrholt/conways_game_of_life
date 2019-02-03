//
// Created by Bridger Holt on 2019-02-03.
//

#ifndef CONWAYS_GAME_OF_LIFE_WRITE_GIF_H
#define CONWAYS_GAME_OF_LIFE_WRITE_GIF_H

#include <string>

extern "C"
{
#include <gifenc.h>
}

void gifFill(ge_GIF & gif, int width, int height, uint8_t value)
{
  for (int i {0}; i < width * height; i++)
  {
    gif.frame[i] = value;
  }
}



template <class MapEngine>
void writeGif(MapEngine const & mapEngine, std::string const & filePrefix) {
  writeGif(mapEngine, filePrefix, mapEngine.defaultRange());
}

template <class MapEngine>
void writeGif(
  MapEngine const & mapEngine,
  std::string const & filePrefix,
  typename MapEngine::FrameRange range)
{
  writeGif(mapEngine, filePrefix, range, mapEngine.defaultRect());
}


template <class MapEngine>
void writeGif(
  MapEngine const & mapEngine,
  std::string const & filePrefix,
  typename MapEngine::Rect rect)
{
  writeGif(mapEngine, filePrefix, mapEngine.defaultRange(), rect);
}

template <class MapEngine>
void writeGif(
  MapEngine const & mapEngine,
  std::string const & filePrefix,
  typename MapEngine::FrameRange range,
  typename MapEngine::Rect rect)
{
  auto end = range.end;
  auto begin = range.begin;
  auto frames = end - begin;

  assert(begin < end);
  assert(end <= mapEngine.frameCount());

  auto min = rect.min;
  auto max = rect.max;


  typename MapEngine::PosType delta = -min;

  assert(min.x < max.x);
  assert(min.y < max.y);

  auto width = max.x - min.x;
  auto height = max.y - min.y;

  auto fileName = filePrefix + ".gif";

  ge_GIF *gif = ge_new_gif(
    fileName.c_str(),  /* file name */
    width, height,           /* canvas size */
    (uint8_t []) {  /* palette */
      0xFF, 0xFF, 0xFF, /* 0 -> white */
      0x00, 0x00, 0x00, /* 1 -> black */
    },
    1,              /* palette depth == log2(# of colors) */
    0               /* infinite loop */
  );

  for (auto i = begin; i < end; ++i) {
    auto const & state = mapEngine.state(i);
    gifFill(*gif, width, height, 0);

    auto const & posSet = state.posSet();

    for (auto pos = posSet.begin(); pos != posSet.end(); pos++)
    {
      gif->frame[width * (pos->y + delta.y) + pos->x + delta.x] = 1;
    }

    // 5 is speed, 10 is like normal
    ge_add_frame(gif, 5);


    //writePng(fileName, stateList_[i].posSet(), rect, delta, mat);
    //fileName.resize(prefixSize);
  }

  ge_close_gif(gif);
}

#endif //CONWAYS_GAME_OF_LIFE_WRITE_GIF_H
