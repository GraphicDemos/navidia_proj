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

#ifdef _WIN32
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#endif

#include <vector>

#include "dynamic_normalmap.h"
#include <glh/glh_convenience.h>
#include <shared/array_texture.h>
#include <shared/bumpmap_to_normalmap.h>
#include "../nvparse/include/nvparse.h"
#ifndef _WIN32
float min(float x,float y)  {return ((x>y)?(y):(x));}
#endif
using namespace std;
using namespace glh;

namespace
{
	bool initialized = false;
	
	struct dynamic_normalmap_state
	{
		struct component
		{
			float freq;
			float phase;
			float dphase;
			void incr() { phase += dphase; phase -= floor(phase); }
		};

		component cmp[4];
		tex_object_2D basetex;
		display_list combiner_config;
	};	

	dynamic_normalmap_state dn;

	void initialize()
	{
		initialized = true;

		float freqscale = 4;
		dn.cmp[0].freq = 3         * freqscale;
		dn.cmp[1].freq = 5.4101    * freqscale;
		dn.cmp[2].freq = 7.90032   * freqscale;
		dn.cmp[3].freq = 10.0602   * freqscale;

		dn.cmp[0].dphase = .011111;
		dn.cmp[1].dphase = .009111;
		dn.cmp[2].dphase = .013511;
		dn.cmp[3].dphase = .004111;

		int sz = 64;
		array2<vec3ub> nmap(sz, sz);
		{

			for(int i=0; i < sz; i++)
			{
				for(int j=0; j < sz; j++)
				{
					vec3f dpdi, dpdj, n;

					float theta_i = i/(sz-1.f) * 2 * 3.14159;
					float theta_j = j/(sz-1.f) * 2 * 3.14159;

					// position would be -> p = vec3f(i, j, sin(theta_i) + sin(theta_j))

					float cos_theta_i = cos(theta_i);
					float cos_theta_j = cos(theta_j);

					dpdi = vec3f(1, 0, cos_theta_i);
					dpdj = vec3f(0, 1, cos_theta_j);
					n = dpdi.cross(dpdj);
					n.normalize();
					n += 1.f;
					n *= 127.5;
					nmap(i,j)[0] = (unsigned char) n[0];
					nmap(i,j)[1] = (unsigned char) n[1];
					nmap(i,j)[2] = (unsigned char) n[2];
				}
			}
			dn.basetex.bind();
			make_rgb_texture(nmap, true);
			dn.basetex.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			dn.basetex.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			dn.basetex.parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
			dn.basetex.parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		dn.combiner_config.new_list(GL_COMPILE);
		nvparse(
			"!!RC1.0                                                          \n"
			"    {                                                            \n"
			"       rgb                                                       \n"
			"         {                                                       \n"
			"            discard = half_bias(tex0);                           \n"
			"            discard = half_bias(tex1);                           \n"
			"            spare0 = sum();                                      \n"
			"            scale_by_one_half();                                 \n"
			"         }                                                       \n"
			"    }                                                            \n"
			"    {                                                            \n"
			"       rgb                                                       \n"
			"         {                                                       \n"
			"            discard = half_bias(tex2);                           \n"
			"            discard = half_bias(tex3);                           \n"
			"            spare1 = sum();                                      \n"
			"            scale_by_one_half();                                 \n"
			"         }                                                       \n"
			"    }                                                            \n"
			"    {                                                            \n"
			"       rgb                                                       \n"
			"         {                                                       \n"
			"            discard = spare0;                                    \n"
			"            discard = spare1;                                    \n"
			"            spare0 = sum();                                      \n"
			"            scale_by_one_half();                                 \n"
			"         }                                                       \n"
			"    }                                                            \n"
			"    {                                                            \n"
			"       rgb                                                       \n"
			"         {                                                       \n"
			"            discard = spare0;                                    \n"
			"            discard = -half_bias(zero);                          \n"
			"            spare0 = sum();                                      \n"
			"         }                                                       \n"
			"    }                                                            \n"
			"    out.rgb = spare0;                                            \n"
			"    out.a   = zero;                                              \n"
			);
		for (char * const * errors = nvparse_get_errors(); *errors; errors++)
			fprintf(stderr, *errors);
		dn.combiner_config.end_list();

	}

}

void update_normalmap(tex_object_2D & normalmap)
{
	if( ! initialized ) initialize();

	glMatrixMode(GL_TEXTURE);

	GLenum tex_unit[4] = {GL_TEXTURE0_ARB, GL_TEXTURE1_ARB, GL_TEXTURE2_ARB, GL_TEXTURE3_ARB};

	{
		for(int i=0; i < 4; i++)
		{
			dn.cmp[i].incr();
			
			glActiveTextureARB(tex_unit[i]);
			dn.basetex.bind();
			dn.basetex.enable();
			glLoadIdentity();
			glTranslatef(dn.cmp[i].phase, dn.cmp[i].phase, 0);
			glScalef(dn.cmp[i].freq, dn.cmp[i].freq, 1);
		}
	}

	dn.combiner_config.call_list();
	glEnable(GL_REGISTER_COMBINERS_NV);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(-1, -1, 0);
	glScalef(2,2,1);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	glViewport(0, 0, 512, 512);

	glDisable(GL_DEPTH_TEST);

	GLfloat x, y;
	glBegin(GL_QUADS);

	x = 0; y = 0;
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE2_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE3_ARB, x, y);
	glVertex2f(x, y);	

