class CFPBuffer
{
    public:
        CFPBuffer() 
          : width(0), height(0), m_pDisplay(0), m_glxPbuffer(0), m_glxContext(0),
            m_pOldDisplay(0), m_glxOldDrawable(0), m_glxOldContext(0), valid(false)
        { }

        bool create(int w, int h)
        {
            // clean up pbuffer resources if already created
            if (valid)
                destroy();
    
            width = w;
            height = h;
            
            // get a copy of the main windows device context and rendering
            // context
            
            m_pOldDisplay = glXGetCurrentDisplay();
            m_glxOldDrawable = glXGetCurrentDrawable();
            m_glxOldContext = glXGetCurrentContext();
            int iScreen = DefaultScreen(m_pOldDisplay);
    
            GLXFBConfig *glxConfig;
            int iConfigCount;   
            
            int pfAttribList[] = 
            {
                GLX_RED_SIZE,               32,
                GLX_GREEN_SIZE,             32,
                GLX_BLUE_SIZE,              32,
                GLX_ALPHA_SIZE,             32,
                GLX_STENCIL_SIZE,           8,
                GLX_DEPTH_SIZE,             24,
                GLX_FLOAT_COMPONENTS_NV,    true,
                GLX_DRAWABLE_TYPE,          GLX_PBUFFER_BIT,
                0,
            };
    
            glxConfig = glXChooseFBConfigSGIX(m_pOldDisplay, iScreen, pfAttribList, &iConfigCount);
            if (!glxConfig)
            {
                fprintf(stderr, "pbuffer creation error:  glXChooseFBConfigSGIX() failed\n");
                return false;
            }
            
            int pbAttribList[] = 
            {
                GLX_LARGEST_PBUFFER, true,
                GLX_PRESERVED_CONTENTS, true,
                0,
            };
    
            m_glxPbuffer = glXCreateGLXPbufferSGIX(m_pOldDisplay, glxConfig[0], width, height, pbAttribList);
    
            if (!m_glxPbuffer)
            {
                fprintf(stderr, "pbuffer creation error:  glXCreatePbufferSGIX() failed\n");
                return false;
            }
    
            m_glxContext = glXCreateContextWithConfigSGIX(m_pOldDisplay, glxConfig[0], GLX_RGBA_TYPE, m_glxOldContext, true);
            if (!glxConfig)
            {
                fprintf(stderr, "pbuffer creation error:  glXCreateContextWithConfigSGIX() failed\n");
                return false;
            }
            
            m_pDisplay = m_pOldDisplay;
            
            valid = true;
            
            return true;
        }
    
        void activate()
        {
            if (valid)
                glXMakeCurrent(m_pDisplay, m_glxPbuffer, m_glxContext);
        }
    
        void deactivate()
        {
            if (valid)
                glXMakeCurrent(m_pOldDisplay, m_glxOldDrawable, m_glxOldContext);
        }
    
        void destroy()
        {
            if (valid)
            {
                glXDestroyContext(m_pDisplay, m_glxContext);
                glXDestroyGLXPbufferSGIX(m_pDisplay, m_glxPbuffer);        
        
                // Return to the normal context
                glXMakeCurrent(m_pOldDisplay, m_glxOldDrawable, m_glxOldContext);
    
                width = 0;
                height = 0;
                
                m_pDisplay = 0;
                m_glxPbuffer = 0;
                m_glxContext = 0;

                m_pOldDisplay = 0;
                m_glxOldDrawable = 0;
                m_glxOldContext = 0;
                
                valid = false;
            }
        }
    
        int getWidth()
        { return width; }

        int getHeight()
        { return height; }

        ~CFPBuffer()
        { }
    
    private:
        int width;
        int height;

        Display    *m_pDisplay;
        GLXPbuffer  m_glxPbuffer;
        GLXContext  m_glxContext;

        Display    *m_pOldDisplay;
        GLXPbuffer  m_glxOldDrawable;
        GLXContext  m_glxOldContext;

        bool valid;
};
