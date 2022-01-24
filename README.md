## anyjson

C++ JSON parser / stringifier based on STL (c++17)

# API

~~~~~~~~~~cpp
namespace anyjson {
  using Value   = std::any;
  using Object  = std::map<std::string, Value>;
  using Array   = std::vector<Value>;
  using String  = std::string;
  using Number  = double;
  using Boolean = bool;
  using Null    = std::nullptr_t;

  bool parse(std::istream&, Value&);
  bool stringify(const Value&, std::ostream&);
}
~~~~~~~~~~

As its repository name indicates, main object used to store JSON data is std::any.
So that only recent compilers like g++-7 or greater can build this source code.

* no dependency except the STL
* template method to<T> that converts std::any to other types
* print traces in debug mode
* usable in multithreaded program

# Usage

~~~~~~~~~~cpp

int main()
{
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

	} catch (const std::exception& e) {
	  std::cerr << e.what() << std::endl;
	}

	if (anyjson::stringify(data, std::cout)) {
	  return 0;
	}
	std::cout << std::endl;
  }
  return 1;
}

~~~~~~~~~~

# Limitations

* stringify offers only the compact format (no whitespace)
* stringigy uses type comparaison to unstack std::any variable

Tested on Ubuntu 20.04, g++ 9.3