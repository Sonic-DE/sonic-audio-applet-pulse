ecm_generate_export_header(plasma-volume-declarative
    BASE_NAME PipeWireQt
    VERSION ${PROJECT_VERSION}
)

pkg_check_modules(libpipewire libpipewire-0.3 REQUIRED IMPORTED_TARGET GLOBAL)
pkg_check_modules(libspa libspa-0.2 REQUIRED IMPORTED_TARGET GLOBAL)

target_include_directories(plasma-volume-declarative PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_sources(plasma-volume-declarative PRIVATE context.cpp node.cpp initializer.cpp threadloop.cpp core.cpp)
target_link_libraries(plasma-volume-declarative PRIVATE PkgConfig::libpipewire PkgConfig::libspa)
