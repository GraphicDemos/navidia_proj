#include "dct_gpu.h"
#include "shared/read_text_file.h"

CGcontext g_context;

DCT_GPU::DCT_GPU(int w, int h, bool use_mrt) : width(w), height(h), mrt(use_mrt)
{
    char *format = "float=16 rgba textureRECT";
//    char *format = "float=32 rgba textureRECT";

	// create buffers
    img_buffer = CreateBuffer(w, h, format);
    dest_buffer = CreateBuffer(w, h, format);
    temp_buffer = CreateBuffer(w, h, format);

    row_buffer = CreateBuffer(w/8, h, format);
    if (!mrt) row_buffer2 = CreateBuffer(w/8, h, format);
    col_buffer = CreateBuffer(w, h/8, format);
    if (!mrt) col_buffer2 = CreateBuffer(w, h/8, format);

    InitCg();
}

DCT_GPU::~DCT_GPU()
{
    delete img_buffer;
    delete row_buffer;
	if (!mrt) delete row_buffer2;
    delete col_buffer;
	if (!mrt) delete col_buffer2;
}

RenderTexture *
DCT_GPU::CreateBuffer(int w, int h, char *mode)
{
    RenderTexture *buffer = new RenderTexture(mode, w, h, GL_TEXTURE_RECTANGLE_NV);
    buffer->Activate();

    glDisable(GL_DITHER);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, 0.0, h, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, w, h);

    buffer->Deactivate();
    return buffer;
}

void 
cgErrorCallback()
{
    CGerror lastError = cgGetError();
    if(lastError) {
        printf("Cg error:\n");
        printf("%s\n\n", cgGetErrorString(lastError));
        printf("%s\n", cgGetLastListing(g_context));
        cgDestroyContext(g_context);
        exit(-1);
    }
}

void
DCT_GPU::InitCg()
{
    cgSetErrorCallback(cgErrorCallback);
    g_context = cgCreateContext();

    const char *code = read_text_file("gpgpu_dct/dct.cg");
    fprog_profile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

	if (mrt) {
		dct_rows_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "DCT_rows_singlepass_PS", NULL);
	} else {
		dct_rows_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "DCT_rows_pass1_PS", NULL);
		dct_rows2_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "DCT_rows_pass2_PS", NULL);
	}
    dct_unpack_rows_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "DCT_unpack_rows_PS", NULL);

	if (mrt) {
		dct_cols_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "DCT_cols_singlepass_PS", NULL);
	} else {
		dct_cols_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "DCT_cols_pass1_PS", NULL);
		dct_cols2_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "DCT_cols_pass2_PS", NULL);
	}
    dct_unpack_cols_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "DCT_unpack_cols_PS", NULL);

	if (mrt) {
		idct_rows_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "IDCT_rows_singlepass_PS", NULL);
	} else {
		idct_rows_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "IDCT_rows_pass1_PS", NULL);
		idct_rows2_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "IDCT_rows_pass2_PS", NULL);
	}
	idct_unpack_rows_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "IDCT_unpack_rows_PS", NULL);

	if (mrt) {
		idct_cols_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "IDCT_cols_singlepass_PS", NULL);
	} else {
		idct_cols_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "IDCT_cols_pass1_PS", NULL);
		idct_cols2_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "IDCT_cols_pass2_PS", NULL);
	}
    idct_unpack_cols_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "IDCT_unpack_cols_PS", NULL);

    display_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "Display_PS", NULL);
    brightness_param = cgGetNamedParameter(display_fprog, "brightness");

	quantize_fprog = cgCreateProgram(g_context, CG_SOURCE, code, fprog_profile, "Quantize_PS", NULL);
    quantize_param = cgGetNamedParameter(quantize_fprog, "quantize_level");

    cgGLLoadProgram(dct_rows_fprog);
    if (!mrt) cgGLLoadProgram(dct_rows2_fprog);
    cgGLLoadProgram(dct_unpack_rows_fprog);
    cgGLLoadProgram(dct_cols_fprog);
    if (!mrt) cgGLLoadProgram(dct_cols2_fprog);
    cgGLLoadProgram(dct_unpack_cols_fprog);

    cgGLLoadProgram(idct_rows_fprog);
    if (!mrt) cgGLLoadProgram(idct_rows2_fprog);
    cgGLLoadProgram(idct_unpack_rows_fprog);
    cgGLLoadProgram(idct_cols_fprog);
    if (!mrt) cgGLLoadProgram(idct_cols2_fprog);
    cgGLLoadProgram(idct_unpack_cols_fprog);

    cgGLLoadProgram(display_fprog);
    cgGLLoadProgram(quantize_fprog);

    delete [] code;
}

void
DCT_GPU::LoadImage(array2<unsigned char> &img)
{
    glGenTextures(1, &img_tex);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, img_tex);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, img.get_width(), img.get_height(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, (const void *)img.get_pointer());

    img_buffer->Activate();
    DisplayTexture(img_tex);
    img_buffer->Deactivate();
}

void
DCT_GPU::LoadImageFloat(array2<float> &img)
{
    glGenTextures(1, &img_tex);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, img_tex);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_R32_NV, img.get_width(), img.get_height(), 0, GL_LUMINANCE, GL_FLOAT, (const void *)img.get_pointer());

    img_buffer->Activate();
    DisplayTexture(img_tex);
    img_buffer->Deactivate();
}

void
DCT_GPU::DrawQuad(int w, int h, int tw, int th)
{
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0); glVertex2i(0, 0);
    glTexCoord2i(tw, 0); glVertex2i(w, 0);
    glTexCoord2i(tw, th); glVertex2i(w, h);
    glTexCoord2i(0, th); glVertex2i(0, h);
    glEnd();
}

