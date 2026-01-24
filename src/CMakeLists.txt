# Pankti Source
include("${CMAKE_CURRENT_LIST_DIR}/sources.cmake")

if(IS_GFX_BUILD)
    list(APPEND PANKTI_SRC_FILES "${PANKTI_GFX_SRC_FILES}")
endif()

target_sources(${PROJECT_NAME}
	PRIVATE 
	${PANKTI_SRC_FILES} 
	${PANKTI_MAIN_FILE} 
	${PANKTI_HEADER_FILES}
)

