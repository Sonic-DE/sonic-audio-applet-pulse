# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_test(
    percentvalidatortest.cpp
    TEST_NAME percentvalidatortest
    LINK_LIBRARIES Qt6::Test Qt6::Gui plasma-volume
)

ecm_add_test(
    microphonelifecycletest.cpp
    ../src/microphonetest.cpp
    TEST_NAME microphonelifecycletest
    LINK_LIBRARIES Qt6::Test KF6::PulseAudioQt KF6::I18n PkgConfig::LIBPULSE
)
target_include_directories(microphonelifecycletest PRIVATE ../src)
