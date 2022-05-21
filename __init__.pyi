import abc


class TexElement(abc.ABC):
    """
    Base class for all TeX elements
    """

    def find_command(self, name):
        """
        Find a command matching ``name`` in descendants

        :param name: command name
        :type name: str or list[str]
        :return: matching command
        :rtype: TexCommand or None
        """

    def find_commands(self, name):
        """
        Find all commands with name ``name`` in descendants

        :param str name: command name
        :return: matching commands
        :rtype: list[TexCommand]
        """

    def find_env(self, name):
        """
        Find an environment with name ``name`` in descendants

        :param str name: environment name
        :return: matching environment
        :rtype: TexEnv or None
        """

    def find_envs(self, name):
        """
        Find all environments with name ``name`` in descendants

        :param str name: environment name
        :return: matching environment
        :rtype: list[TexEnv]
        """

    @property
    def children(self):
        """
        Element's children

        :return: children
        :rtype: list[TexElement]
        """

    @children.setter
    def children(self, value):
        """
        Element's children

        :param value: children
        :type value: list[TexElement]
        """

    @property
    def string(self):
        """
        String inside element (not including element's delimiters)

        :return: inner string
        :rtype: str
        """

    @property
    def outer_string(self):
        """
        String of entire element (including element's delimiters)

        :return: outer string
        :rtype: str
        """

    @property
    def start_line(self):
        """
        Line element starts on

        :return: starting line
        :rtype: int
        """

    @property
    def end_line(self):
        """
        Line element ends on

        :return: ending line
        :rtype: int
        """

    @property
    def start_pos(self):
        """
        Character position element starts on

        :return: starting position
        :rtype: int
        """

    @property
    def end_pos(self):
        """
        Character position element ends on

        :return: ending position
        :rtype: int
        """


class TexCommand(TexElement):
    r"""
    A TeX command, for example
        \\emph{some text}
    """

    def __init__(self, name, args=[]):
        """
        :param str name: command name
        :param args: command arguments
        :type args: list[TexArg]
        """

    @property
    def name(self):
        """
        Command name (eg., ``emph``)

        :return: name
        :rtype: str
        """

    @name.setter
    def name(self, name):
        """
        Command name (eg., ``emph``)

        :param name: name
        :type name: str
        """

    @property
    def args(self):
        """
        Command arguments (eg. ``{some text}``)

        :return: arguments
        :rtype: list[TexArg]
        """

    @args.setter
    def args(self, arg):
        """
        Command arguments (eg. ``{some text}``)

        :param arg: arguments
        :type arg: list[TexArg]
        """


class TexArg(TexElement):
    """
    A TeX command argument, for example
        {some text}
    """

    def __init__(self, start_delim='{', end_delim='}', children=[]):
        """
        :param str start_delim: start delimiter
        :param str end_delim: start delimiter
        :param children: command arguments
        :type children: list[TexElement]
        """

    @property
    def start_delim(self):
        """
        Start delimiter (open bracket, eg. ``{``)

        :return: start delimiter
        :rtype: str
        """

    @start_delim.setter
    def start_delim(self, start_delim):
        """
        Start delimiter (open bracket, eg. ``{``)

        :param start_delim: start delimiter
        :type start_delim: str
        """

    @property
    def end_delim(self):
        """
        End delimiter (close bracket, eg. ``}``)

        :return: end delimiter
        :rtype: str
        """

    @end_delim.setter
    def end_delim(self, end_delim):
        """
        End delimiter (close bracket, eg. ``}``)

        :param end_delim: end delimiter
        :type end_delim: str
        """


class TexEnv(TexElement):
    r"""
    A TeX environment, for example
        \\begin{document}
            some text
        \\end{document}
    """

    def __init__(self, name, children=[]):
        """
        :param str name: environment name
        :param children: environment contents
        :type children: list[TexElement]
        """

    @property
    def name(self):
        """
        Environment name (eg., ``document``)

        :return: name
        :rtype: str
        """

    @name.setter
    def name(self, name):
        """
        Environment name (eg., ``document``)

        :param name: name
        :type name: str
        """


class TexComment(TexElement):
    """
    A TeX comment, for example
        % some text
    """

    def __init__(self, text):
        """
        :param str text: comment text
        """

    @property
    def text(self):
        """
        Comment text (eg., `` some text``)

        :return: text
        :rtype: str
        """

    @text.setter
    def text(self, text):
        """
        Comment text (eg., `` some text``)

        :param text: text
        :type text: str
        """

    @property
    def end_delim(self):
        """
        End delimiter (usually a newline)

        :return: end delimiter
        :rtype: str
        """

    @end_delim.setter
    def end_delim(self, end_delim):
        """
        End delimiter (usually a newline)

        :param end_delim: end delimiter
        :type end_delim: str
        """


class TexText(TexElement):
    """
    Some plain TeX text, for example
        some text
    """

    def __init__(self, text):
        """
        :param str text: text
        """

    @property
    def text(self):
        """
        Text

        :return: text
        :rtype: str
        """

    @text.setter
    def text(self, text):
        """
        Text

        :param text: text
        :type text: str
        """

    @property
    def end_delim(self):
        """
        End delimiter (usually a newline)

        :return: end delimiter
        :rtype: str
        """

    @end_delim.setter
    def end_delim(self, end_delim):
        """
        End delimiter (usually a newline)

        :param end_delim: end delimiter
        :type end_delim: str
        """


class TexRoot(TexElement):
    """
    Root of a TeX tree, containing all parsed elements
    """

    def __init__(self, children=[]):
        """
        :param children: TeX elements
        :type children: list[TexElement]
        """

    @property
    def length(self):
        """
        Total number of characters

        :return: characters
        :rtype: int
        """

    @property
    def lines(self):
        """
        Total number of lines

        :return: lines
        :rtype: int
        """


def parse(string):
    """
    Parse TeX from a string

    :param string: TeX string
    :return: TeX root
    :rtype: TexRoot
    """


def parse_file(path):
    """
    Parse TeX from a file

    :param path: path to file to parse
    :return: TeX root
    :rtype: TexRoot
    """
