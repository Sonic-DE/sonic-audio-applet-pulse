#!/usr/bin/env python3
"""
SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
SPDX-License-Identifier: GPL-2.0-or-later
"""

import os
import shutil
from typing import Final

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

assert "CMAKE_CURRENT_BINARY_DIR" in os.environ
CMAKE_CURRENT_BINARY_DIR: Final = os.environ["CMAKE_CURRENT_BINARY_DIR"]


class CMakeExtension(Extension):

    def __init__(self, name, cmake_lists_dir='.', **kwa) -> None:
        Extension.__init__(self, name, sources=[], **kwa)
        self.cmake_lists_dir = os.path.abspath(cmake_lists_dir)


class cmake_build_ext(build_ext):

    def build_extensions(self) -> None:
        for ext in self.extensions:
            extdir: str = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
            if not os.path.exists(extdir):
                os.makedirs(extdir)
            shutil.copy(f"{CMAKE_CURRENT_BINARY_DIR}/{ext.name}.so", f"{extdir}/{ext.name}.so")


setup(
    name="inputsynth",
    version="0.1",
    ext_modules=[CMakeExtension("inputsynth")],
    cmdclass={'build_ext': cmake_build_ext},
)
