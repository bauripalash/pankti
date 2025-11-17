# Pankti Source
include("${CMAKE_CURRENT_LIST_DIR}/sources.cmake")

target_sources(${PROJECT_NAME}
	PRIVATE 
	${PANKTI_SRC_FILES} 
	${PANKTI_MAIN_FILE} 
	${PANKTI_HEADER_FILES}
)

