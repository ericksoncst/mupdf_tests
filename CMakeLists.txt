cmake_minimum_required(VERSION 3.24)
project(MuPDFSample)

# Dependencies
include(FetchContent)

# Fetch MuPDF from GitHub
set(MUPDF_VERSION "1.25.0")
FetchContent_Declare(
    mupdf
    GIT_REPOSITORY https://github.com/ArtifexSoftware/mupdf.git
    GIT_TAG ${MUPDF_VERSION}  # You can specify a tag (e.g., release version) or a branch
)

set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(mupdf)

set(MUPDF_BUILD_DIR ${mupdf_SOURCE_DIR}/build/release)

# Step 1: Generate MuPDF build files using `make generate`
add_custom_target(generate_mupdf
    COMMAND make generate
    WORKING_DIRECTORY ${mupdf_SOURCE_DIR}
    COMMENT "Generating MuPDF build files"
    # Explicitly specify that the output of this step is the libmupdf.a
    # This ensures that `make generate` will be considered complete when libmupdf.a exists
)

# Step 2: Build MuPDF using `make`
add_custom_target(build_mupdf
    COMMAND make
    WORKING_DIRECTORY ${mupdf_SOURCE_DIR}
    COMMENT "Building MuPDF"
    DEPENDS generate_mupdf  # Ensure the generate step is finished before building
    # Specify that libmupdf.a is the result of this build step
)

# Our Project
set(SOURCE_FILES
    mupdfsample.cpp)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Ensure that the MuPDF build steps (generate and build) are completed first
add_dependencies(${PROJECT_NAME} build_mupdf)

# Link the MuPDF libraries
target_link_libraries(${PROJECT_NAME} 
    ${MUPDF_BUILD_DIR}/libmupdf.a
    ${MUPDF_BUILD_DIR}/libmupdf-third.a
)

# Include directories for MuPDF
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${mupdf_SOURCE_DIR}/include
    ${mupdf_SOURCE_DIR}/thirdparty
)
