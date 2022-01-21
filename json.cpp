
#include "json.hpp"

#include <string.h>		// memset
#include <ctype.h>		// isdigit

#include <iomanip>		// quoted c++14
#include <sstream>

#include <typeindex>
#include <functional>
#include <unordered_map>

void print() {
}

#if defined(NDEBUG)
void println() {
}
template<typename Elm, typename ...Args>
void print(Elm&&, Args&& ...) {
}
template<typename Elm, typename ...Args>
void println(Elm&&, Args&& ...) {
}
#else
void println() {
  std::cout << std::endl;
}
template<typename Elm, typename ...Args>
void print(Elm&& elm, Args&& ...args) {
  std::cout << std::forward<Elm>(elm);
  print(std::forward<Args>(args)...);
}
template<typename Elm, typename ...Args>
void println(Elm&& elm, Args&& ...args) {
  std::cout << std::forward<Elm>(elm);
  println(std::forward<Args>(args)...);
}
#endif

/// PARSING PART

namespace {
  void consumeChar(std::istream& is, char expected) {
	char ch;

	print("consuming char ... ");
	is.get(ch);
	println(ch);
	if (ch != expected) {
	  std::ostringstream oss;
	  oss << "expect " << expected << " have [" << ch << "]";
	  throw std::runtime_error(oss.str());
	}
  }

  inline void consumeWhitespaces(std::istream& is) {
	//print("consuming whitespaces ... ");
	is >> std::ws;
	//println("ok");
  }

  void parse(std::istream& is, std::string& str) {
	print("parsing string ... ");
	is >> std::quoted(str);
	println(str);
  }

  void parse(std::istream&, json::Value&);
  void parse(std::istream& is, json::Object& obj) {
	bool bContinue;

	println("parse object");
	consumeChar(is, '{');
	consumeWhitespaces(is);

	do {
	  char ch = static_cast<char>(is.peek());

	  bContinue = false;
	  if (ch == '"') {
		std::string key;

		parse(is, key);
		if (obj.find(key) != obj.cend()) {
		  std::ostringstream oss;
		  oss << "duplicate object key [" << key << "]";
		  throw std::runtime_error(oss.str());
		}

		consumeWhitespaces(is);
		consumeChar(is, ':');

		{
		  json::Value val;

		  consumeWhitespaces(is);
		  parse(is, val);
		  consumeWhitespaces(is);

		  obj.emplace(key, std::move(val));
		}

		ch = static_cast<char>(is.peek());

		if (ch == ',') {
		  consumeChar(is, ',');
		  consumeWhitespaces(is);

		  bContinue = true;

		} else if (ch == '}') {
		  // do nth

		} else {
		  std::ostringstream oss;
		  oss << "unexpected character [" << ch << "] in object (2)";
		  throw std::runtime_error(oss.str());
		}

	  } else if (ch == '}') {
		// do nth

	  } else {
		std::ostringstream oss;
		oss << "unexpected character [" << ch << "] in object (1)";
		throw std::runtime_error(oss.str());
	  }

	} while (bContinue);

	consumeWhitespaces(is);
	consumeChar(is, '}');

	println("object parsed");
  }

  void parse(std::istream& is, json::Array& arr) {
	println("parse array");

	bool bContinue;
	char ch;

	consumeChar(is, '[');

	ch = static_cast<char>(is.peek());
	if (ch != ']') {
	  do {
		bContinue = false;

		{
		  json::Value val;

		  consumeWhitespaces(is);
		  parse(is, val);
		  consumeWhitespaces(is);

		  arr.emplace_back(std::move(val));
		}

		ch = is.peek();

		if (ch == ',') {
		  consumeChar(is, ',');

		  bContinue = true;

		} else if (ch == ']') {
		  // do nth

		} else {
		  std::ostringstream oss;
		  oss << "unexpected character [" << ch << "] in array";
		  throw std::runtime_error(oss.str());
		}

	  } while (bContinue);

	} else {
	  consumeWhitespaces(is);
	}
	consumeChar(is, ']');

	println("array parsed");
  }

  void parse(std::istream& is, double& num) {
	print("parsing double ... ");
	is >> num;
	println(num);
  }

  void parse(std::istream& is, json::Value& val) {
	int ch = is.peek();

	switch ( static_cast<char>(ch) ) {
	case '"': {
	  std::string str;
	  parse(is, str);
	  val = std::move(str);
	  break;
	}
	case '{': {
	  json::Object obj;
	  parse(is, obj);
	  val = std::move(obj);
	  break;
	}
	case '[': {
	  json::Array arr;
	  parse(is, arr);
	  val = std::move(arr);
	  break;
	}
	case 't': {

	  // TRUE

	  std::string str;
	  print("parsing true ... ");
	  std::getline(is, str, 'e');
	  if (str == "tru") {
		println("ok");
		val = true;
	  } else {
		println("ko");
		std::ostringstream oss;
		oss << "expect true have [" << str << "]";
		throw std::runtime_error(oss.str());
	  }
	  break;
	}
	case 'f': {

	  // FALSE

	  std::string str;
	  print("parsing true ... ");
	  std::getline(is, str, 'e');
	  if (str == "fals") {
		println("ok");
		val = false;
	  } else {
		println("ko");
		std::ostringstream oss;
		oss << "expect false have [" << str << "]";
		throw std::runtime_error(oss.str());
	  }
	  break;
	}
	case 'n': {

	  // NULL

	  const size_t size = 4 + 1;
	  char buffer[size];

	  memset(buffer, 0, size);
	  print("parsing null ... ");
	  is.read(buffer, size - 1);
	  if (strcmp(buffer, "null") == 0) {
		println("ok");
		val = nullptr;
	  } else {
		println("ko");
		std::ostringstream oss;
		oss << "expect null have [" << buffer << "]";
		throw std::runtime_error(oss.str());
	  }
	  break;
	}
	default: {

	  // NUMBER

	  if (isdigit(ch) || static_cast<char>(ch) == '-') {
		double num;
		parse(is, num);
		val = num;

	  } else {
		std::ostringstream oss;
		oss << "unexpected character [" << ch << "] in value";
		throw std::runtime_error(oss.str());
	  }
	  break;
	}

	}
  }

}

