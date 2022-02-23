cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/libraries/Arduino-CMake-Toolchain/Arduino-toolchain.cmake")

# See BoardOptions.cmake in the build directory for more options.
# Note however, that Keyboard and Mouse are currently only supported on 32u5 chips.
set(ARDUINO_BOARD "avr.micro") # Arduino Micro

project(KeyboardSimulator C CXX)

set(TARGET_NAME KeyboardSimulator)

add_executable(${TARGET_NAME}
    main.cpp
)

# global Arduino libraries
target_link_arduino_libraries(${TARGET_NAME}
    PRIVATE core
    PRIVATE Keyboard
)

## local Arduino libraries
#target_link_arduino_libraries(${TARGET_NAME}
#    PRIVATE
#)

# This Arduino specific statement is required in order to generate HEX
# and upload it to the board. May be removed in future, but currently
# wrapped with ARDUINO flag to ensure script portability.
if (ARDUINO)
    target_enable_arduino_upload(${TARGET_NAME})
endif()

