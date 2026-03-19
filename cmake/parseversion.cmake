function(ParseVersion header)
    file(READ "${header}" rawfile)
    foreach(var MAJOR MINOR PATCH)
        string(REGEX MATCH 
                "#define PANKTI_VERSION_${var}[ \t]+([0-9]+)" _ ${rawfile})

        if(CMAKE_MATCH_1 STREQUAL "")
            message(FATAL_ERROR 
                "ParseVersion(..) could not find #define PANKTI_VERSION_${var}")
        endif()
        set(PANKTI_VERSION_${var} ${CMAKE_MATCH_1} PARENT_SCOPE)
    endforeach()

    string(REGEX MATCH 
        "#define PANKTI_RELEASE_LEVEL[ \t]+\"([^\"]*)\"" _ ${rawfile})
    if(CMAKE_MATCH_1 STREQUAL "")
        set(PANKTI_RELEASE_LEVEL "" PARENT_SCOPE)
    else()
        set(PANKTI_RELEASE_LEVEL "${CMAKE_MATCH_1}" PARENT_SCOPE)
    endif()
endfunction()