	x = 0; y = 1;
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE2_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE3_ARB, x, y);
	glVertex2f(x, y);	

	x = 1; y = 1;
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE2_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE3_ARB, x, y);
	glVertex2f(x, y);	

	x = 1; y = 0;
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE2_ARB, x, y);
	glMultiTexCoord2fARB(GL_TEXTURE3_ARB, x, y);
	glVertex2f(x, y);	

	glEnd();


	// copy the rendering
	glActiveTextureARB(GL_TEXTURE0_ARB);
	normalmap.bind();
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0,   0, 0,    0, 0, 512, 512);


	glEnable(GL_DEPTH_TEST);

	glViewport(vp[0], vp[1], vp[2], vp[3]);


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glDisable(GL_REGISTER_COMBINERS_NV);

	glMatrixMode(GL_TEXTURE);
	{
		for(int i=0; i < 4; i++)
		{
			glActiveTextureARB(tex_unit[i]);
			dn.basetex.disable();
			glLoadIdentity();
		}
	}
	glMatrixMode(GL_MODELVIEW);


}

namespace 
{
	bool bulges_initialized = false;
	struct particle
	{
		float x, y;
		float dxdt, dydt; 
		float radius;
	};
	
	vector<particle> particles;
	
	tex_object_2D bulge;
	display_list combiner_identity, combiner_negate;
	
	float myrand() 
	{
		float f = rand() / float(RAND_MAX) ;
		return f;
	}

	void initialize_bulges()
	{
		bulges_initialized = true;
		
		{
			for(int i=0; i < 60; i++)
			{
				particle p;
				p.x = myrand() - .5;
				p.y = myrand() - .5;
				p.dxdt = .01 - .02 * myrand();
				p.dydt = .01 - .02 * myrand();
				p.radius = .2 * myrand() + .1;
				particles.push_back(p);
			}
		}


		int sz = 128;
		array2<vec3ub> nmap;
		array2<unsigned char> bmap(sz, sz);
		array2<unsigned char> alpha(sz, sz);
		{
			
			for(int i=0; i < sz; i++)
			{
				for(int j=0; j < sz; j++)
				{
					float x, y, z;
					
					x = i/(sz-1.f) * 2 - 1;
					y = j/(sz-1.f) * 2 - 1;

					x *= 1.05;
					y *= 1.05;
	
					float xxyy = (x*x+y*y) * 12;
					z = pow(2.0f, -xxyy);

					bmap(i,j) = (unsigned char)(z * 255);
					alpha(i,j) = (unsigned char)min(255, (x*x+y*y) * 255);
				}
			}
			bumpmap_to_normalmap(bmap, nmap, vec3f(1,1,.1));
			bulge.bind();
			make_rgba_texture(nmap, alpha, true);
			bulge.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			bulge.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			bulge.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bulge.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			
		}

		combiner_identity.new_list(GL_COMPILE);
		nvparse(
			"!!RC1.0                                                          \n"
			"    {                                                            \n"
			"       rgb                                                       \n"
			"         {                                                       \n"
			"            spare0 = half_bias(tex0);                            \n"
			"         }                                                       \n"
			"    }                                                            \n"
			"    out.rgb = spare0;                                            \n"
			"    out.a   = tex0;                                              \n"
			);
		{
		for (char * const * errors = nvparse_get_errors(); *errors; errors++)
			fprintf(stderr, *errors);
		}
		combiner_identity.end_list();


		combiner_negate.new_list(GL_COMPILE);
		nvparse(
			"!!RC1.0                                                          \n"
			"    {                                                            \n"
			"       rgb                                                       \n"
			"         {                                                       \n"
			"            spare0 = -half_bias(tex0);                           \n"
			"         }                                                       \n"
			"    }                                                            \n"
			"    out.rgb = spare0;                                            \n"
			"    out.a   = tex0;                                              \n"
			);
		{
		for (char * const * errors = nvparse_get_errors(); *errors; errors++)
			fprintf(stderr, *errors);
		}
		combiner_negate.end_list();



	}
	
