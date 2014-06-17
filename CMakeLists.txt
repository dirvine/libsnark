cmake_minimum_required (VERSION 2.8)

#ban in source builds (keeps the tree clean)

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  set(Msg "\n\nThis project doesn't allow \"in-source builds\".\nYou must run CMake from a folder outside")
  set(Msg "${Msg} the root of this project.\nFor more details about configuring the project see\n")
  set(Msg "${Msg} https://github.com/scipr-lab/libsnark/blob/master/README.md")
  message(WARNING "${Msg}\n\n")
  message(STATUS "Cancelling CMake and cleaning up source tree...")
  execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/CMakeFiles")
  # This next call should cause CMake to crash.  We should remove this dirty hack if CMake becomes
  # able to be cancelled in a clean way (i.e. doesn't leave behind files/folders).
  math(EXPR Crash 0/0)
  message(FATAL_ERROR "CMake should have crashed - this is a failsafe in case the call used to trigger the crash gets fixed.")
endif()

project(libsnark)
# to add all tests to ctest they need located here
set(EXECUTABLE_OUTPUT_PATH ${libsnark_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${libsnark_BINARY_DIR})
# Dependencies
include(CheckIncludeFileCXX)
check_include_file_cxx(gmpxx.h HAVE_GMPXX)
if (${HAVE_GMPXX})
  message("gmpxx found OK!")
else()
  message(FATAL_ERROR "could not locate gpmxx package, please install")
endif()



#compiler 
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CLANG ON)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(GCC ON)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    set(INTEL ON)
endif()

message("To define the curve to use (-DCURVE=XXX), one of: ALT_BN128, BN128 (default), EDWARDS, MNT4, MNT6 ")
if ("${CURVE}" STREQUAL "")
  set(CURVE "BN128")
endif()

#options
option(LIBCXX "Use libc++ (clang), default on APPLE" OFF)
option(LIBCXXABI "Use libc++abi (clang), if linked against libc++" OFF)
option(BINARY_OUTPUT "in serialization, output raw binary data (instead of decimal, when not set)" OFF) 
option(LOWMEM "Limit the size of multi-exponentiation tables, for low-memory platforms" OFF)
option(MINDEPS "Rely on a minimum amount of external libraries (at the cost of some functionality). In particular, do not rely on boost::program_options and libprocps" ON)
option(MULTICORE "Enable parallelized execution of the ppzkSNARK generator and prover, using OpenMP." OFF)
option(NO_PT_COMPRESSION "Do not use point compression. This gives much faster serialization times, at the expense of ~2x larger sizes for serialized keys and proofs" OFF)
option(PROFILE_OP_COUNTS "Collect counts for field and curve operations inside static variables of the corresponding algebraic objects. This option works for all curves except bn128." ON)
option(USE_ASM "Use unrolled assembly routines for Fp arithmetic and faster heap in multi-exponentiation. " OFF) 
option(PERFORMANCE "Enables various compiler optimizations for the current CPU, and disables debugging aids." ON)


add_definitions("-DCURVE_${CURVE}")

if ("${USE_ASM}")
  add_definitions(-DUSE_ASM)
endif()

if ("${MINDEPS}")
  add_definitions(-DMINDEPS)
else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lprocps")
endif()



#compiler settings
if (NOT "${CLANG}")
  set(OMP "-fopenmp")
endif()

if("${LIBCXX}" OR APPLE)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

if(LIBCXXABI)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
endif()

if(UNIX)  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Wno-unused-parameter") 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comment -march=native -mtune=native -std=c++11 ${OMP}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-operator-names")
endif()

# Create json database, used by many tools (vim, clang-format etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories (.)
#ignore any installed gtest headers as they may be incompatible
include_directories (BEFORE SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/include)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/third_party/xbyak)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/third_party/ate-pairing/include)

add_subdirectory(third_party)

