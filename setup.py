import sys
from glob import glob
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

__version__ = "0.0.2"

ext_modules = [
    Pybind11Extension(
        "fast_tex_parser",
        sorted(glob("src/*.cpp")),
        include_dirs=["include"],
    ),
]

# noinspection PyTypeChecker
setup(
    name="fast_tex_parser",
    version=__version__,
    author="Elonia Moonbeam",
    author_email="zarainia@gmail.com",
    url="https://github.com/pybind/python_example",
    description="A test project using pybind11",
    long_description="",
    ext_modules=ext_modules,
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.6",
    data_files=[
        (
            'shared/typehints/python{}.{}/foo/bar'.format(*sys.version_info[:2]),
            ["__init__.pyi"]
        )
    ]
)
