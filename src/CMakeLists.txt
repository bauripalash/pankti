# Copyright (c) 2022 Palash Bauri
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

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

