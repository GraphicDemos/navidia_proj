#pragma once

#define DECODER_DOES_RENDER 0

#define EXR_IMAGE_PATH      "../../../../MEDIA/textures/hdr/"
#define MISC_IMAGE_PATH     "../../../../MEDIA/textures/2D/"
#define VIDEO_FILE_PATH     "../../../../MEDIA/Textures/video/"
#define SHADER_PATH         "../../../../MEDIA/programs/"

#define MIN_WIDTH 760

#define USE_TEXTURE_2D 0

#if USE_TEXTURE_2D
#define TEXTURE_AR ImageSource::GL_TEX_2D
#else
#define TEXTURE_AR ImageSource::GL_TEX_RECT
#endif

