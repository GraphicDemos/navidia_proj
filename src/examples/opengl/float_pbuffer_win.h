class CFPBuffer
{
    public:
        CFPBuffer() 
          : width(0), height(0), handle(NULL), hDC(NULL), hRC(NULL), 
            hWindowDC(NULL), hWindowRC(NULL), valid(false)
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
            hWindowDC = wglGetCurrentDC();
            hWindowRC = wglGetCurrentContext();
    
            int format = 0;
            unsigned int nformats;
            int attribList[] = 
            {
                WGL_RED_BITS_ARB,               32,
                WGL_GREEN_BITS_ARB,             32,
                WGL_BLUE_BITS_ARB,              32,
                WGL_ALPHA_BITS_ARB,             32,
                WGL_STENCIL_BITS_ARB,           8,
                WGL_DEPTH_BITS_ARB,             24,
                WGL_FLOAT_COMPONENTS_NV,        true,
                WGL_DRAW_TO_PBUFFER_ARB,        true,
                0,
            };
        
            wglChoosePixelFormatARB(hWindowDC, attribList, NULL, 1, &format, &nformats);
            if (nformats == 0)
            {
                printf("Unable to find any RGBA32 floating point pixel formats\n");
                return false;
            }
        
            // clear attribute list
            attribList[0] = 0;
        
            handle = wglCreatePbufferARB(hWindowDC, format, width, height, attribList);
            if (handle == NULL) 
            {
                printf("Unable to create floating point pbuffer (wglCreatePbufferARB failed)\n");
                return false;
            }
        
            hDC = wglGetPbufferDCARB(handle);
            if (hDC == NULL) 
            {
                printf("Unable to retrieve handle to pbuffer device context\n");
                return false;
            }
        
            hRC = wglCreateContext(hDC);
            if (hRC == NULL) 
            {
                printf("Unable to create a rendering context for the pbuffer\n");
                return false;
            }    
            
            if (!wglShareLists(hWindowRC, hRC)) 
            {
                printf("Unable to share data between rendering contexts\n");
                return false;
            }
        
            valid = true;
    
            return true;
        }
    
        void activate()
        {
            wglMakeCurrent(hDC, hRC);
        }
    
        void deactivate()
        {
            wglMakeCurrent(hWindowDC, hWindowRC);
        }
    
        void destroy()
        {
            if (valid)
            {
                // Delete pbuffer and related
                wglDeleteContext(hRC);
                wglReleasePbufferDCARB(handle, hDC);
                wglDestroyPbufferARB(handle);
        
                // Return to the normal context
                wglMakeCurrent(hWindowDC, hWindowRC);
    
                width = 0;
                height = 0;
                handle = NULL;
                hDC = NULL;
                hRC = NULL;
                hWindowDC = NULL;
                hWindowRC = NULL;
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

        HPBUFFERARB handle;
        HDC hDC;
        HGLRC hRC;

        HDC hWindowDC;
        HGLRC hWindowRC;

        bool valid;
};
