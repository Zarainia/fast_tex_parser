#include "parse_item.h"

static const std::map<char, char> DELIMITER_MAP = {{'{', '}'},
												   {'[', ']'}};

inline bool ends_with(std::string const &value, std::string const &ending) {
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

ParseItem::ParseItem(uint32_t pos, uint16_t line, std::string delimiter) {
	start_pos = pos;
	start_line = line;
	start_delimiter = delimiter;
	delim_done = false;
}

bool ParseItem::check_start_delim_done(char c) {
	delim_done = true;
	return delim_done;
}

EndDelimiterData ParseCommand::get_end_delimiter(const ParseInfo &p) {
	if (p.c != ' ' && p.c != '[' && p.c != '{')
		return EndDelimiterData{.end_delimiter = "", .is_end = true, .handles_char = false};
	return EndDelimiterData{.is_end = false};
}

std::shared_ptr<TexElement>
ParseCommand::build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
		const ParseInfo &p) {
	std::string name;
	if (!start_delimiter.empty())
		name = start_delimiter.substr(1);

	return std::make_shared<TexCommand>(start_pos, start_line, end_pos, end_line, start_delimiter,
			end_delimiter, p.get_contents_between(start_pos, end_pos), children, name);
}

bool ParseCommand::check_start_delim_done(char c) {
	delim_done = !std::isalnum(c);
	return delim_done;
}

EndDelimiterData ParseArg::get_end_delimiter(const ParseInfo &p) {
	char end_delimiter = DELIMITER_MAP.at(start_delimiter.back());
	if (!p.contents.empty() && p.c == end_delimiter)
		return EndDelimiterData{.end_delimiter = {end_delimiter}, .is_end = true};
	return EndDelimiterData{.is_end = false};
}

std::shared_ptr<TexElement>
ParseArg::build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
		const ParseInfo &p) {
	return std::make_shared<TexArg>(start_pos, start_line, end_pos, end_line, start_delimiter,
			end_delimiter, p.get_contents_between(start_pos, end_pos), children);
}

ParseEnv::ParseEnv(uint32_t pos, uint16_t line, std::shared_ptr<TexCommand> start_command)
		: ParseItem(pos, line, start_command->string()) {
	if (start_command->args.empty() || typeid(*start_command->last_arg()) != typeid(TexArg))
		throw std::runtime_error(
				"wrong argument for ParseEnv start TextCommand: " + start_command->repr());
	std::shared_ptr<TexArg> arg = start_command->last_arg();
	if (arg->children.empty() || typeid(*arg->last_child()) != typeid(TexText))
		throw std::runtime_error("wrong text for ParseEnv start TextCommand: " +
				std::string(typeid(*arg->last_child()).name()));
	name = std::dynamic_pointer_cast<TexText>(start_command->last_arg()->last_child())->text;
}

EndDelimiterData ParseEnv::get_end_delimiter(const ParseInfo &p) {
	if (!children.empty() && typeid(*children.back()) == typeid(TexCommand)) {
//		std::cout << "FOUND COMMAND\n";
		std::shared_ptr<TexCommand> end_command = std::dynamic_pointer_cast<TexCommand>(
				children.back());
		if (end_command->name == "end" && !end_command->args.empty() &&
				typeid(*end_command->last_arg()) == typeid(TexArg)) {
			std::shared_ptr<TexArg> first_arg = std::dynamic_pointer_cast<TexArg>(
					end_command->last_arg());
			if (!first_arg->children.empty() &&
					typeid(*first_arg->last_child()) == typeid(TexText) &&
					std::dynamic_pointer_cast<TexText>(first_arg->last_child())->text == name) {
				std::string end_delimiter = children.back()->string();
				children.pop_back();
				return EndDelimiterData{.end_delimiter = end_delimiter, .is_end = true, .handles_char = false};
			}
		}
	}
	return EndDelimiterData{.is_end = false};
}

std::shared_ptr<TexElement>
ParseEnv::build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
		const ParseInfo &p) {
	return std::make_shared<TexEnv>(start_pos, start_line, end_pos, end_line, start_delimiter,
			end_delimiter, p.get_contents_between(start_pos, end_pos), children, name);
}

EndDelimiterData ParseComment::get_end_delimiter(const ParseInfo &p) {
	if (p.c == '\n')
		return EndDelimiterData{.end_delimiter = "\n", .is_end = true};
	return EndDelimiterData{.is_end = false};
}

std::shared_ptr<TexElement>
ParseComment::build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
		const ParseInfo &p) {
	return std::make_shared<TexComment>(start_pos, start_line, end_pos, end_line, start_delimiter,
			end_delimiter, p.get_contents_between(start_pos, end_pos));
}

ParseText::ParseText(uint32_t pos, uint16_t line) : ParseItem(pos, line, "") {
}

EndDelimiterData ParseText::get_end_delimiter(const ParseInfo &p) {
	return EndDelimiterData{.is_end = false};
}

std::shared_ptr<TexElement>
ParseText::build_element(uint32_t end_pos, uint16_t end_line, std::string end_delimiter,
		const ParseInfo &p) {
	return std::make_shared<TexText>(start_pos, start_line, end_pos, end_line, text);
}

