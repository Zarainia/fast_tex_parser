#include "tex_element.h"

const std::map<std::string, std::string> repr_replacements = {{"\n", "\\n"},
															  {"\t", "\\t"}};

inline std::string reprfy_string(std::string string) {
	for (auto const &[key, replacement]: repr_replacements)
		string = std::regex_replace(string, std::regex(key), replacement);
	return string;
}

TexElement::TexElement(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
		std::string start_delimiter, std::string end_delimiter, std::string contents,
		std::vector<std::shared_ptr<TexElement>> children) {
	this->start_pos = start_pos;
	this->start_line = start_line;
	this->end_pos = end_pos;
	this->end_line = end_line;
	this->start_delimiter = start_delimiter;
	this->end_delimiter = end_delimiter;
	this->children = py::cast(children);
	this->_string = contents;
	this->_inner_string = contents.substr(start_delimiter.size(),
			contents.size() - start_delimiter.size() - end_delimiter.size());
}

TexElement::TexElement(std::optional<std::string> contents, py::list children) {
	this->start_pos = -1;
	this->start_line = -1;
	this->end_pos = -1;
	this->end_line = -1;
	this->children = children;
	this->_string = contents;
	if (contents.has_value() && contents->size() >= start_delimiter.size() + end_delimiter.size())
		this->_inner_string = contents->substr(start_delimiter.size(),
				contents->size() - start_delimiter.size() - end_delimiter.size());
}

std::string TexElement::get_children_repr(uint8_t indent_level) {
	std::string r;
	for (py::handle child: children)
		r += _get_indent(indent_level) +
				py::cast<std::shared_ptr<TexElement>>(child)->repr(indent_level) + ",\n";
	if (r.empty())
		return r;
	return "\n" + r + _get_indent(indent_level - 1);
}

std::string TexElement::_default_repr(std::string type_name, uint8_t indent_level,
		std::optional<std::string> _start_delimiter, std::optional<std::string> _end_delimiter) {
	return type_name + "(" + _start_delimiter.value_or(start_delimiter) +
			get_children_repr(indent_level + 1) + _end_delimiter.value_or(end_delimiter) + ")";
}

std::string TexElement::repr(uint8_t indent_level) {
	return _default_repr("TexElement", indent_level);
}

std::string TexElement::__repr__() {
	return repr();
}

std::string TexElement::get_children_string() {
	std::string str;
	for (py::handle child: children) str += py::cast<std::shared_ptr<TexElement>>(child)->string();
	return str;
}

std::string TexElement::inner_string() {
	return get_children_string();
}

std::string TexElement::string() {
	return start_delimiter + inner_string() + end_delimiter;
}

template<typename T>
std::optional<std::shared_ptr<T>>
TexElement::_find_element(std::function<bool(std::shared_ptr<T>)> test) {
	for (py::handle handle: children) {
		std::shared_ptr<TexElement> child = py::cast<std::shared_ptr<TexElement>>(handle);
		if (typeid(*child) == typeid(T) && test(std::dynamic_pointer_cast<T>(child))) {
			return std::dynamic_pointer_cast<T>(child);
		} else {
			std::optional<std::shared_ptr<T>> result = child->_find_element(test);
			if (result.has_value())
				return result;
		}
	}
	return std::optional<std::shared_ptr<T>>();
}

template<typename T>
std::vector<std::shared_ptr<T>>
TexElement::_find_elements(std::function<bool(std::shared_ptr<T>)> test) {
	std::vector<std::shared_ptr<T>> results;
	for (py::handle handle: children) {
		std::shared_ptr<TexElement> child = py::cast<std::shared_ptr<TexElement>>(handle);
		if (typeid(*child) == typeid(T) && test(std::dynamic_pointer_cast<T>(child))) {
			results.push_back(std::dynamic_pointer_cast<T>(child));
		} else {
			std::vector<std::shared_ptr<T>> child_results = child->_find_elements(test);
			if (!child_results.empty())
				results.insert(results.end(), child_results.begin(), child_results.end());
		}
	}
	return results;
}

std::optional<std::shared_ptr<TexCommand>> TexElement::find_command(std::string name) {
	return _find_element<TexCommand>([&](std::shared_ptr<TexCommand> command) {
		return command->name == name;
	});
}

std::optional<std::shared_ptr<TexCommand>> TexElement::find_command(std::vector<std::string> name) {
	return _find_element<TexCommand>([&](std::shared_ptr<TexCommand> command) {
		return std::find(name.begin(), name.end(), command->name) != name.end();
	});
}

std::vector<std::shared_ptr<TexCommand>> TexElement::find_commands(std::string name) {
	return _find_elements<TexCommand>([&](std::shared_ptr<TexCommand> command) {
		return command->name == name;
	});
}

std::optional<std::shared_ptr<TexEnv>> TexElement::find_env(std::string name) {
	return _find_element<TexEnv>([&](std::shared_ptr<TexEnv> env) {
		return env->name == name;
	});
}

