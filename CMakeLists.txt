cmake_minimum_required(VERSION 3.15)
project(GameCheat_Restored)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Embedded config (injected at build time, e.g. from GitHub Actions secrets)
# Usage: cmake -B build -S . -D EMBEDDED_API_URL=... -D EMBEDDED_SHARED_KEY=...
# ============================================================================
set(EMBEDDED_API_URL "https://majcheat-production.up.railway.app" CACHE STRING "Embedded backend URL")
set(EMBEDDED_SHARED_KEY "" CACHE STRING "Embedded API shared key (use GitHub secret)")
set(EMBEDDED_BOT_USERNAME "" CACHE STRING "Telegram bot username")

# Windows-specific settings
if(WIN32)
    # Static runtime linking (no VC++ redistributables needed)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # Hide console window in Release builds (optional: comment out for debug console)
    # set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# Find required packages
find_package(Threads REQUIRED)

# Source files
set(SOURCES
    main.cpp
    src/MemoryManager.cpp
    src/DLLInjector.cpp
    src/NetworkClient.cpp
    src/DirectXOverlay.cpp
    src/GameCheat.cpp
    src/ConfigManager.cpp
    src/UIRenderer.cpp
    src/Aimbot.cpp
    src/Misc.cpp
    src/Visuals.cpp
    src/JSExecutor.cpp
    src/Logger.cpp
    src/AntiCheatBypass.cpp
    src/LogoRenderer.cpp
    src/KeyBindManager.cpp
    src/OBSBypass.cpp
    src/SelfDestruct.cpp
    src/AdminDetector.cpp
    src/AntiCheatDetector.cpp
    src/HWIDManager.cpp
    src/AuthManager.cpp
    src/TelegramBot.cpp
    src/BackendAPI.cpp
    src/Obfuscator.cpp
    src/AuthFlow.cpp
    src/GameMemory.cpp
)

# Header files
set(HEADERS
    src/MemoryManager.h
    src/DLLInjector.h
    src/NetworkClient.h
    src/DirectXOverlay.h
    src/GameCheat.h
    src/ConfigManager.h
    src/UIRenderer.h
    src/Aimbot.h
    src/Misc.h
    src/Visuals.h
    src/JSExecutor.h
    src/Logger.h
    src/AntiCheatBypass.h
    src/LogoRenderer.h
    src/KeyBindManager.h
    src/OBSBypass.h
    src/SelfDestruct.h
    src/AdminDetector.h
    src/AntiCheatDetector.h
    src/HWIDManager.h
    src/AuthManager.h
    src/TelegramBot.h
    src/BackendAPI.h
    src/Obfuscator.h
    src/AuthFlow.h
    src/EmbeddedConfig.h
    src/GameMemory.h
    src/GameOffsets.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Compiler warnings (treat as errors for critical issues)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

# Inject embedded config as compile-time string defines
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "EMBEDDED_API_URL=\"${EMBEDDED_API_URL}\""
    "EMBEDDED_SHARED_KEY=\"${EMBEDDED_SHARED_KEY}\""
    "EMBEDDED_BOT_USERNAME=\"${EMBEDDED_BOT_USERNAME}\""
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE
    d3d11.lib
    d3dcompiler.lib
    dxgi.lib
    d2d1.lib
    dwrite.lib
    wininet.lib
    ws2_32.lib
    iphlpapi.lib
    psapi.lib
    shell32.lib
    shlwapi.lib
    Threads::Threads
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

# Copy config file to output directory
if(EXISTS ${CMAKE_SOURCE_DIR}/config.ini)
    configure_file(config.ini ${CMAKE_BINARY_DIR}/bin/config.ini COPYONLY)
endif()

# Copy scripts directory if exists
if(EXISTS ${CMAKE_SOURCE_DIR}/scripts)
    file(COPY ${CMAKE_SOURCE_DIR}/scripts DESTINATION ${CMAKE_BINARY_DIR}/bin)
endif()
