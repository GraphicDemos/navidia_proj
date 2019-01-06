project(nvparse)

SET(NVPARSE_SRCS
    src/macro.h
	include/nvparse.h
    src/nvparse.cpp
    src/nvparse_errors.h 
	src/nvparse_errors.cpp
    src/nvparse_externs.h
	src/ps1.0_program.h 
	src/ps1.0_program.cpp
	src/rc1.0_combiners.h 
	src/rc1.0_combiners.cpp
	src/rc1.0_final.h 
	src/rc1.0_final.cpp
	src/rc1.0_general.h 
	src/rc1.0_general.cpp
	src/ts1.0_inst.h 
	src/ts1.0_inst.cpp
	src/ts1.0_inst_list.h 
	src/ts1.0_inst_list.cpp
	src/vcp1.0_impl.cpp
	src/vs1.0_inst.h 
	src/vs1.0_inst.cpp
	src/vs1.0_inst_list.h 
	src/vs1.0_inst_list.cpp
	src/vsp1.0_impl.cpp
	src/vp1.0_impl.cpp
	src/_ps1.0_lexer.cpp
	src/_ps1.0_parser.cpp
	src/_ps1.0_parser.hpp
	src/_rc1.0_lexer.cpp
	src/_rc1.0_parser.cpp
	src/_rc1.0_parser.hpp
	src/_ts1.0_lexer.cpp
	src/_ts1.0_parser.cpp
	src/_ts1.0_parser.hpp
	src/_vs1.0_lexer.cpp
	src/_vs1.0_parser.cpp
	src/_vs1.0_parser.hpp
	)
	
if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

execute_process(COMMAND "rm ${CMAKE_SOURCE_DIR}/src/nvparse/_*")
if (WIN32)
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/src/nvparse/buildparsers.bat ${CMAKE_SOURCE_DIR}/src/nvparse/ WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/nvparse)
else ()
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/src/nvparse/buildparsers.sh ${CMAKE_SOURCE_DIR}/src/nvparse/ WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/nvparse)
endif ()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/flex)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/bison)



INCLUDE_DIRECTORIES(${GLEW_INCLUDE_PATH} ${GLUT_INCLUDE_DIR})

# targets
ADD_DEFINITIONS(-DNVPARSE_EXPORTS)

IF(NVPARSE_SHARED)
    ADD_DEFINITIONS(-DNVPARSE_SHARED=1)
    ADD_LIBRARY(nvparse SHARED ${NVPARSE_SRCS})
ELSE(NVPARSE_SHARED)
    ADD_LIBRARY(nvparse ${NVPARSE_SRCS})
ENDIF(NVPARSE_SHARED)

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

TARGET_LINK_LIBRARIES(nvparse ${LIBS} opengl32 ${GLEW_LIBRARY} ${GLUT_LIBRARY})

INSTALL(TARGETS nvparse
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)

INSTALL(FILES include/nvparse.h  DESTINATION include/nvparse)