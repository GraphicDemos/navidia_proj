PROJECT(nv_nvb)

SET(NV_NVB_SRCS
	nv_animation.h nv_animation.cpp
	nv_camera.h nv_camera.cpp	
	nv_drawmodel_visitor.h nv_drawmodel_visitor.cpp	
	nv_factory.h nv_factory.cpp
	nv_file.h nv_file.cpp
	nv_light.h nv_light.cpp	
	nv_material.h nv_material.cpp	
	nv_mesh.h nv_mesh.cpp	
	nv_model.h nv_model.cpp
	nv_node.h nv_node.cpp
	nv_nvb.h nv_nvb.cpp
	nv_nvbfactory.h nv_nvbfactory.cpp
	nv_refcount.h nv_refcount.cpp
	nv_scene.cpp nv_scene.h
	nv_texcoord_set.h nv_texcoord_set.cpp
	nv_texture.h nv_texture.cpp
	nv_visitor.h nv_visitor.cpp
	StdAfx.cpp StdAfx.h nv_nvb.rc
	nv_nvbdecl.h nv_visitor.h
	resource.h 
	)
                  
# targets
ADD_DEFINITIONS(-DNV_NVB_EXPORTS)

IF(NV_NVB_SHARED)
	ADD_DEFINITIONS(-DNV_NVB_DLL=1)
	ADD_LIBRARY(nv_nvb SHARED ${NV_NVB_SRCS})
ELSE(NV_NVB_SHARED)
	ADD_LIBRARY(nv_nvb ${NV_NVB_SRCS})
ENDIF(NV_NVB_SHARED)

target_include_directories( nv_nvb PRIVATE  ../nv_math/  ../nvIO ../nv_nvbloader ${CMAKE_CURRENT_SOURCE_DIR}/../3rth/)
add_dependencies(nv_nvb nv_math nvIO nv_nvbloader)

TARGET_LINK_LIBRARIES(nv_nvb ${LIBS} nv_math  nv_nvbloader nvIO ${ZLIB_LIBRARY_RELEASE} )

INSTALL(TARGETS nv_nvb
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static)
