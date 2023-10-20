# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

if(NOT BUILD_TESTING OR NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
    return()
endif()

add_test(
    NAME inputsynth_build
    COMMAND python3 setup.py build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/inputsynth
)
add_test(
    NAME inputsynth_install
    COMMAND python3 setup.py install --user
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/inputsynth
)
set_tests_properties(inputsynth_build inputsynth_install PROPERTIES
    TIMEOUT 300
    ENVIRONMENT "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX};CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
)

add_test(
    NAME applettest
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/applettest.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=libasan.so OUTPUT_VARIABLE LIBASAN_PATH ECHO_OUTPUT_VARIABLE OUTPUT_STRIP_TRAILING_WHITESPACE)
set_tests_properties(applettest PROPERTIES TIMEOUT 300 ENVIRONMENT "LD_PRELOAD=${LIBASAN_PATH}")
