# GenerateGitHash.cmake

execute_process(
    COMMAND git rev-parse --short=8 HEAD
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT GIT_HASH)
    set(GIT_HASH "unknown")
endif()

set(OUTPUT_FILE "${BINARY_DIR}/git_version.h")

file(WRITE "${OUTPUT_FILE}.tmp"
"#pragma once

#define GIT_HASH \"${GIT_HASH}\"
"
)

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${OUTPUT_FILE}.tmp"
            "${OUTPUT_FILE}"
)

file(REMOVE "${OUTPUT_FILE}.tmp")
