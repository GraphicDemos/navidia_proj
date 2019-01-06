#ifndef DYNAMIC_NORMALMAP
#define DYNAMIC_NORMALMAP

/*
	This module is used to dynamically build normal maps by
	compositing multiple RGB maps, and copying the results to
	the normalmap texture.
	
	This approach produces denormalized normal maps, so it
	will work best with GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV,
	GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV, and 
	GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV texture shader operations. 
	
	This approach will also be most practical when used with the
	SGIS_generate_mipmap extension.

	Cass Everitt
	12-02-00
*/

#include <glh/glh_obs.h>  // for tex_object_2D


void update_normalmap(glh::tex_object_2D & normalmap);

void update_normalmap_bulges(glh::tex_object_2D & normalmap);



#endif
