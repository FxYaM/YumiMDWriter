# SPDX-FileCopyrightText: 2026 XiangXiang
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(cmarkgfmapitest VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Test)

qt_standard_project_setup()

ecm_add_test(
    cmarkgfmapitest.cpp
    ../../src/export/exporter.cpp
    ../../src/export/exportformat.cpp
    ../../src/export/cmarkgfmexporter.cpp
    ../../src/markdown/cmarkgfmapi.cpp
    ../../src/markdown/markdownast.cpp
    ../../src/markdown/markdownnode.cpp
    TEST_NAME cmarkgfmapitest
    LINK_LIBRARIES
        PRIVATE
            Qt::Core
            Qt::Test
            libcmark-gfm
            libcmark-gfm-extensions
)

target_include_directories(cmarkgfmapitest PRIVATE
    "${PROJECT_SOURCE_DIR}/../.."
    "${PROJECT_SOURCE_DIR}/../../src"
    "${PROJECT_BINARY_DIR}/../.."
    "${PROJECT_BINARY_DIR}/../../3rdparty/cmark-gfm/src"
)

enable_testing(true)
