add_executable(security-client client.c)
target_link_libraries(security-client ${LIB_TDNF})
target_compile_options(security-client PRIVATE -UNDEBUG)
add_test(NAME security-client COMMAND security-client)

add_executable(security-metalink metalink.c)
target_link_libraries(security-metalink tdnfmetalink ${LIB_TDNF})
target_compile_options(security-metalink PRIVATE -UNDEBUG)
add_test(NAME security-metalink COMMAND security-metalink)

add_executable(security-solver solver.c)
target_link_libraries(security-solver ${LIB_TDNF})
target_compile_options(security-solver PRIVATE -UNDEBUG)
add_test(NAME security-solver COMMAND security-solver)
