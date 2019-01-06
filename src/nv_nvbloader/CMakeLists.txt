PROJECT(nv_nvbloader)

SET(NV_NVB_LOADER_SRCS
	NVBBreaker.h NVBBreaker.cpp
	NVBBuffer.h NVBBuffer.cpp	
	NVBContainer.h NVBContainer.cpp	
	NVBCustomArray.h NVBCustomArray.cpp	
	NVBFile.h NVBFile.cpp
	NVBFormat.h NVBFPU.h	
	NVBLoader.h NVBMemoryMacros.h	
	NVBString.h NVBString.cpp	
	NVBTypes.h NVBUtils.h
	nv_nvb_loader_decl.h nv_streams.h
	nv_streams.cpp 
	nv_attribute.h nv_attribute.cpp
	)
                  
# targets
ADD_DEFINITIONS(-DNV_NVB_LOADER_EXPORTS)

IF(NV_NVB_LOADER_SHARED)
	ADD_DEFINITIONS(-DNV_NVB_LOADER_DLL=1)
	ADD_LIBRARY(nv_nvbloader SHARED ${NV_NVB_LOADER_SRCS})
ELSE(NV_NVB_LOADER_SHARED)
	ADD_LIBRARY(nv_nvbloader ${NV_NVB_LOADER_SRCS})
ENDIF(NV_NVB_LOADER_SHARED)

target_include_directories( nv_nvbloader PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../3rth/ ../nv_nvb ../nv_math)

TARGET_LINK_LIBRARIES(nv_nvbloader ${LIBS} nv_math)

INSTALL(TARGETS nv_nvbloader
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static)
