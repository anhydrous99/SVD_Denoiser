cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

# HUNTER GATE STUFF
include("${CMAKE_SOURCE_DIR}/cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.27.tar.gz"
    SHA1 "70d75a25f895df3da00d8382417e8c2a08c93173"
)

project(SVD_Denoiser)

hunter_add_package(PNG)
hunter_add_package(Eigen)

find_package(PNG CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

include_directories(${LAPACK_lapack_LIBRARIES})

set(CMAKE_BUILD_TYPE Debug)

# Bring in project headers
include_directories(include)

set(SOURCES src/pngreq.cpp src/denoise.cpp src/main.cpp)

add_executable(SVD_Denoiser ${SOURCES})

target_link_libraries(SVD_Denoiser PNG::png OpenMP::OpenMP_CXX Eigen3::Eigen)
