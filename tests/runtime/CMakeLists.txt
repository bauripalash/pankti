set(RUNTIME_TEST_SRC test_runtime.c)
set(RUNTIME_TEST_EXE pankti_runtime_tests)
add_executable(${RUNTIME_TEST_EXE}
    ${RUNTIME_TEST_SRC}
)

if (NOT IS_MSVC)
    target_link_libraries(${RUNTIME_TEST_EXE} PRIVATE m)
endif()
