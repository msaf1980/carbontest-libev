# Conan install
# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.14/conan.cmake"
                 "${CMAKE_SOURCE_DIR}/conan.cmake")
endif()

include(${CMAKE_SOURCE_DIR}/conan.cmake)
conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP NO_OUTPUT_DIRS
                BUILD missing
)