std::vector<std::shared_ptr<TexEnv>> TexElement::find_envs(std::string name) {
	return _find_elements<TexEnv>([&](std::shared_ptr<TexEnv> env) {
		return env->name == name;
	});
}

TexCommand::TexCommand(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
		std::string start_delimiter, std::string end_delimiter, std::string contents,
		std::vector<std::shared_ptr<TexElement>> children, std::string name) : TexElement(start_pos,
		start_line, end_pos, end_line, start_delimiter, end_delimiter, contents, children) {
	this->name = name;

	for (std::shared_ptr<TexElement> child: children) {
		if (typeid(*child) == typeid(TexArg)) {
			assert(child->_string.has_value());
			_args_string += child->_string.value();
			args.append(child);
		}
	}
}

TexCommand::TexCommand(std::string name, py::list args) : TexElement({}, args) {
	this->name = name;
	start_delimiter = "\\" + name;
	this->args = args;
	_args_has_changes();
}


bool TexCommand::_args_has_changes() {
	std::string args_string;
	for (py::handle arg: args) args_string += py::cast<std::shared_ptr<TexArg>>(arg)->string();
	bool has_changes = args_string != _args_string;
//	if (has_changes) {
//		std::cout << "START " << _args_string << "\n";
//		std::cout << "THEN " << args_string << "\n";
//	}
	_args_string = args_string;
	return has_changes;
}

bool TexCommand::_update_children() {
	if (_args_has_changes()) {
		children = args;
		return true;
	}
	return false;
}

std::string TexCommand::inner_string() {
	if (_update_children()) {
		return _args_string;
	} else if (_inner_string.has_value())
		return _inner_string.value();
	return get_children_string();
}

std::string TexCommand::repr(uint8_t indent_level) {
	_update_children();
	return _default_repr("TexCommand", indent_level, name + ": ", "");
}

void TexCommand::set_name(std::string name) {
	this->name = name;
	start_delimiter = "\\" + name;
}

TexArg::TexArg(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
		std::string start_delimiter, std::string end_delimiter, std::string contents,
		std::vector<std::shared_ptr<TexElement>> children) : TexElement(start_pos, start_line,
		end_pos, end_line, start_delimiter, end_delimiter, contents, children) {
}

TexArg::TexArg(std::string start_delimiter, std::string end_delimiter, py::list children)
		: TexElement({}, children) {
	this->start_delimiter = start_delimiter;
	this->end_delimiter = end_delimiter;
}

std::string TexArg::repr(uint8_t indent_level) {
	return _default_repr("TexArg", indent_level);
}

TexEnv::TexEnv(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
		std::string start_delimiter, std::string end_delimiter, std::string contents,
		std::vector<std::shared_ptr<TexElement>> children, std::string name) : TexElement(start_pos,
		start_line, end_pos, end_line, start_delimiter, end_delimiter, contents, children) {
	this->name = name;
}

TexEnv::TexEnv(std::string name, py::list children) : TexElement({}, children) {
	this->name = name;
	start_delimiter = "\\begin{" + name + "}";
	end_delimiter = "\\end{" + name + "}";
}

std::string TexEnv::repr(uint8_t indent_level) {
	return _default_repr("TexEnv", indent_level, name + ": ", "");
}

void TexEnv::set_name(std::string name) {
	this->name = name;
	start_delimiter = "\\begin{" + name + "}";
	end_delimiter = "\\end{" + name + "}";
}

TexComment::TexComment(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
		std::string start_delimiter, std::string end_delimiter, std::string contents) : TexElement(
		start_pos, start_line, end_pos, end_line, start_delimiter, end_delimiter, contents, {}) {
	this->text = contents.substr(start_delimiter.size(),
			contents.size() - start_delimiter.size() - end_delimiter.size());
}

TexComment::TexComment(std::string text) : TexElement("%" + text + "\n") {
	this->text = text;
	start_delimiter = "%";
	end_delimiter = "\n";
}

std::string TexComment::repr(uint8_t indent_level) {
	return "TexComment(" + reprfy_string(text) + ")";
}

std::string TexComment::inner_string() {
	return text;
}

TexText::TexText(uint32_t start_pos, uint16_t start_line, uint32_t end_pos, uint16_t end_line,
		std::string text) : TexElement(start_pos, start_line, end_pos, end_line, "", "", text,
		std::vector<std::shared_ptr<TexElement>>()) {
	this->text = text;
}

TexText::TexText(std::string text) : TexElement(text) {
	this->text = text;
}

std::string TexText::repr(uint8_t indent_level) {
	return "t'" + reprfy_string(text) + "'";
}

std::string TexText::inner_string() {
	return text;
}

TexRoot::TexRoot(uint32_t length, uint16_t lines, std::string contents,
		std::vector<std::shared_ptr<TexElement>> children) : TexElement(0, 0, length, lines, "", "",
		contents, children) {
	this->length = length;
	this->lines = lines;
}

TexRoot::TexRoot(py::list children) : TexElement({}, children) {
}

std::string TexRoot::repr(uint8_t indent_level) {
	return _default_repr("TexRoot", indent_level);
}