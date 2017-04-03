# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2017 Chris Gregory czipperz@gmail.com

include_directories(${CCOMP_SOURCE_DIR}/src)
link_directories(${CCOMP_SOURCE_DIR}/src)

file(GLOB files "*.h" "*.c")
add_executable(ccomp ${files})
add_executable(tests ${files})

target_compile_definitions(tests PRIVATE TEST_MODE=1)
add_definitions("-Werror=incompatible-pointer-types")
add_definitions("-Werror=implicit-function-declaration")
add_definitions("-Wall")
add_definitions("-Wextra")
add_definitions("-Wpedantic")
add_definitions("-Werror")
add_definitions("-Wno-error=unused-parameter")
add_definitions("-ansi")
add_definitions("-Wno-unused-function")
add_definitions("-Wno-unused-variable")
