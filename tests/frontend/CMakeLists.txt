# Pankti Native Tests
include("${CMAKE_SOURCE_DIR}/src/sources.cmake")
include("${CMAKE_SOURCE_DIR}/tests/frontend/tests.cmake")


add_executable(pankti_tests 
	${PANKTI_SRC_FILES}
	${PANKTI_HEADER_FILES}
	${PANKTI_NATIVE_TEST_FILES}
)

add_compile_definitions(NO_GFX_SUPPORT)

target_link_libraries(pankti_tests PRIVATE libgrapheme)

if (NOT IS_MSVC)
	target_link_libraries(pankti_tests PRIVATE m)
endif()
