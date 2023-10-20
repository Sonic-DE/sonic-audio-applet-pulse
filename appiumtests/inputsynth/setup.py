#!/usr/bin/env python3
"""
SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
SPDX-License-Identifier: GPL-2.0-or-later
"""

import os
import subprocess
import sys
from typing import Final

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

assert "CMAKE_INSTALL_PREFIX" in os.environ
CMAKE_INSTALL_PREFIX: Final = os.environ["CMAKE_INSTALL_PREFIX"]


class CMakeExtension(Extension):

    def __init__(self, name, cmake_lists_dir='.', **kwa) -> None:
        Extension.__init__(self, name, sources=[], **kwa)
        self.cmake_lists_dir = os.path.abspath(cmake_lists_dir)


class cmake_build_ext(build_ext):

    def build_extensions(self) -> None:
        for ext in self.extensions:
            extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
            cmake_args = [
                "-DCMAKE_BUILD_TYPE=Debug",
                # Ask CMake to place the resulting library in the directory
                # containing the extension
                f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG={extdir}",
                # Other intermediate static libraries are placed in a
                # temporary build directory instead
                f"-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_BUILD={self.build_temp}",
                # Hint CMake to use the same Python executable that
                # is launching the build, prevents possible mismatching if
                # multiple versions of Python are installed
                f"-DPYTHON_EXECUTABLE={sys.executable}",
                f"-DCMAKE_INSTALL_PREFIX={CMAKE_INSTALL_PREFIX}",
            ]

            if not os.path.exists(self.build_temp):
                os.makedirs(self.build_temp)

            # Config
            subprocess.check_call(['cmake', ext.cmake_lists_dir] + cmake_args, cwd=self.build_temp)

            # Build
            subprocess.check_call(['cmake', '--build', '.', '--config', "Debug"], cwd=self.build_temp)


setup(
    name="inputsynth",
    version="0.1",
    ext_modules=[CMakeExtension("inputsynth")],
    cmdclass={'build_ext': cmake_build_ext},
)
