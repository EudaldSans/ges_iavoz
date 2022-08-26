# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/llmerlos/esp/env/v4.2/esp-idf/components/bootloader/subproject"
  "/Users/llmerlos/WS/work/simon/iavoz_simon/intpj/micro_speech/build/bootloader"
  "/Users/llmerlos/WS/work/simon/iavoz_simon/intpj/micro_speech/build/bootloader-prefix"
  "/Users/llmerlos/WS/work/simon/iavoz_simon/intpj/micro_speech/build/bootloader-prefix/tmp"
  "/Users/llmerlos/WS/work/simon/iavoz_simon/intpj/micro_speech/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/llmerlos/WS/work/simon/iavoz_simon/intpj/micro_speech/build/bootloader-prefix/src"
  "/Users/llmerlos/WS/work/simon/iavoz_simon/intpj/micro_speech/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/llmerlos/WS/work/simon/iavoz_simon/intpj/micro_speech/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/llmerlos/WS/work/simon/iavoz_simon/intpj/micro_speech/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