add_library(zksnark
  #include ate-pairing files here, saves linking
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ate-pairing/src/zm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ate-pairing/src/zm2.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/alt_bn128/alt_bn128_g1.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/alt_bn128/alt_bn128_g2.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/alt_bn128/alt_bn128_init.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/alt_bn128/alt_bn128_pairing.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/alt_bn128/alt_bn128_pp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/common/profiling.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/common/utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/examples/simple_example.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/infrastructure.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/adapters.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/constraint.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/gadget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/integration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/pp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/protoboard.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/variable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/bn128/bn128_g1.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/bn128/bn128_g2.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/bn128/bn128_gt.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/bn128/bn128_init.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/bn128/bn128_pairing.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/bn128/bn128_pp.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/edwards/edwards_g1.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/edwards/edwards_g2.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/edwards/edwards_init.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/edwards/edwards_pairing.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/algebra/curves/edwards/edwards_pp.cpp)

add_executable(ppzsnark_profile 
	${CMAKE_CURRENT_SOURCE_DIR}/src/r1cs_ppzksnark/examples/profile_r1cs_ppzksnark.cpp)
 add_executable(tutorial
   ${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/examples/tutorial.cpp) 
add_executable(gadgetlib_test 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/tests/adapters_UTEST.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/tests/constraint_UTEST.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/tests/gadget_UTEST.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/tests/protoboard_UTEST.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/tests/variable_UTEST.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gadgetlib2/tests/gadgetlib2_test.cpp)

target_link_libraries(ppzsnark_profile zksnark gmpxx gmp)
target_link_libraries(tutorial zksnark gtest gmpxx gmp )
target_link_libraries(gadgetlib_test zksnark gtest gmpxx gmp)

include(CTest)

add_test(NAME gadgetlib_test COMMAND gadgetlib_test)
add_test(NAME tutorial COMMAND tutorial)

if(${gtest_build_samples})
  add_test(NAME gtest-sample1_unittest COMMAND sample1_unittest)
  add_test(NAME gtest-sample2_unittest COMMAND sample2_unittest)
  add_test(NAME gtest-sample3_unittest COMMAND sample3_unittest)
  add_test(NAME gtest-sample4_unittest COMMAND sample4_unittest)
  add_test(NAME gtest-sample5_unittest COMMAND sample5_unittest)
  add_test(NAME gtest-sample6_unittest COMMAND sample6_unittest)
  add_test(NAME gtest-sample7_unittest COMMAND sample7_unittest)
  add_test(NAME gtest-sample8_unittest COMMAND sample8_unittest)
  add_test(NAME gtest-sample9_unittest COMMAND sample9_unittest)
endif()

if(${gtest_build_tests})
  add_test(NAME gtest-death-test_test COMMAND gtest-death-test_test)
  add_test(NAME gtest_environment_test COMMAND gtest_environment_test)
  add_test(NAME gtest-filepath_test COMMAND gtest-filepath_test)
  add_test(NAME gtest-linked_ptr_test COMMAND gtest-linked_ptr_test)
  add_test(NAME gtest-listener_test COMMAND gtest-listener_test )
  add_test(NAME gtest_main_unittest COMMAND gtest_main_unittest)
  add_test(NAME gtest-message_test COMMAND gtest-message_test)
  add_test(NAME gtest_no_test_unittest COMMAND gtest_no_test_unittest)
  add_test(NAME gtest-options_test COMMAND gtest-options_test)
  add_test(NAME gtest-param-test_test COMMAND gtest-param-test_test)
  add_test(NAME gtest-port_test COMMAND gtest-port_test)
  add_test(NAME gtest_pred_impl_unittest COMMAND gtest_pred_impl_unittest)
  add_test(NAME gtest_premature_exit_test COMMAND gtest_premature_exit_test)
  add_test(NAME gtest-printers_test COMMAND gtest-printers_test)
  add_test(NAME gtest_prod_test COMMAND gtest_prod_test)
  add_test(NAME gtest_repeat_test COMMAND gtest_repeat_test)
  add_test(NAME gtest_sole_header_test COMMAND gtest_sole_header_test)
  add_test(NAME gtest_stress_test COMMAND gtest_stress_test)
  add_test(NAME gtest-test-part_test  COMMAND gtest-test-part_test)
  add_test(NAME gtest_throw_on_failure_ex_test COMMAND gtest_throw_on_failure_ex_test)
  add_test(NAME gtest-typed-test_test  COMMAND gtest-typed-test_test)
  add_test(NAME gtest_unittest  COMMAND gtest_unittest)
  add_test(NAME gtest-unittest-api_test COMMAND gtest-unittest-api_test)
endif()

