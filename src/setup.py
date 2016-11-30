#! -*- coding: utf-8 -*-

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
import numpy

ext_modules = [
        Extension("chainer_tagger",
                  ["py/tagger.pyx"],
                  include_dirs=[numpy.get_include()],
                  extra_compile_args=["-O3"]
                  ),
        Extension("py.py_utils",
                  ["py/py_utils.pyx"],
                  include_dirs=[numpy.get_include()],
                  extra_compile_args=["-O3", "-ffast-math", "-fPIC"]
                  ),
        ]

setup(
        name = "chainer_tagger",
        cmdclass = { "build_ext" : build_ext },
        ext_modules = ext_modules,
)