# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(inputsynth)

set(QT_MIN_VERSION "6.5.0")
set(KF6_MIN_VERSION "5.240.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.240")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Gui
    WaylandClient
)
find_package(Wayland REQUIRED COMPONENTS Client)
find_package(PlasmaWaylandProtocols 1.6 REQUIRED)
find_package(Threads REQUIRED)
find_package(Python COMPONENTS Interpreter Development REQUIRED)

add_library(inputsynth SHARED inputsynth.cpp)
set_target_properties(inputsynth PROPERTIES
    PREFIX ""
    OUTPUT_NAME "inputsynth"
    LINKER_LANGUAGE C
    POSITION_INDEPENDENT_CODE ON
)

qt6_generate_wayland_protocol_client_sources(inputsynth FILES ${PLASMA_WAYLAND_PROTOCOLS_DIR}/fake-input.xml)

target_include_directories(inputsynth
    PRIVATE ${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(inputsynth
    PRIVATE
        Qt::Core
        Qt::Gui # QGuiApplication
        Qt::WaylandClient # Fake input protocol
        Threads::Threads
        Wayland::Client
        ${PYTHON_LIBRARY}
)