// display contents of buffer to screen
void
DCT_GPU::DisplayBuffer(RenderTexture *buffer, float brightness)
{
    cgGLBindProgram(display_fprog);
    cgGLSetParameter1f(brightness_param, brightness);
    cgGLEnableProfile(fprog_profile);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    buffer->Bind();
    DrawQuad(width, height, buffer->GetWidth(), buffer->GetHeight());
    buffer->Release();
    cgGLDisableProfile(fprog_profile);
}

void
DCT_GPU::DisplayTexture(GLuint tex)
{
    cgGLBindProgram(display_fprog);
    cgGLEnableProfile(fprog_profile);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex);
    DrawQuad(width, height, width, height);
    cgGLDisableProfile(fprog_profile);
}

// perform a rendering pass with given fragment program and source and destination buffers
void
DCT_GPU::Pass(CGprogram prog, RenderTexture *src, RenderTexture *src2, RenderTexture *dest)
{
    dest->Activate();
    cgGLBindProgram(prog);
    cgGLEnableProfile(fprog_profile);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    src->Bind();
    if (src2) {
        glActiveTextureARB(GL_TEXTURE1_ARB);
        src2->Bind();
    }
    DrawQuad(dest->GetWidth(), dest->GetHeight(), width, height);
    src->Release();
    if (src2) src2->Release();
    cgGLDisableProfile(fprog_profile);
    dest->Deactivate();
}

// perform a rendering pass with given fragment program and source and destination buffers, using multiple render targets
void
DCT_GPU::Pass_MRT(CGprogram prog, RenderTexture *src, GLenum src_buffer, GLenum src_buffer2, RenderTexture *dest, GLenum dest_buffer, GLenum dest_buffer2)
{
    dest->Activate();

	GLenum buffers[] = {
		dest_buffer,
		dest_buffer2,
	};
	if (dest_buffer2) {
		glDrawBuffersATI(2, buffers);
	} else {
		glDrawBuffer(dest_buffer);
	}

    cgGLBindProgram(prog);
    cgGLEnableProfile(fprog_profile);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    src->Bind(src_buffer);
    if (src_buffer2) {
        glActiveTextureARB(GL_TEXTURE1_ARB);
        src->Bind(src_buffer2);
    }
    DrawQuad(dest->GetWidth(), dest->GetHeight(), width, height);
    src->Release(src_buffer);
    if (src_buffer2) src->Release(src_buffer2);
    cgGLDisableProfile(fprog_profile);

	glDrawBuffer(GL_FRONT);	
	dest->Deactivate();
}

// forward DCT
void
DCT_GPU::FDCT(RenderTexture *src, RenderTexture *dest)
{
	if (mrt) {
		Pass_MRT(dct_rows_fprog, src, WGL_FRONT_LEFT_ARB, 0, row_buffer, GL_AUX0, GL_AUX1);
		Pass_MRT(dct_unpack_rows_fprog, row_buffer, WGL_AUX0_ARB, WGL_AUX1_ARB, dest, GL_FRONT_LEFT, 0);

		Pass_MRT(dct_cols_fprog, dest, WGL_FRONT_LEFT_ARB, 0, col_buffer, GL_AUX0, GL_AUX1);
		Pass_MRT(dct_unpack_cols_fprog, col_buffer, WGL_AUX0_ARB, WGL_AUX1_ARB, dest, GL_FRONT_LEFT, 0);
	} else {
		Pass(dct_rows_fprog, src, 0, row_buffer);
		Pass(dct_rows2_fprog, src, 0, row_buffer2);
		Pass(dct_unpack_rows_fprog, row_buffer, row_buffer2, dest);

		Pass(dct_cols_fprog, dest, 0, col_buffer);
		Pass(dct_cols2_fprog, dest, 0, col_buffer2);
		Pass(dct_unpack_cols_fprog, col_buffer, col_buffer2, dest);
	}
}

// inverse DCT
void
DCT_GPU::IDCT(RenderTexture *src, RenderTexture *dest)
{
	if (mrt) {
		Pass_MRT(idct_rows_fprog, src, WGL_FRONT_LEFT_ARB, 0, row_buffer, GL_AUX0, GL_AUX1);
		Pass_MRT(idct_unpack_rows_fprog, row_buffer, WGL_AUX0_ARB, WGL_AUX1_ARB, dest, GL_FRONT_LEFT, 0);

		Pass_MRT(idct_cols_fprog, dest, WGL_FRONT_LEFT_ARB, 0, col_buffer, GL_AUX0, GL_AUX1);
		Pass_MRT(idct_unpack_cols_fprog, col_buffer, WGL_AUX0_ARB, WGL_AUX1_ARB, dest, GL_FRONT_LEFT, 0);
	} else {
		Pass(idct_cols_fprog, src, 0, col_buffer);
		Pass(idct_cols2_fprog, src, 0, col_buffer2);
		Pass(idct_unpack_cols_fprog, col_buffer, col_buffer2, dest);

		Pass(idct_rows_fprog, dest, 0, row_buffer);
		Pass(idct_rows2_fprog, dest, 0, row_buffer2);
		Pass(idct_unpack_rows_fprog, row_buffer, row_buffer2, dest);
	}
}

void
DCT_GPU::Quantize(RenderTexture *src, RenderTexture *dest, float quantize_level)
{
    cgGLSetParameter1f(quantize_param, quantize_level);
    Pass(quantize_fprog, src, 0, dest);
}