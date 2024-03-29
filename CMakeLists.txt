cmake_minimum_required(VERSION 3.12)
set(CMAKE_TOOLCHAIN_FILE "$ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cmake/psp2-snc-toolchain.cmake")
project(QuickLauncher)
include(VitaDevelopmentSuite)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
enable_language(EMD)

add_compile_options(
    -D_SCE_TARGET_OS_PSP2=1 -Xdiag=0 -Xquit=2
)
add_link_options(
  --prx-no-runtime-support
)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
  ${VDSUITE_STANDARD_INCLUDE_DIRECTORIES}
)

link_directories(
  ${VDSUITE_LIBRARY_DIRECTORIES}
)

add_library(${PROJECT_NAME} MODULE
    ${PROJECT_NAME}.emd
    src/main.c
)

target_link_libraries(${PROJECT_NAME}
    SceKernel_stub
    QuickMenuReborn_stub
    ScePafStdc_stub
    ScePafMisc_stub
    SceAppMgrUser_stub_weak
)

VDSuiteSignElf(${PROJECT_NAME}.suprx ${PROJECT_NAME})