add_executable(security-client client.c)
target_link_libraries(security-client ${LIB_TDNF})
target_compile_options(security-client PRIVATE -UNDEBUG)
add_test(NAME security-client COMMAND security-client)
