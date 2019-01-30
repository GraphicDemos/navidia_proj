# ================================================= Caustics 1D =================================================
file(GLOB Caustics_SRC
    "Caustics.cpp"
	"plane.cpp"
	"point.cpp"
	"texmanager.cpp"
	"text.cpp"
	"texture.cpp"
)
 						   
add_executable(Caustics WIN32 ${Caustics_SRC})
set_target_properties(Caustics PROPERTIES PROJECT_LABEL "Caustics")						
set_target_properties(Caustics PROPERTIES OUTPUT_NAME "Caustics")
set_target_properties(Caustics PROPERTIES FOLDER "examples/GpuGems")
target_include_directories(Caustics PRIVATE  ${GLEW_INCLUDE_PATH} ${GLUT_INCLUDE_DIR})
target_link_libraries(Caustics opengl32 ${GLUT_glut_LIBRARY} ${GLEW_LIBRARY})
install(TARGETS Caustics RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# ================================================= CausticsSnell =================================================
file(GLOB CausticsSnell_SRC
    "CausticsSnell.cpp"
	"plane.cpp"
	"point.cpp"
	"texmanager.cpp"
	"text.cpp"
	"texture.cpp"
)
 						   
add_executable(CausticsSnell WIN32 ${CausticsSnell_SRC})															
set_target_properties(CausticsSnell PROPERTIES PROJECT_LABEL "CausticsSnell")						
set_target_properties(CausticsSnell PROPERTIES OUTPUT_NAME "CausticsSnell")
set_target_properties(CausticsSnell PROPERTIES FOLDER "examples/GpuGems")
target_include_directories(CausticsSnell PRIVATE  ${GLEW_INCLUDE_PATH} ${GLUT_INCLUDE_DIR})
target_link_libraries(CausticsSnell opengl32 ${GLUT_glut_LIBRARY} ${GLEW_LIBRARY})
install(TARGETS CausticsSnell RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

 
