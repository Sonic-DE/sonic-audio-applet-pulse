include_directories (${CMAKE_CURRENT_BINARY_DIR}/..)

kcoreaddons_add_plugin(audioshortcutsservice
    SOURCES
        audioshortcutsservice.cpp
        audioshortcutsservice.h
    INSTALL_NAMESPACE
        "kf6/kded")

target_link_libraries(audioshortcutsservice
                        plasma-volume
                        KF6::CoreAddons
                        KF6::GlobalAccel
                        KF6::I18n
                        KF6::ConfigCore
                        KF6::ConfigGui
                        KF6::DBusAddons
                        PkgConfig::LIBPULSE
                        PkgConfig::LIBPULSE_MAINLOOP)