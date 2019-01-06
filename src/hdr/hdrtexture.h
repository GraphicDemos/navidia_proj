

#if NVHDR_DLL

#if defined _WIN32 || defined WIN32 || defined __NT__ || defined __WIN32__ || defined __MINGW32__
#  ifdef NVHDR_EXPORTS
#    define NVHDR_API __declspec(dllexport)
#  else
#    define NVHDR_API __declspec(dllimport)
#  endif
#endif

#if defined __GNUC__ >= 4
#  ifdef NVHDR_EXPORTS
#    define NVHDR_API __attribute__((visibility("default")))
#  endif
#endif

#endif //  

#if !defined NVHDR_API
#  define NVHDR_API
#endif

class NVHDR_API HDRtexture {
public:
  HDRtexture(char *filename);
  ~HDRtexture();

  unsigned char *GetPixel(int x, int y);
  float *GetPixelHilo(int x, int y);

  GLuint Create2DTextureRGBE(GLenum target);
  GLuint Create2DTextureHILO(GLenum target, bool rg);
  GLuint CreateCubemap(GLenum format);
  GLuint CreateCubemapRGBE();
  GLuint CreateCubemapHILO(bool rg);

  int GetWidth() { return m_width; };
  int GetHeight() { return m_height; };

  bool IsValid() { return m_valid; };

  void Analyze();
  void ConvertHILO();

  bool m_valid;
  int m_width, m_height;
  unsigned char *m_data;
  float *m_hilodata;

  GLenum m_target;

  float m_max_r, m_max_g, m_max_b;
  float m_min_r, m_min_g, m_min_b;
  float m_max;
};
