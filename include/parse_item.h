#ifndef FAST_TEX_PARSER_PARSE_ITEM_H
#define FAST_TEX_PARSER_PARSE_ITEM_H

#include <string>
#include <vector>
#include <map>

#include "tex_element.h"
#include "fast_tex_parser.h"

struct EndDelimiterData {
	std::string end_delimiter;
	bool is_end;
	bool handles_char = true;
};

class ParseItem {
public:
	uint32_t start_pos;
	uint16_t start_line;
	std::string start_delimiter;
	std::vector<std::shared_ptr<TexElement>> children;
	bool delim_done;

	ParseItem(uint32_t pos, uint16_t line, std::string delimiter);

	virtual EndDelimiterData get_end_delimiter(const ParseInfo &p) {
		throw new std::exception();
	}

	virtual std::shared_ptr<TexElement>
	build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
			const ParseInfo &p) {
		throw new std::exception();
	};

	virtual bool check_start_delim_done(char c);
};

class ParseCommand : public ParseItem {
public:
	using ParseItem::ParseItem;

	EndDelimiterData get_end_delimiter(const ParseInfo &p) override;

	std::shared_ptr<TexElement>
	build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
			const ParseInfo &p) override;

	bool check_start_delim_done(char c) override;
};

class ParseArg : public ParseItem {
public:
	using ParseItem::ParseItem;

	EndDelimiterData get_end_delimiter(const ParseInfo &p) override;

	std::shared_ptr<TexElement>
	build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
			const ParseInfo &p) override;
};

class ParseEnv : public ParseItem {
public:
	std::string name;

	ParseEnv(uint32_t pos, uint16_t line, std::shared_ptr<TexCommand> start_command);

	EndDelimiterData get_end_delimiter(const ParseInfo &p) override;

	std::shared_ptr<TexElement>
	build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
			const ParseInfo &p) override;
};

class ParseComment : public ParseItem {
public:
	std::string text;

	using ParseItem::ParseItem;

	EndDelimiterData get_end_delimiter(const ParseInfo &p) override;

	std::shared_ptr<TexElement>
	build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
			const ParseInfo &p) override;
};

class ParseText : public ParseItem {
public:
	std::string text;

	ParseText(uint32_t pos, uint16_t line);

	EndDelimiterData get_end_delimiter(const ParseInfo &p) override;

	std::shared_ptr<TexElement>
	build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
			const ParseInfo &p) override;
};

#endif //FAST_TEX_PARSER_PARSE_ITEM_H
