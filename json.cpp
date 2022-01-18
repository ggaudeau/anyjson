
#include "json.hpp"

#include <string.h>		// memset
#include <ctype.h>		// isdigit

#include <cassert>
#include <fstream>
#include <iomanip>		// quoted c++14
#include <sstream>

#include <typeindex>
#include <map>
#include <vector>
#include <functional>
#include <unordered_map>

/*#define DEBUG

#if defined(DEBUG)
# define DEBUG_PRINTF(ftm, ...) fprintf(fmt, ##__VA_ARGS__)
#else
# define DEBUG_PRINTF(fmt, ...)
#endif*/

namespace json {
  using Value	= std::any;

  using Object	= std::map<std::string, Value>;
  using Array	= std::vector<Value>;
  using String	= std::string;
  using Number	= double;
  using Boolean	= bool;
  using Null	= std::nullptr_t;
}

/// PARSING PART

static void consume_char(std::istream& is, char expected_ch) {
  char ch;

  is.get(ch);
  if (ch != expected_ch) {
	std::ostringstream oss;
	oss << "consume_char() failure: expect " << expected_ch << " have [" << ch << "]";
	throw oss;
  }
}

static void parse_number(std::istream& is, double& num) {
  is >> num;
}

static void parse_string(std::istream& is, std::string& str) {
  is >> std::quoted(str);
}

static void parse_object(std::istream&, std::map<std::string, std::any>&);
static void parse_array(std::istream&, std::vector<std::any>&);

static void parse_value(std::istream& is, std::any& val) {
  int d = is.peek();
  char c = static_cast<char>(d);

  switch (c) {
  case '"': {
	std::string str;
	parse_string(is, str);
	val = str;
	break ;
  }
  case '{': {
	auto obj = std::map<std::string, std::any>();
	parse_object(is, obj);
	val = std::move(obj);
	break ;
  }
  case '[': {
	auto arr = std::vector<std::any>();
	parse_array(is, arr);
	val = std::move(arr);
	break ;
  }
  case 't': {
	std::string str;
	std::getline(is, str, 'e');
	if (str == "tru") {
	  val = true;
	} else {
	  std::ostringstream oss;
	  oss << "parse_value() failure: expect true have [" << str << "]";
	  throw oss;
	}
	break ;
  }
  case 'f': {
	std::string str;
	std::getline(is, str, 'e');
	if (str == "fals") {
	  val = false;
	} else {
	  std::ostringstream oss;
	  oss << "parse_value() failure: expect false have [" << str << "]";
	  throw oss;
	}
	break ;
  }
  case 'n': {
	static const size_t size = 4 + 1;
	static char buffer[size];

	memset(buffer, 0, size);
	is.read(buffer, size - 1);
	if (strcmp(buffer, "null") == 0) {
	  val = nullptr;
	} else {
	  std::ostringstream oss;
	  oss << "parse_value() failure: expect null have [" << buffer << "]";
	  throw oss;
	}
	break ;
  }
  default: {
	if (isdigit(d) || c == '-') {
	  double num;
	  parse_number(is, num);
	  val = num;

	} else {
	  std::ostringstream oss;
	  oss << "parse_value() failure: unexpect char [" << c << "]";
	  throw oss;
	}
	break ;
  }
  }
}

static void parse_array(std::istream& is, std::vector<std::any>& arr)
{
  consume_char(is, '[');

  bool bContinue;
  char c = static_cast<char>( is.peek() );

  if (c != ']') { // NOT ]

	do {
	  bContinue = false;

	  std::any value;
	  is >> std::ws;
	  parse_value(is, value);
	  is >> std::ws;
	  arr.push_back(value);

	  c = is.peek();

	  if (c == ',') {
		consume_char(is, ',');

		bContinue = true;

	  } else if (c == ']') {
		// do nth

	  } else {
		std::ostringstream oss;
		oss << "parse_array() failure: unexpect char [" << c << "]";
		throw oss;
	  }

	} while (bContinue);

  } else {
	is >> std::ws;
  }
  consume_char(is, ']');
}

