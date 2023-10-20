# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

find_package(Qt6 REQUIRED COMPONENTS WaylandClient)
find_package(Wayland REQUIRED COMPONENTS Client)
find_package(PlasmaWaylandProtocols 1.6 REQUIRED)
find_package(Threads REQUIRED)
find_package(Python 3.11 COMPONENTS Interpreter Development REQUIRED)

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
        Python::Python
)
if (DEFINED ENV{KDECI_BUILD})
target_link_libraries(inputsynth PRIVATE gcov)
endif()

add_custom_command(TARGET inputsynth
    POST_BUILD
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py install --user
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
