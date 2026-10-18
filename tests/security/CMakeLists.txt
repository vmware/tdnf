add_executable(security-client client.c)
target_link_libraries(security-client ${LIB_TDNF})
target_compile_options(security-client PRIVATE -UNDEBUG)
add_test(NAME security-client COMMAND security-client)

add_executable(security-solver solver.c)
target_link_libraries(security-solver ${LIB_TDNF})
target_compile_options(security-solver PRIVATE -UNDEBUG)
add_test(NAME security-solver COMMAND security-solver)