namespace json {

  int parseValue(std::istream& is, json::Value& val)
  {
	int retcode = 0;
	std::ios_base::fmtflags flags = is.flags();
	std::ios::iostate state = is.exceptions();

	if ( ! (flags & std::ios_base::skipws) ) {
	  std::cout << "DEBUG: set skipws flag" << std::endl;
	  is.flags( flags | std::ios_base::skipws );
	}
	is.exceptions(std::istream::failbit | std::istream::badbit | std::istream::eofbit);

	try {
	  parse(is, val);

	} catch (const std::istream::failure& err) {
	  std::cerr << err.what()
				<< ", eof=" << ((is.eof()) ? 1 : 0)
				<< ", fail=" << ((is.fail()) ? 1 : 0)
				<< ", bad=" << ((is.bad()) ? 1 : 0)
				<< std::endl;
	  retcode = 1;
	} catch (const std::exception& err) {
	  std::cerr << err.what() << std::endl;
	  retcode = 1;
	}

	is.exceptions(state);
	is.flags(flags);
	return retcode;
  }

} // namespace json

/// STRINGIFY PART

static void stringify(const json::Null&, std::ostream& os) {
  os << "null";
}

static void stringify(const json::Boolean& b, std::ostream& os) {
  os << ((b) ? "true" : "false");
}

static void stringify(const json::Number& num, std::ostream& os) {
  os << num;
}

static void stringify(const json::String& str, std::ostream& os) {
  os << '"' << str << '"';
}

static void stringify(const json::Value&, std::ostream&);

static void stringify(const json::Array& arr, std::ostream& os) {
  os << '[';

  json::Array::const_iterator cit = arr.cbegin();

  while (cit != arr.cend()) {
	stringify(*cit, os);

	++cit;

	if (cit != arr.cend()) {
	  os << ',';
	}
  }

  os << ']';
}

static void stringify(const json::Object& obj, std::ostream& os) {
  os << '{';

  json::Object::const_iterator cit = obj.cbegin();

  while (cit != obj.cend()) {
	os << '"' << cit->first << '"';
	os << ":";

	stringify(cit->second, os);

	++cit;

	if (cit != obj.cend()) {
	  os << ',';
	}
  }

  os << '}';
}

//
// Unfortunately, we cant use type-safe proposed by std::any_cast
// because it involves copy of JSON object. We hope comparing type
// is enough safe...
//

static inline void _stringify_object(const json::Value& obj, std::ostream& os) {
  stringify(std::any_cast<json::Object const&>(obj), os);
}

static inline void _stringify_array(const json::Value& arr, std::ostream& os) {
  stringify(std::any_cast<json::Array const&>(arr), os);
}

static inline void _stringify_string(const json::Value& str, std::ostream& os) {
  stringify(std::any_cast<json::String const&>(str), os);
}

static inline void _stringify_number(const json::Value& num, std::ostream& os) {
  stringify(std::any_cast<json::Number const&>(num), os);
}

static inline void _stringify_boolean(const json::Value& b, std::ostream& os) {
  stringify(std::any_cast<json::Boolean const&>(b), os);
}

static inline void _stringify_null(const json::Value& n, std::ostream& os) {
  stringify(std::any_cast<json::Null const&>(n), os);
}

static void stringify(const json::Value& val, std::ostream& os) {
  using MapKeyType = std::type_index;
  using MapValueType = std::function<void(json::Value const&, std::ostream&)>;

  const std::unordered_map<MapKeyType, MapValueType>
	map {
		 { std::type_index(typeid(json::Object)), &_stringify_object },
		 { std::type_index(typeid(json::Array)), &_stringify_array },
		 { std::type_index(typeid(json::String)), &_stringify_string },
		 { std::type_index(typeid(json::Number)), &_stringify_number },
		 { std::type_index(typeid(json::Boolean)), &_stringify_boolean },
		 { std::type_index(typeid(json::Null)), &_stringify_null }
  };

  std::unordered_map<MapKeyType, MapValueType>::const_iterator cit =
	map.find( std::type_index(val.type()) );

  if (cit != map.cend()) {
	((*cit).second)(val, os);

  } else {
	std::ostringstream oss;
	oss << "unknown type index [" << val.type().name() << "]";
	throw std::runtime_error(oss.str());
  }
}

namespace json {
int stringifyValue(const json::Value& val, std::ostream& os)
{
  int retcode = 0;
  std::ios::iostate oldState = os.exceptions();

  os.exceptions(std::ostream::failbit | std::ostream::badbit | std::ostream::eofbit);

  try {
	stringify(val, os);

  } catch (const std::exception& err) {
	std::cerr << err.what() << std::endl;
	retcode = 1;
  }

  os.exceptions(oldState);
  return retcode;
}
}

