# Pankti Native Tests
include("${CMAKE_SOURCE_DIR}/src/sources.cmake")
include("${CMAKE_SOURCE_DIR}/tests/tests.cmake")

add_executable(pankti_tests 
	${PANKTI_SRC_FILES}
	${PANKTI_HEADER_FILES}
	${PANKTI_NATIVE_TEST_FILES}
)

target_link_libraries(pankti_tests PRIVATE m)
