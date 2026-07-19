cmake_minimum_required(VERSION 3.25)

project(
    MetaDrop
    VERSION 0.1.5
    DESCRIPTION "A native Linux metadata inspector and sanitizer"
    HOMEPAGE_URL "https://github.com/Trendorin/MetaDrop"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

option(METADROP_BUILD_TESTS "Build the MetaDrop test suite" ${BUILD_TESTING})
option(METADROP_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Concurrent Network Svg LinguistTools)
find_package(PkgConfig REQUIRED)

pkg_check_modules(EXIV2 REQUIRED IMPORTED_TARGET exiv2)
pkg_check_modules(TAGLIB REQUIRED IMPORTED_TARGET taglib)
pkg_check_modules(QPDF REQUIRED IMPORTED_TARGET libqpdf)
pkg_check_modules(LIBARCHIVE REQUIRED IMPORTED_TARGET libarchive)

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

add_library(metadrop_core STATIC
    src/core/AppSettings.cpp
    src/core/AppSettings.h
    src/core/EngineRegistry.cpp
    src/core/EngineRegistry.h
    src/core/FileGuard.cpp
    src/core/FileGuard.h
    src/core/MetadataEngine.h
    src/core/MetadataService.cpp
    src/core/MetadataService.h
    src/core/MetadataTypes.cpp
    src/core/MetadataTypes.h
    src/core/OfficeXmlSanitizer.cpp
    src/core/OfficeXmlSanitizer.h
    src/core/OutputPlanner.cpp
    src/core/OutputPlanner.h
    src/core/RiskClassifier.cpp
    src/core/RiskClassifier.h
    src/core/engines/AudioEngine.cpp
    src/core/engines/AudioEngine.h
    src/core/engines/DocumentEngine.cpp
    src/core/engines/DocumentEngine.h
    src/core/engines/ImageEngine.cpp
    src/core/engines/ImageEngine.h
    src/core/engines/PdfEngine.cpp
    src/core/engines/PdfEngine.h
)

target_include_directories(metadrop_core PUBLIC src)
target_compile_definitions(metadrop_core PRIVATE POINTERHOLDER_TRANSITION=0)
target_link_libraries(metadrop_core
    PUBLIC
        Qt6::Core
    PRIVATE
        PkgConfig::EXIV2
        PkgConfig::TAGLIB
        PkgConfig::QPDF
        PkgConfig::LIBARCHIVE
)
metadrop_set_project_warnings(metadrop_core)
metadrop_enable_sanitizers(metadrop_core)

qt_add_executable(metadrop
    src/app/main.cpp
    src/app/SingleInstance.cpp
    src/app/SingleInstance.h
    src/ui/AboutDialog.cpp
    src/ui/AboutDialog.h
    src/ui/DropArea.cpp
    src/ui/DropArea.h
    src/ui/FileTableModel.cpp
    src/ui/FileTableModel.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/MetadataTreeModel.cpp
    src/ui/MetadataTreeModel.h
    src/ui/SettingsDialog.cpp
    src/ui/SettingsDialog.h
    resources/metadrop.qrc
)

target_compile_definitions(metadrop PRIVATE
    METADROP_VERSION="${PROJECT_VERSION}"
    METADROP_APP_ID="io.github.trendorin.MetaDrop"
)
target_link_libraries(metadrop PRIVATE
    metadrop_core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Network
    Qt6::Svg
)
metadrop_set_project_warnings(metadrop)
metadrop_enable_sanitizers(metadrop)

qt_add_translations(metadrop
    TS_FILES translations/metadrop_ru.ts
    RESOURCE_PREFIX "/i18n"
)

install(TARGETS metadrop
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES data/io.github.trendorin.MetaDrop.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(FILES data/io.github.trendorin.MetaDrop.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)
install(FILES data/icons/hicolor/512x512/apps/io.github.trendorin.MetaDrop.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
)
install(FILES
    README.md
    README.ru.md
    README.uk.md
    README.de.md
    CHANGELOG.md
    CONTRIBUTING.md
    CONTRIBUTORS.md
    RELEASE_NOTES.md
    LICENSE
    SECURITY.md
    SUPPORT.md
    THIRD_PARTY_NOTICES.md
    docs/PRIVACY.md
    docs/SECURITY_MODEL.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/metadrop
)
install(FILES packaging/debian/copyright
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/metadrop
)

if(METADROP_BUILD_TESTS)
    find_package(Qt6 6.4 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_NAME "MetaDrop")
set(CPACK_PACKAGE_VENDOR "Trendorin")
set(CPACK_PACKAGE_CONTACT "Trendorin <193045225+Trendorin@users.noreply.github.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Native Linux metadata inspector and sanitizer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "MetaDrop-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "TGZ;DEB;RPM")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6svg6 (>= 6.4)")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_RPM_PACKAGE_NAME "metadrop")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_AUTOREQ ON)
include(CPack)
