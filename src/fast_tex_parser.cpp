#include "fast_tex_parser.h"

ParseInfo::ParseInfo() {
	push_text_delim();
}

void ParseInfo::push_text_delim() {
	curr_text_item = std::make_shared<ParseText>(i, line);
}

void ParseInfo::_push_element(std::shared_ptr<TexElement> element) {
	if (!curr_items.empty()) {
		curr_items.top()->children.push_back(element);
	} else {
		parsed_elements.push_back(element);
	}
}

void ParseInfo::push_text_element(uint32_t i, uint16_t line) {
	if (!curr_text_item->text.empty()) {
		std::shared_ptr<TexElement> element = curr_text_item->build_element(i, line, "", *this);
		_push_element(element);
	}
}

void ParseInfo::push_text_element() {
	push_text_element(i, line);
}

void ParseInfo::push_delim(std::shared_ptr<ParseItem> parse_item) {
	push_text_element();
	curr_items.push(parse_item);
	push_text_delim();
}

void ParseInfo::push_element(EndDelimiterData end_delimiter) {
	push_text_element();
	std::shared_ptr<ParseItem> start = curr_items.top();
	curr_items.pop();
	std::shared_ptr<TexElement> element = start
			->build_element(i - (end_delimiter.handles_char ? 0 : 1), line,
					end_delimiter.end_delimiter, *this);
	_push_element(element);
	push_text_delim();
	if (typeid(*element) == typeid(TexCommand)) {
		if (!element->children.empty() && typeid(*element->last_child()) == typeid(TexText)) {
			curr_text_item->text += std::dynamic_pointer_cast<TexText>(element->last_child())->text;
			element->children.attr("pop")();
			element->_string = element->_string
					->substr(0, element->_string.value().size() - curr_text_item->text.size());
			element->_inner_string = element->_string->substr(element->start_delimiter.size(),
					element->_string->size() - element->start_delimiter.size() -
							element->end_delimiter.size());
		}
	}
}

void process_char(ParseInfo &p, char c) {
	p.c = c;
	p.contents += c;

	bool char_handled = false;
	if (!p.curr_items.empty()) {
		std::shared_ptr<ParseItem> start = p.curr_items.top();
		if (!start->delim_done) {
			if (!start->check_start_delim_done(c) && c != EOF) {
				start->start_delimiter += c;
				char_handled = true;
			}
		}
	}

	if (!char_handled && (p.contents.size() < 2 || p.contents.at(p.contents.size() - 2) != '\\')) {
		switch (c) {
			case '\\':
				char_handled = true;
				if (!p.curr_items.empty() && typeid(*p.curr_items.top()) == typeid(ParseCommand)) {
					EndDelimiterData end_delimiter = p.curr_items.top()->get_end_delimiter(p);
					p.push_element(end_delimiter);
				}
				p.push_delim(std::make_shared<ParseCommand>(p.i, p.line, std::string(1, c)));
				break;
			case '{':
			case '[':
				if (p.curr_items.empty() || (typeid(*p.curr_items.top()) != typeid(ParseComment) &&
						typeid(*p.curr_items.top()) != typeid(ParseCommand)))
					throw std::runtime_error("found argument start without command on line " +
							std::to_string(p.line));
				if (typeid(*p.curr_items.top()) == typeid(ParseCommand)) {
					p.push_delim(std::make_shared<ParseArg>(p.i, p.line, std::string(1, c)));
					char_handled = true;
				}
				break;
			case '%':
				p.push_delim(std::make_shared<ParseComment>(p.i, p.line, std::string(1, c)));
				char_handled = true;
				break;
		}
	}

	while (!char_handled && !p.curr_items.empty() && p.curr_items.top()->delim_done) {
		std::shared_ptr<ParseItem> start = p.curr_items.top();
		EndDelimiterData end_delimiter = start->get_end_delimiter(p);
		if (end_delimiter.is_end) {
			if ((typeid(*start) == typeid(ParseCommand)) && (start->start_delimiter == "\\begin")) {
				std::shared_ptr<TexCommand> command = std::dynamic_pointer_cast<TexCommand>(
						start->build_element(p.i - 1, p.line, end_delimiter.end_delimiter, p));
				p.curr_items.pop();
				p.push_delim(std::make_shared<ParseEnv>(p.i, p.line, command));
			} else
				p.push_element(end_delimiter);
			char_handled = end_delimiter.handles_char;
		} else
			break;
	}

	if (!char_handled && c != EOF) {
		p.curr_text_item->text += c;
	}

	p.i++;
	if (c == '\n')
		p.line++;
}

