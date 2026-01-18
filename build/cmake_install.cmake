# Install script for directory: /Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/build/libChoirV2PureDSP.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libChoirV2PureDSP.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libChoirV2PureDSP.a")
    execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libChoirV2PureDSP.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ChoirV2" TYPE FILE FILES
    "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/include/dsp/ChoirV2PureDSP.h"
    "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/include/dsp/core/PhonemeTypes.h"
    "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/include/dsp/core/PhonemeDatabase.h"
    "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/include/dsp/core/G2PEngine.h"
    "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/include/dsp/core/LanguageLoader.h"
    "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/include/dsp/synthesis/ISynthesisMethod.h"
    "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/include/dsp/synthesis/FormantSynthesisMethod.h"
    "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/include/dsp/synthesis/DiphoneSynthesisMethod.h"
    "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/include/dsp/synthesis/SubharmonicSynthesisMethod.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
