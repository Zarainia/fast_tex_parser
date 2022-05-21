#ifndef FAST_TEX_PARSER_TEX_ELEMENT_H
#define FAST_TEX_PARSER_TEX_ELEMENT_H

#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <optional>
#include <map>
#include <cassert>
#include <functional>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

class TexCommand;

class TexEnv;

class TexElement {
public:
	uint32_t start_pos;
	uint16_t start_line;
	uint32_t end_pos;
	uint16_t end_line;
	std::string start_delimiter;
	std::string end_delimiter;
	py::list children;
	std::optional<std::string> _string;
	std::optional<std::string> _inner_string;

	TexElement(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
			std::string start_delimiter, std::string end_delimiter, std::string contents,
			std::vector<std::shared_ptr<TexElement>> children);

	TexElement(std::optional<std::string> contents, py::list children = py::list());

	std::string __repr__();

	virtual std::string repr(uint8_t indent_level = 0);

	virtual std::string string();

	virtual std::string inner_string();

	std::optional<std::shared_ptr<TexCommand>> find_command(std::string name);

	std::optional<std::shared_ptr<TexCommand>> find_command(std::vector<std::string> name);

	std::vector<std::shared_ptr<TexCommand>> find_commands(std::string name);

	std::optional<std::shared_ptr<TexEnv>> find_env(std::string name);

	std::vector<std::shared_ptr<TexEnv>> find_envs(std::string name);

	inline std::shared_ptr<TexElement> last_child() {
		if (children.empty())
			throw std::runtime_error("tried to access last child of empty element");
		return py::cast<std::shared_ptr<TexElement>>(children[children.size() - 1]);
	}

protected:
	std::string get_children_repr(uint8_t indent_level = 0);

	std::string get_children_string();

	std::string _default_repr(std::string type_name, uint8_t indent_level = 0,
			std::optional<std::string> _start_delimiter = {},
			std::optional<std::string> _end_delimiter = {});

	template<typename T>
	std::optional<std::shared_ptr<T>> _find_element(std::function<bool(std::shared_ptr<T>)> test);

	template<typename T>
	std::vector<std::shared_ptr<T>> _find_elements(std::function<bool(std::shared_ptr<T>)> test);

	inline std::string _get_indent(uint8_t indent_level) {
		if (indent_level <= 0)
			return "";
		return std::string(indent_level * 4, ' ');
	}
};

class TexArg : public TexElement {
public:
	TexArg(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
			std::string start_delimiter, std::string end_delimiter, std::string contents,
			std::vector<std::shared_ptr<TexElement>> children);

	TexArg(std::string start_delimiter = "{", std::string end_delimiter = "}",
			py::list children = py::list());

	std::string repr(uint8_t indent_level = 0) override;
};

class TexCommand : public TexElement {
public:
	std::string name;
	py::list args;

	TexCommand(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
			std::string start_delimiter, std::string end_delimiter, std::string contents,
			std::vector<std::shared_ptr<TexElement>> children, std::string name);

	TexCommand(std::string name, py::list args = py::list());

	std::string repr(uint8_t indent_level = 0) override;

	std::string inner_string() override;

	inline std::string get_name() {
		return name;
	}

	void set_name(std::string name);

	inline std::shared_ptr<TexArg> last_arg() {
		if (args.empty())
			throw std::runtime_error("tried to access last child of empty element");
		return py::cast<std::shared_ptr<TexArg>>(args[args.size() - 1]);
	}

private:
	std::string _args_string;

	bool _args_has_changes();

	bool _update_children();
};

class TexEnv : public TexElement {
public:
	std::string name;

	TexEnv(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
			std::string start_delimiter, std::string end_delimiter, std::string contents,
			std::vector<std::shared_ptr<TexElement>> children, std::string name);

	TexEnv(std::string name, py::list children = py::list());

	std::string repr(uint8_t indent_level = 0) override;

	inline std::string get_name() {
		return name;
	}

	void set_name(std::string name);
};

class TexComment : public TexElement {
public:
	std::string text;

	TexComment(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
			std::string start_delimiter, std::string end_delimiter, std::string contents);

	TexComment(std::string text);

	std::string repr(uint8_t indent_level = 0) override;

	std::string inner_string() override;
};

class TexText : public TexElement {
public:
	std::string text;

	TexText(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
			std::string text);

	TexText(std::string text);

	std::string repr(uint8_t indent_level = 0) override;


	std::string inner_string() override;
};

class TexRoot : public TexElement {
public:
	uint32_t length;
	uint16_t lines;

	TexRoot(uint32_t length, uint16_t lines, std::string contents,
			std::vector<std::shared_ptr<TexElement>> children);

	TexRoot(py::list children = py::list());

	std::string repr(uint8_t indent_level = 0) override;
};

#endif //FAST_TEX_PARSER_TEX_ELEMENT_H
