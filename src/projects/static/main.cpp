#include <iostream>
#include <random>

#include <engine.h>
#include <map_engine.h>
#include <write_gif.h>

int main()
{
  std::mt19937 randomEngine;
  MapEngine<> engine;// {300, 300};

  engine.populate<std::uniform_real_distribution<float>>(randomEngine, {1000, 1000}, 0.05);

  //std::cout << outPretty(engine) << '\n';

  //std::cout << std::flush;

  MapEngine<>::StateType state;

  //MapEngine<> mapEngine;
  //mapEngine.populate(state);

  //engine.writePng("../../../../output/static/test-");

  for (int i = 1; i <= 1000; ++i) {
    //std::cout << i << ' ';
    if (i % 100 == 0)
      std::cout << i << '\n';
    engine.update();
    //std::cout << outPretty(engine) << '\n';
  }

  //engine.writePng("../../../../output/static/map-test-");

  writeGif(engine, "../../../../output/static/gif-test-3");

  return 0;
}