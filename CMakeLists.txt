# Projects Settings
cmake_minimum_required(VERSION 4.2)
project(sald-sandbox3d)

# `cmake --install` goes to release/ unless a prefix is given (--prefix <dir>)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/release" CACHE PATH "Install directory" FORCE)
endif()

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
    src/main.cpp
    src/SandboxApp.cpp
    src/SandboxLayer.cpp
    src/TownLayer.cpp
    src/BuildingCollision.cpp
    src/Terrain.cpp
)

add_subdirectory(vendor/sald)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE sald)

# The app loads Shaders/, Textures/ and Models/ relative to the working
# directory, so copy them next to the built executable
foreach(ASSET_DIR Shaders Textures Models)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/src/${ASSET_DIR}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/${ASSET_DIR}
    )
endforeach()

# Debug from Visual Studio in the folder the assets were copied to
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION .
)

install(DIRECTORY src/Shaders src/Textures src/Models
    DESTINATION .
)
