#ifndef FAST_TEX_PARSER_FAST_TEX_PARSER_H
#define FAST_TEX_PARSER_FAST_TEX_PARSER_H

#include <fstream>
#include <string>
#include <stack>
#include <iostream>
#include <sstream>

#define PY_SSIZE_T_CLEAN

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include "tex_element.h"

class ParseItem;

class ParseText;

struct EndDelimiterData;

class ParseInfo {
public:
	char c;
	std::string contents;

	uint32_t i = 0;
	uint16_t line = 0;

	std::string delimiter = "";

	std::stack<std::shared_ptr<ParseItem>> curr_items;
	std::vector<std::shared_ptr<TexElement>> parsed_elements;

	std::shared_ptr<ParseText> curr_text_item;

	ParseInfo();

	void push_text_delim();

	void push_text_element(uint32_t i, uint16_t line);

	void push_text_element();

	void push_delim(std::shared_ptr<ParseItem> parse_item);

	void push_element(EndDelimiterData end_delimiter);

	void _print_debug();

	inline std::string get_contents_between(uint32_t start, uint32_t end) const {
		return contents.substr(start, end + 1 - start);
	}

private:
	void _push_element(std::shared_ptr<TexElement>);
};

TexRoot parse_file(std::string filename);

TexRoot parse(std::string string);

#include "parse_item.h"

#endif //FAST_TEX_PARSER_FAST_TEX_PARSER_H
