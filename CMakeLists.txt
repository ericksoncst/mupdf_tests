cmake_minimum_required(VERSION 3.24)
project(MuPDFSample)

# Path to pre-built MuPDF
set(MUPDF_ROOT ${CMAKE_SOURCE_DIR}/thirdparty/mupdf)

# Include MuPDF headers
include_directories(${MUPDF_ROOT})
include_directories(${MUPDF_ROOT}/third)

# Set source files for your project
set(SOURCE_FILES
    mupdfsample.cpp)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link to the pre-built MuPDF libraries
target_link_libraries(${PROJECT_NAME}
    ${MUPDF_ROOT}/build/release/libmupdf.a
    ${MUPDF_ROOT}/build/release/libmupdf-third.a
)

# Ensure that we include any other necessary directories from MuPDF
target_include_directories(${PROJECT_NAME} PRIVATE
    ${MUPDF_ROOT}/third
)