static void parse_object(std::istream& is, std::map<std::string, std::any>& obj)
{
  consume_char(is, '{');
  is >> std::ws;

  bool bContinue;
  char c;

  do {
	bContinue = false;

	c = static_cast<char>( is.peek() );
	if (c == '"') {

	  // KEY

	  std::string key;
	  parse_string(is, key);
	  if (obj.find(key) != obj.cend()) {
		std::ostringstream oss;

		oss << "parse_object() failure: duplicate key [" << key << "]";
		throw oss;
	  }

	  is >> std::ws;
	  consume_char(is, ':');

	  // VALUE

	  std::any value;
	  is >> std::ws;
	  parse_value(is, value);
	  is >> std::ws;
	  obj[key] = value;

	  c = static_cast<char>( is.peek() );

	  if (c == ',') {
		consume_char(is, ',');
		is >> std::ws;

		bContinue = true;

	  } else if (c == '}') {
		// do nth

	  } else {
		std::ostringstream oss;
		oss << "parse_object() VALUE failure: unexpect char [" << c << "]";
		throw oss;
	  }

	} else if (c == '}') {
	  // do nth

	} else {
	  std::ostringstream oss;
	  oss << "parse_object() KEY failure: unexpect char [" << c << "]";
	  throw oss;
	}

  } while (bContinue);

  is >> std::ws;
  consume_char(is, '}');
}

namespace json {
int parse(std::istream& is, std::any& out)
{
  int retcode = 0;
  std::ios_base::fmtflags flags = is.flags();
  std::ios::iostate state = is.exceptions();

  if ( ! (flags & std::ios_base::skipws) ) {
	std::cout << "DEBUG: set skipws flag" << std::endl;
	is.flags( flags | std::ios_base::skipws );
  }
  is.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);

  try {
	parse_value(is, out);

  } catch (const std::istream::failure& err) {
	std::cerr << err.what() << " ";
	if (is.eof()) {
	  std::cerr << "(EOF)";
	} else if (is.fail()) {
	  std::cerr << "(FAIL)";
	} else if (is.bad()) {
	  std::cerr << "(BAD)";
	} else {
	  std::cerr << "(?)";
	}	
	std::cerr << std::endl;
	retcode = 1;

  } catch (const std::ostringstream& oss) {
	std::cerr << oss.str() << std::endl;
	retcode = 1;
  }

  is.exceptions(state);
  is.flags(flags);
  return retcode;
}
}

/// STRINGIFY PART

static void stringify_null(const json::Null&, std::ostream& os) {
  os << "null";
}

static void stringify_boolean(const json::Boolean& b, std::ostream& os) {
  os << ((b) ? "true" : "false");
}

static void stringify_number(const json::Number& num, std::ostream& os) {
  os << num;
}

static void stringify_string(const json::String& str, std::ostream& os) {
  os << '"' << str << '"';
}

static void stringify_value(const std::any&, std::ostream&);

static void stringify_array(const json::Array& arr, std::ostream& os) {
  os << '[';

  json::Array::const_iterator cit = arr.cbegin();

  while (cit != arr.cend()) {
	stringify_value(*cit, os);

	++cit;

	if (cit != arr.cend()) {
	  os << ',';
	}
  }

  os << ']';
}

static void stringify_object(const json::Object& obj, std::ostream& os) {
  os << '{';

  json::Object::const_iterator cit = obj.cbegin();

  while (cit != obj.cend()) {
	os << '"' << cit->first << '"';
	os << ":";

	stringify_value(cit->second, os);

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

static inline void _stringify_object(const std::any& in, std::ostream& os) {
  stringify_object(std::any_cast<json::Object const&>(in), os);
}

static inline void _stringify_array(const std::any& in, std::ostream& os) {
  stringify_array(std::any_cast<json::Array const&>(in), os);
}

static inline void _stringify_string(const std::any& in, std::ostream& os) {
  stringify_string(std::any_cast<json::String const&>(in), os);
}

static inline void _stringify_number(const std::any& in, std::ostream& os) {
  stringify_number(std::any_cast<json::Number const&>(in), os);
}

static inline void _stringify_boolean(const std::any& in, std::ostream& os) {
  stringify_boolean(std::any_cast<json::Boolean const&>(in), os);
}

static inline void _stringify_null(const std::any& in, std::ostream& os) {
  stringify_null(std::any_cast<json::Null const&>(in), os);
}

static void stringify_value(const std::any& in, std::ostream& os) {
  using MapKeyType = std::type_index;
  using MapValueType = std::function<void(std::any const&, std::ostream&)>;

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
	map.find( std::type_index(in.type()) );

  if (cit != map.cend()) {
	((*cit).second)(in, os);

  } else {
	std::ostringstream oss;
	oss << "stringify_value() failure: unknown type index [" << in.type().name() << "]";
	throw oss;
  }
}

namespace json {
int stringify(const std::any& in, std::ostream& os)
{
  int retcode = 0;
  std::ios::iostate oldState = os.exceptions();

  os.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);

  try {
	stringify_value(in, os);

  } catch (const std::istream::failure& err) {
	std::cerr << err.what() << std::endl;
	retcode = 1;
  } catch (const std::ostringstream& oss) {
	std::cerr << oss.str() << std::endl;
	retcode = 1;
  }

  os.exceptions(oldState);
  return retcode;
}
}

