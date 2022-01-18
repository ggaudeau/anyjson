
#include <fstream>
#include <iostream>
#include <cassert>

#include "json.hpp"

int main(int argc, char* argv[])
{
  if (argc > 1) {
	std::fstream ifs(argv[1]);

	if (ifs) {
	  std::any out;

	  int retcode = json::parse(ifs, out);

	  assert(retcode == 0);

	  // TODO: create std::any accessors such as:
	  //  - bool is<T>(any)
	  //  - T to<T>(any)
	  //  - T value<T>(any of object, key)

	  json::stringify(out, std::cout);

	  assert(retcode == 0);
	}

  } else {
	std::cerr << "anyjson parser" << std::endl;
	std::cerr << "\tusage: " << argv[0] << " <json file>" << std::endl;
  }

  return 0;
}