	void advance_particles()
	{
        for(unsigned int i=0;  i < particles.size(); i++)
		{
			particle & p = particles[i]; 
			p.x += p.dxdt;
			p.y += p.dydt;
			if(p.x < -1  ||  p.x > 1) 
			{
				p.dxdt *= -1;
				p.x += p.dxdt;
			}
			if(p.y < -1  ||  p.y > 1) 
			{
				p.dydt *= -1;
				p.y += p.dydt;
			}
		}
	}

	void render_particles()
	{
		glBegin(GL_QUADS);
		for(unsigned int i=0;  i < particles.size(); i++)
		{
			particle & p = particles[i]; 
#if 1 // glCopyTexSubImage is flipping y
			glTexCoord2f(0, 0);
			glVertex2f(p.x - p.radius, p.y - p.radius) ;
			glTexCoord2f(0, 1);
			glVertex2f(p.x - p.radius, p.y + p.radius) ;
			glTexCoord2f(1, 1);
			glVertex2f(p.x + p.radius, p.y + p.radius) ;
			glTexCoord2f(1, 0);
			glVertex2f(p.x + p.radius, p.y - p.radius) ;
#else  // or perhaps it was my mistake... :-)
			glTexCoord2f(0, 1);
			glVertex2f(p.x - p.radius, p.y - p.radius) ;
			glTexCoord2f(0, 0);
			glVertex2f(p.x - p.radius, p.y + p.radius) ;
			glTexCoord2f(1, 0);
			glVertex2f(p.x + p.radius, p.y + p.radius) ;
			glTexCoord2f(1, 1);
			glVertex2f(p.x + p.radius, p.y - p.radius) ;
#endif
		}
		glEnd();
	}

}

void update_normalmap_bulges(tex_object_2D & normalmap)
{
	if(! bulges_initialized) initialize_bulges();
	
	advance_particles();


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	glViewport(0, 0, 512, 512);

	glDisable(GL_DEPTH_TEST);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	bulge.disable();

	glColor3f(.5, .5, 1); 

	glBegin(GL_QUADS);
	glVertex2f(-1, -1);	
	glVertex2f(-1,  1);	
	glVertex2f( 1,  1);	
	glVertex2f( 1, -1);	
	glEnd();

	bulge.bind();
	bulge.enable();

	glEnable(GL_REGISTER_COMBINERS_NV);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_LESS, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	// render negative part subtractive
	glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT);
	combiner_negate.call_list();
	render_particles();

	// render positive part additive
	glBlendEquationEXT(GL_FUNC_ADD);
	combiner_identity.call_list();
	render_particles();


	// copy the rendering
	glActiveTextureARB(GL_TEXTURE0_ARB);
    if (!normalmap.is_valid())
    {
        normalmap.bind();
        normalmap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        normalmap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 0, 0, 512, 512, 0);
    }
    else
    {
        normalmap.bind();
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0,   0, 0,    0, 0, 512, 512);
    }
	glDisable(GL_REGISTER_COMBINERS_NV);

	glDisable(GL_BLEND);

	glDisable(GL_ALPHA_TEST);

	glEnable(GL_DEPTH_TEST);

	glViewport(vp[0], vp[1], vp[2], vp[3]);


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glDisable(GL_REGISTER_COMBINERS_NV);

	glMatrixMode(GL_MODELVIEW);


	
}
