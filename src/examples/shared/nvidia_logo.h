#ifndef NVIDIA_LOGO
#define NVIDIA_LOGO
/// \file nvidia_logo.h
#include <shared/array_texture.h>
#include <shared/nv_png.h>
#include <glh/glh_obs.h>

/************************************************************************/ /**
 * This class is a very simple way to add a logo (NVidia logo) in front
 * of the screen. Commercial people always argue that logos are missing
 * in standalone demos...
 */
class CNVLogo
{
private:
	const char *m_logoname;
	glh::tex_object_2D m_logomap;
	int m_w,m_h;
public:
	CNVLogo(const char * logoname=NULL)
	{
		if(!logoname) 
			m_logoname = "nvlogo_ue.png";
	}
#ifdef EBNOLOGO
	// if we're compiling for Effect Browser
	void render_logo(float scale=0.5, float alpha=0.5, bool left=true, bool top=true) {}
	bool init(const char * logoname=NULL, const char * modhandlename=NULL) { return true; }
#else
	bool init(const char * logoname=NULL, const char * modhandlename=NULL)
	{
		glh::array2<glh::vec4ub> rgba_map;
		if (logoname) 
			m_logoname = logoname;
#ifdef WIN32
		if(modhandlename) // in case of resource : PNG picture can be insite the exe or dll file
		{
			unsigned long hModule = (unsigned long)GetModuleHandle(modhandlename);
			set_png_module_handle(hModule);
		}
		set_png_module_restypename("PNG");
#endif		

		m_logomap.enable();
		m_logomap.bind();
		read_png_rgba(m_logoname, rgba_map);
		m_w = rgba_map.get_width();
		m_h = rgba_map.get_height();
		make_rgba_texture(rgba_map, true);
		m_logomap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		m_logomap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		return true;
	}
	void render_logo(float scale=0.5, float alpha=0.5, bool left=true, bool top=true)
	{
		GLint vp[4];
		glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT|GL_LIGHTING_BIT);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_Q);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(glActiveTextureARB)
		{
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		m_logomap.enable();
		m_logomap.bind();

		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glGetIntegerv(GL_VIEWPORT, vp);
		glLoadIdentity();
		gluOrtho2D(vp[0], vp[2], vp[1], vp[3]);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		float x,y;
		x = left ? 0 : vp[2]-m_w*scale;
		y = top ? vp[3]-m_h*scale : 0;
		glColor4f(1,1,1, alpha);
		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex3f(x,y,0);
		glTexCoord2f(1,0);
		glVertex3f(x+m_w*scale,y,0);
		glTexCoord2f(1,1);
		glVertex3f(x+m_w*scale,y+m_h*scale,0);
		glTexCoord2f(0,1);
		glVertex3f(x+0,y+m_h*scale,0);
		glEnd();
		glPopAttrib();

		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		m_logomap.disable();
		glBindTexture(GL_TEXTURE_2D, 0);

	}
#endif
};

#endif
