#if defined(WIN32)
#include <windows.h>
#endif
#include <glh/glh_extensions.h>
#include <glh/glh_array.h>

#include "cg/cgGL.h"
#include "shared/RenderTexture.h"

using namespace glh;

class DCT_GPU {
public:
    DCT_GPU(int width, int height, bool use_mrt);	// use multiple render targets?
    ~DCT_GPU();

    void LoadImage(array2<unsigned char> &img);
    void LoadImageFloat(array2<float> &img);

    void FDCT(RenderTexture *src, RenderTexture *dest);
    void IDCT(RenderTexture *src, RenderTexture *dest);
    void Quantize(RenderTexture *src, RenderTexture *dest, float quantize_level);

    void DisplayBuffer(RenderTexture *buffer, float brightness = 1.0);

    RenderTexture *GetSrcBuffer() { return img_buffer; }
    RenderTexture *GetDestBuffer() { return dest_buffer; }
    RenderTexture *GetTempBuffer() { return temp_buffer; }

private:
    RenderTexture *CreateBuffer(int w, int h, char *mode);
    void InitCg();
    void DrawQuad(int w, int h, int tw, int th);
    void Pass(CGprogram prog, RenderTexture *src, RenderTexture *src2, RenderTexture *dest);
	void Pass_MRT(CGprogram prog, RenderTexture *src, GLenum src_buffer, GLenum src_buffer2, RenderTexture *dest, GLenum dest_buffer, GLenum dest_buffer2);
    void DisplayTexture(GLuint tex);

    int width, height;

    RenderTexture *img_buffer, *dest_buffer, *temp_buffer;
    RenderTexture *row_buffer, *row_buffer2;
    RenderTexture *col_buffer, *col_buffer2;
    GLuint quantize_tex, dequantize_tex;

    CGprogram dct_rows_fprog, dct_rows2_fprog, dct_unpack_rows_fprog;
    CGprogram dct_cols_fprog, dct_cols2_fprog, dct_unpack_cols_fprog;

    CGprogram idct_rows_fprog, idct_rows2_fprog, idct_unpack_rows_fprog;
    CGprogram idct_cols_fprog, idct_cols2_fprog, idct_unpack_cols_fprog;

    CGprogram display_fprog, quantize_fprog;   
    CGprofile fprog_profile;

    CGparameter quantize_param;
    CGparameter brightness_param;

    GLuint img_tex;
	bool mrt;
};