TexRoot handle_file_end(ParseInfo &p) {
	p.push_text_element();

	if (!p.curr_items.empty()) {
		std::cout << "remaining: " << p.curr_items.size() << "\n";
		std::cout << "top: " << p.curr_items.top()->build_element(p.i, p.line, "", p)
				->repr() << "\n";
	}

	return TexRoot(p.i, p.line, p.contents, p.parsed_elements);
}

TexRoot parse_file(std::string filename) {
	ParseInfo p;
	std::ifstream file(filename);

	if (file.is_open()) {
		while (file.good()) {
			process_char(p, file.get());
		}
	}

	return handle_file_end(p);
}

TexRoot parse(std::string string) {
	ParseInfo p;
	string += EOF;

	for (char &c: string)
		process_char(p, c);

	return handle_file_end(p);
}

PYBIND11_MODULE(fast_tex_parser, m) {
	m.doc() = "Fast TeX parser";

	m.def("parse", &parse);
	m.def("parse_file", &parse_file);

	py::class_<TexElement, std::shared_ptr<TexElement>> tex_element(m, "TexElement");
	tex_element.def("__repr__", &TexElement::__repr__).def("__str__", &TexElement::string)
			.def("find_command", py::overload_cast<std::string>(&TexElement::find_command))
			.def("find_command",
					py::overload_cast<std::vector<std::string>>(&TexElement::find_command))
			.def("find_commands", &TexElement::find_commands).def("find_env", &TexElement::find_env)
			.def("find_envs", &TexElement::find_envs)
			.def_readwrite("children", &TexElement::children)
			.def_property_readonly("string", &TexElement::inner_string)
			.def_property_readonly("outer_string", &TexElement::string)
			.def_readonly("start_line", &TexElement::start_line)
			.def_readonly("end_line", &TexElement::end_line)
			.def_readonly("start_pos", &TexElement::start_pos)
			.def_readonly("end_pos", &TexElement::end_pos);

	py::class_<TexCommand, std::shared_ptr<TexCommand>>(m, "TexCommand", tex_element)
			.def(py::init<const std::string &, const py::list &>(), py::arg("name"),
					py::arg("args") = py::list())
			.def_property("name", &TexCommand::get_name, &TexCommand::set_name)
			.def_readwrite("args", &TexCommand::args);

	py::class_<TexArg, std::shared_ptr<TexArg>>(m, "TexArg", tex_element)
			.def(py::init<const std::string &, const std::string &, const py::list &>(),
					py::arg("start_delim") = "{", py::arg("end_delim") = "}",
					py::arg("children") = py::list())
			.def_readwrite("start_delim", &TexArg::start_delimiter)
			.def_readwrite("end_delim", &TexArg::end_delimiter);

	py::class_<TexEnv, std::shared_ptr<TexEnv>>(m, "TexEnv", tex_element)
			.def(py::init<const std::string &, const py::list &>(), py::arg("name"),
					py::arg("children") = py::list())
			.def_property("name", &TexEnv::get_name, &TexEnv::set_name);

	py::class_<TexComment, std::shared_ptr<TexComment>>(m, "TexComment", tex_element)
			.def(py::init<const std::string &>(), py::arg("text"))
			.def_readwrite("text", &TexComment::text)
			.def_readwrite("end_delim", &TexArg::end_delimiter);;

	py::class_<TexText, std::shared_ptr<TexText>>(m, "TexText", tex_element)
			.def(py::init<const std::string &>(), py::arg("text"))
			.def_readwrite("text", &TexText::text);

	py::class_<TexRoot, std::shared_ptr<TexRoot>>(m, "TexRoot", tex_element)
			.def(py::init<const py::list &>(), py::arg("children") = py::list())
			.def_readonly("length", &TexRoot::length).def_readonly("lines", &TexRoot::lines);
}
