
#include <fstream>
#include <iostream>
#include <cassert>
#include <chrono>
#include <sstream>

#include "json.hpp"

int main(int argc, char* argv[])
{
  if (argc > 1) {
	std::fstream ifs(argv[1]);

	if (ifs) {
	  auto start = std::chrono::steady_clock::now();

	  std::any data;
	  int retcode = json::parseValue(ifs, data);

	  auto end = std::chrono::steady_clock::now();
	  std::chrono::duration<double> elapsed_seconds = end-start;
	  std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

	  assert(retcode == 0);

	  // TODO: create std::any accessors such as:
	  //  - bool is<T>(any)
	  //  - T to<T>(any)
	  //  - T value<T>(any of object, key)

	  start = std::chrono::steady_clock::now();

	  std::stringstream oss;

	  json::stringifyValue(data, oss);

	  end = std::chrono::steady_clock::now();
	  elapsed_seconds = end-start;
	  std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

	  assert(retcode == 0);

	  return retcode;
	}

  } else {
	std::cerr << "anyjson parser" << std::endl;
	std::cerr << "\tusage: " << argv[0] << " <json file>" << std::endl;
  }

  return 0;
}
