cmake_minimum_required(VERSION 3.12)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(TheDieIsCast VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(${CMAKE_SOURCE_DIR}/External/include)
include_directories(${CMAKE_SOURCE_DIR}/Include)
file(GLOB SRC "Source/*.cpp")

if(WIN32)
    set(ICON_FILE ${CMAKE_SOURCE_DIR}/Resources/Icon/icon.rc)
    add_executable(TheDieIsCast WIN32 ${SRC} ${ICON_FILE})
elseif(APPLE)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER THEDIEISCAST)
    set(MACOSX_BUNDLE_BUNDLE_VERSION 1.0)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0)
    add_executable(TheDieIsCast MACOSX_BUNDLE ${SRC})
    file(COPY ${CMAKE_SOURCE_DIR}/Resources 
    ${CMAKE_SOURCE_DIR}/Script 
    ${CMAKE_SOURCE_DIR}/History   
    DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Resources)
    file(COPY ${CMAKE_SOURCE_DIR}/Resources/Info.plist 
    DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents)
    file(COPY ${CMAKE_SOURCE_DIR}/Resources/Icon/icon.icns
    DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Resources)
elseif(UNIX AND NOT APPLE)
    add_executable(TheDieIsCast ${SRC})
else()
    message(FATAL_ERROR "system not support")
endif()

if(WIN32)
    set(SFML_ROOT ${CMAKE_SOURCE_DIR}/External/lib_win32)
    target_link_libraries(TheDieIsCast
     ${SFML_ROOT}/sfml-system.lib 
     ${SFML_ROOT}/sfml-audio.lib 
     ${SFML_ROOT}/sfml-graphics.lib 
     ${SFML_ROOT}/sfml-window.lib)
elseif(APPLE)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    if(CORE_FOUNDATION_LIBRARY)
        target_link_libraries(TheDieIsCast ${CORE_FOUNDATION_LIBRARY})
    else()
        message(FATAL_ERROR "CoreFoundation framework not found")
    endif()
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        if (EXISTS ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks)
            file(DELETE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks)
        endif()
        if (EXISTS ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks_64)
            file(DELETE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks_64)
        endif()
        file(COPY ${CMAKE_SOURCE_DIR}/External/Frameworks_64
        ${CMAKE_SOURCE_DIR}/External/lib_mac_64
        DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External)
        file(RENAME ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks_64
        ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks)
        set(SFML_ROOT "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/lib_mac_64")
        target_link_options(TheDieIsCast PRIVATE
        -Wl,-rpath,@executable_path/../External/lib_mac_64
        )
        add_custom_command(TARGET TheDieIsCast POST_BUILD
        COMMAND install_name_tool -id "@rpath/libsfml-system.2.6.2.dylib" "${SFML_ROOT}/libsfml-system.2.6.2.dylib"
        COMMAND install_name_tool -id "@rpath/libsfml-audio.2.6.2.dylib" "${SFML_ROOT}/libsfml-audio.2.6.2.dylib"
        COMMAND install_name_tool -id "@rpath/libsfml-graphics.2.6.2.dylib" "${SFML_ROOT}/libsfml-graphics.2.6.2.dylib"
        COMMAND install_name_tool -id "@rpath/libsfml-window.2.6.2.dylib" "${SFML_ROOT}/libsfml-window.2.6.2.dylib"
        COMMAND install_name_tool -change "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/lib_mac_64/libsfml-system.2.6.2.dylib" "@rpath/libsfml-system.2.6.2.dylib" "${SFML_ROOT}/libsfml-audio.2.6.2.dylib"
        )
        target_link_libraries(TheDieIsCast
        ${SFML_ROOT}/libsfml-system.2.6.2.dylib 
        ${SFML_ROOT}/libsfml-audio.2.6.2.dylib 
        ${SFML_ROOT}/libsfml-graphics.2.6.2.dylib 
        ${SFML_ROOT}/libsfml-window.2.6.2.dylib)
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        if (EXISTS ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks)
            file(DELETE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks)
        endif()
        if (EXISTS ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks_arm)
            file(DELETE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks_arm)
        endif()
        file(COPY ${CMAKE_SOURCE_DIR}/External/Frameworks_arm
        ${CMAKE_SOURCE_DIR}/External/lib_mac_arm
        DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External)
        file(RENAME ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks_arm
        ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/Frameworks)
        set(SFML_ROOT "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/lib_mac_arm")
        target_link_options(TheDieIsCast PRIVATE
        -Wl,-rpath,@executable_path/../External/lib_mac_arm
        )
        add_custom_command(TARGET TheDieIsCast POST_BUILD
        COMMAND install_name_tool -id "@rpath/libsfml-system.2.6.2.dylib" "${SFML_ROOT}/libsfml-system.2.6.2.dylib"
        COMMAND install_name_tool -id "@rpath/libsfml-audio.2.6.2.dylib" "${SFML_ROOT}/libsfml-audio.2.6.2.dylib"
        COMMAND install_name_tool -id "@rpath/libsfml-graphics.2.6.2.dylib" "${SFML_ROOT}/libsfml-graphics.2.6.2.dylib"
        COMMAND install_name_tool -id "@rpath/libsfml-window.2.6.2.dylib" "${SFML_ROOT}/libsfml-window.2.6.2.dylib"
        COMMAND install_name_tool -change "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/External/lib_mac_arm/libsfml-system.2.6.2.dylib" "@rpath/libsfml-system.2.6.2.dylib" "${SFML_ROOT}/libsfml-audio.2.6.2.dylib"
        )
        target_link_libraries(TheDieIsCast
        ${SFML_ROOT}/libsfml-system.2.6.2.dylib 
        ${SFML_ROOT}/libsfml-audio.2.6.2.dylib 
        ${SFML_ROOT}/libsfml-graphics.2.6.2.dylib  
        ${SFML_ROOT}/libsfml-window.2.6.2.dylib)
    endif()
elseif(UNIX AND NOT APPLE)
    set(SFML_ROOT ${CMAKE_SOURCE_DIR}/External/lib_linux)
    target_link_libraries(TheDieIsCast
    ${SFML_ROOT}/libsfml-system.so.2.6.2 
    ${SFML_ROOT}/libsfml-audio.so.2.6.2 
    ${SFML_ROOT}/libsfml-graphics.so.2.6.2 
    ${SFML_ROOT}/libsfml-window.so.2.6.2)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()