cmake_minimum_required(VERSION 3.10)
project(process_scheduling)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# --- Scheduler Library ---
add_library(scheduler_lib STATIC
    src/scheduler.cpp
)

# --- Scheduler WASM (Emscripten only) ---
if(EMSCRIPTEN)
    add_executable(scheduler_wasm
        src/wasm_main.cpp
    )
    target_link_libraries(scheduler_wasm PRIVATE scheduler_lib)
    
    # Optional: Set output file extension to .html or .js if desired, usually helpful for emscripten
    # set_target_properties(scheduler_wasm PROPERTIES SUFFIX ".html")
else()
    message(STATUS "Skipping scheduler_wasm target (EMSCRIPTEN not defined)")
endif()

# --- Scheduler Server (Native: Linux or Windows) ---
# Only build server if NOT Emscripten (or if you want it built in emscripten too, but usually server implies native)
# The user prompt implies: "the C++ Launcher executable (links to httplib, for Windows/Linux)"
# So we probably want to exclude it for Emscripten if it relies on things Emscripten doesn't support, 
# but technically one can build server-like things in Wasm. 
# However, usually "C++ Launcher" implies the host. 
# Let's keep it available for all non-Emscripten builds OR if the user explicitly wants it.
# Simpler: Always define it, unless EMSCRIPTEN is set? 
# The prompt says: "scheduler_server: The C++ Launcher executable". 
# Usually one doesn't run the server IN the browser.
if(NOT EMSCRIPTEN)
    add_executable(scheduler_server
        src/server_main.cpp
    )
    target_link_libraries(scheduler_server PRIVATE scheduler_lib)

    # Link httplib
    # Assuming header-only, but we might need system libraries.
    # On Linux, httplib often needs pthread.
    if(UNIX AND NOT APPLE)
        target_link_libraries(scheduler_server PRIVATE pthread)
    endif()

    # On Windows, need ws2_32
    if(WIN32)
        target_link_libraries(scheduler_server PRIVATE ws2_32)
        # Static linking request
        target_link_options(scheduler_server PRIVATE -static)
    endif()
endif()

# --- Test Runner (Local) ---
add_executable(scheduler_test
    tests/test_runner.cpp
)
target_link_libraries(scheduler_test PRIVATE scheduler_lib)
