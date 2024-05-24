# CMakeList.txt: CMake-Projekt "RGBtri". 
#
cmake_minimum_required (VERSION 3.8)

# In case of MSVC-Compiler
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("RGBtri")

find_package(NIFTI CONFIG REQUIRED)

# sources
add_executable (RGBtri "RGBtri.cpp" "RGBtri.h")
include_directories(${NIFTI_PACKAGE_PREFIX}include/nifti)
target_link_libraries(RGBtri PUBLIC ${NIFTI_PACKAGE_PREFIX}NIFTI::${NIFTI_PACKAGE_PREFIX}niftiio)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RGBtri PROPERTY CXX_STANDARD 20)
endif()

