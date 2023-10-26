llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support )
file(GLOB SOURCES "lib/*.cpp" "lib/**/*.cpp")
include_directories(include)
add_executable(svf-util ${SOURCES})
set(BIN_PATH "${CMAKE_BUILD_TYPE}-build")
target_link_libraries(svf-util ${SVF_LIB} ${llvm_libs})
target_link_libraries(svf-util ${Z3_LIBRARIES})
set_target_properties(svf-util PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${BIN_PATH} )