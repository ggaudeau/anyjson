
#include <fstream>
#include <iostream>
#include <cassert>
#include <chrono>
#include <sstream>

#include "anyjson.hpp"
#include "anyjson_tools.hpp"

//#define WITH_CHRONO

#if defined(WITH_CHRONO)
class Chrono {
  std::chrono::time_point<std::chrono::steady_clock> const m_start;
  std::string const m_text;

public:
  Chrono(const std::string& l_text)
	: m_start(std::chrono::steady_clock::now())
	, m_text(l_text)
  {	
  }
  
  ~Chrono()
  {
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - m_start;
	std::cout << "Elapsed time for " << m_text << " = " << elapsed_seconds.count() << "s\n";
  }  
};
#endif


int test_input_file(const char* filePath) {
  int retcode = 0;
  std::fstream ifs(filePath);

  if (ifs) {
	std::any data;
	{
#if defined(WITH_CHRONO)
	  Chrono chrono("parsing");
#endif	  
	  retcode = (anyjson::parse(ifs, data)) ? 0 : 1;
	}

	if (retcode == 0) {
	  std::stringstream oss;
	  {
#if defined(WITH_CHRONO)
		Chrono chrono("stringifying");
#endif		
		retcode += (anyjson::stringify(data, oss)) ? 0 : 1;

		if (retcode == 0) {
		  std::cout << oss.str() << std::endl;
		}
	  }
	}
	
  } else {
	std::cerr << "cant open file: " << filePath << std::endl;
	retcode = 4;
  }
  return retcode;
}

int test_simple_json() {
  int retcode = 0;
  std::stringstream iss("{ \"item\": { \"value\": 0 } }");
  std::any data;

  if (anyjson::parse(iss, data)) {

	try {
	  anyjson::Object& root = to<anyjson::Object&>(data);
	  anyjson::Object::iterator rootIt = root.find("item");
	  
	  if (rootIt != root.end()) {
		anyjson::Object& item = to<anyjson::Object&>(rootIt->second);
		anyjson::Object::iterator itemIt = item.find("value");

		if (itemIt != item.end()) {
		  anyjson::Number& value = to<anyjson::Number&>(itemIt->second);

		  value = 42;
		}
	  }
	  
	  retcode += anyjson::stringify(data, std::cout);

	  std::cout << std::endl;
	  
	} catch (const std::out_of_range& oof) {
	  std::cerr << "key not mapped : " << oof.what() << std::endl;
	  retcode = 2;
	} catch (const std::bad_any_cast&) {
	  std::cerr << "bad any cast" << std::endl;
	  retcode = 4;
	} catch (const std::exception& err) {
	  std::cerr << err.what() << std::endl;
	  retcode = 8;
	}
	
  } else {
	std::cerr << "parse failed" << std::endl;
	retcode = 1;
  }
  if (retcode > 0) {
	retcode += 64;
  }
  return retcode;
}

#include <string.h>

int main(int argc, char* argv[])
{
  int retcode = 0;
  
  if (argc > 1) {
	if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
	  std::cerr << "anyjson parser" << std::endl;
	  std::cerr << "\tusage: " << argv[0] << " [json file]?" << std::endl;
	  retcode = 128;
	} else {	  	
	  retcode = test_input_file(argv[1]);
	}	
  } else {
	retcode = test_simple_json();
  }
  return retcode;
}

