#include "uniforms.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>


void UniformsGLSL::setTextureID(GLuint & texture, int tex_type, int tex_offset)
{
    glActiveTexture(GL_TEXTURE0 + tex_offset);
    glBindTexture(tex_type, texture);
    glUniform1iARB(mPOW2Texture[tex_offset-1], tex_offset);
}

#define PERLIN_TABLE_SIZE 8

typedef struct {
	float v[4];
} DOD_Vector4;

#define sfrand() (float)rand()/65535.0f;

void UniformsGLSL::initPerlinNoiseConstants(const GLhandleARB & mProgram)
{
	int i;
	int p[2*PERLIN_TABLE_SIZE+2];                // permutation table
    DOD_Vector4 g[2*PERLIN_TABLE_SIZE+2];		  // gradient table
	
	static bool first_time = true;
	float length;

	srand( (unsigned)1 );

//	if (first_time) 
	{
		mClipNoise          = GETUNIFORM_GLSL(mProgram, "clip_noise");
		mColorNoise         = GETUNIFORM_GLSL(mProgram, "color_noise");
		mPermGradTableParam = GETUNIFORM_GLSL(mProgram, "pg_table_tex");

		SETUNIFORM_GLSL_1I(mClipNoise, 1);
		SETUNIFORM_GLSL_1I(mColorNoise, 1);

		if (mPermGradTableParam != -1) {
			glGenTextures(1, &_hPermGradTexture);
			glBindTexture(GL_TEXTURE_RECTANGLE_NV, _hPermGradTexture);
			glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//			GL_ASSERT_NO_ERROR;
		}

		// initalize random gradients
		for(i=0; i<PERLIN_TABLE_SIZE; i++) {
			p[i] = i;
			g[i].v[0] = sfrand();
			g[i].v[1] = sfrand();
			g[i].v[2] = sfrand();
			length = sqrt(g[i].v[0] * g[i].v[0] + g[i].v[1] * g[i].v[1] + g[i].v[2] * g[i].v[2]);
			g[i].v[0] /= length;
			g[i].v[1] /= length;
			g[i].v[2] /= length;
		}

		// initialize permutation table (random shuffle)
		for(i=0; i<PERLIN_TABLE_SIZE; i++) {
			int j, t;
			j = (rand() >> 4) % PERLIN_TABLE_SIZE;
			t = p[i];
			p[i] = p[j];
			p[j] = t;

			g[i].v[3] = (float) p[i];

			// mirror first half of table into second half
			g[i+PERLIN_TABLE_SIZE].v[0] = g[i].v[0];
			g[i+PERLIN_TABLE_SIZE].v[1] = g[i].v[1];
			g[i+PERLIN_TABLE_SIZE].v[2] = g[i].v[2];
			g[i+PERLIN_TABLE_SIZE].v[3] = g[i].v[3];
		}
		first_time = false;
	}
/*
    SETUNIFORM_GLSL_4FV(mPermGradTableParam, PERLIN_TABLE_SIZE*2, g[0].v);
    SETUNIFORM_GLSL_4FV(mPermGradTableParam + (PERLIN_TABLE_SIZE*2), 1, g[0].v);	// last two entries are for some count/index
    SETUNIFORM_GLSL_4FV(mPermGradTableParam + (PERLIN_TABLE_SIZE*2)+1, 1, g[1].v);
*/
	if (mPermGradTableParam != -1) {
		glTexImage2D(GL_TEXTURE_RECTANGLE_NV,                     0, GL_FLOAT_RGBA16_NV, PERLIN_TABLE_SIZE*2, 1, 0, GL_RGBA, GL_FLOAT, &g[0]);
		glTexImage2D(GL_TEXTURE_RECTANGLE_NV, (PERLIN_TABLE_SIZE*2), GL_FLOAT_RGBA16_NV,                   1, 1, 0, GL_RGBA, GL_FLOAT, &(g[0].v[0]));
		glTexImage2D(GL_TEXTURE_RECTANGLE_NV, (PERLIN_TABLE_SIZE*2)+1, GL_FLOAT_RGBA16_NV,                 1, 1, 0, GL_RGBA, GL_FLOAT, &(g[1].v[0]));
//		GL_ASSERT_NO_ERROR;
	}
}


void UniformsGLSL::initUniforms(const GLhandleARB & mProgram)
{
  char tmp[64];
  int i=0;

// Common to both Vertex and Fragment Programs
    mTime           = GETUNIFORM_GLSL(mProgram, "Time");
    mSpeed          = GETUNIFORM_GLSL(mProgram, "Speed");
    mScanlines      = GETUNIFORM_GLSL(mProgram, "Scanlines");
    mSamples        = GETUNIFORM_GLSL(mProgram, "Samples");
    mTiles          = GETUNIFORM_GLSL(mProgram, "Tiles");
    mEdgeWidth		= GETUNIFORM_GLSL(mProgram, "EdgeWidth");
    mOffsets        = GETUNIFORM_GLSL(mProgram, "Offsets");
    mUseYUV         = GETUNIFORM_GLSL(mProgram, "UseYUV");

	mDisplacement   = GETUNIFORM_GLSL(mProgram, "Displacement");
    mCurrentAngle   = GETUNIFORM_GLSL(mProgram, "CurrentAngle");
    mRadius         = GETUNIFORM_GLSL(mProgram, "Radius");

    mFrequency      = GETUNIFORM_GLSL(mProgram, "Freq");
    mAmplitude      = GETUNIFORM_GLSL(mProgram, "Amplitude");
    mStartRad       = GETUNIFORM_GLSL(mProgram, "StartRad");

// Uniforms for vertex light, world position, worldview matrix
    mWorldViewMatrix = GETUNIFORM_GLSL(mProgram, "worldViewMatrix");
    mProjMatrix      = GETUNIFORM_GLSL(mProgram, "projMatrix");
    mEyePosition     = GETUNIFORM_GLSL(mProgram, "eyePosition");
    mLightVector     = GETUNIFORM_GLSL(mProgram, "lightVector");
    mLightPos        = GETUNIFORM_GLSL(mProgram, "lightPos");

    mClearColor      = GETUNIFORM_GLSL(mProgram, "clearColor");
    mAmbientColor    = GETUNIFORM_GLSL(mProgram, "ambientColor");
    mDiffuse         = GETUNIFORM_GLSL(mProgram, "diffuseColor");
    mKd              = GETUNIFORM_GLSL(mProgram, "Kd");
    mKs              = GETUNIFORM_GLSL(mProgram, "Ks");
    mLightColor      = GETUNIFORM_GLSL(mProgram, "lightColor");
    mSpecColor       = GETUNIFORM_GLSL(mProgram, "specColor");
    mSpecPower       = GETUNIFORM_GLSL(mProgram, "specPower");

    mTexSize         = GETUNIFORM_GLSL(mProgram, "texsize");
    mWinSize         = GETUNIFORM_GLSL(mProgram, "winsize");

    mEffectScale     = GETUNIFORM_GLSL(mProgram, "effectScale");

// for Pixel Shaders
    for (i=0; i< 4; i++) {
        sprintf(tmp, "tex%d\0", i);
        mTexture[i]     = GETUNIFORM_GLSL(mProgram, tmp);
        sprintf(tmp, "pow2tex%d\0", i);
        mPOW2Texture[i] = GETUNIFORM_GLSL(mProgram, tmp);
        sprintf(tmp, "sceneMap%d\0", i);
        mSceneMap[i]    = GETUNIFORM_GLSL(mProgram, tmp);
    }

    mHBlurSampler       = GETUNIFORM_GLSL(mProgram, "HBlurSampler");
    mFinalBlurSampler   = GETUNIFORM_GLSL(mProgram, "FinalBlurSampler");

    mBlurWidth          = GETUNIFORM_GLSL(mProgram, "BlurWidth");
    mBlurStart          = GETUNIFORM_GLSL(mProgram, "BlurStart");
    mBlurCenter         = GETUNIFORM_GLSL(mProgram, "BlurCenter");

    mClearDepth         = GETUNIFORM_GLSL(mProgram, "ClearDepth");
    mSceneIntensity     = GETUNIFORM_GLSL(mProgram, "SceneIntensity");
    mGlowColor          = GETUNIFORM_GLSL(mProgram, "GlowColor");
    mGlowIntensity      = GETUNIFORM_GLSL(mProgram, "GlowIntensity");
    mHighlightThreshold = GETUNIFORM_GLSL(mProgram, "HighlightThreshold");
    mHighlightIntensity = GETUNIFORM_GLSL(mProgram, "HighlightIntensity");
    mDownsampleScale    = GETUNIFORM_GLSL(mProgram, "DownSampleScale");
    mBloomBlurWidth     = GETUNIFORM_GLSL(mProgram, "BloomBlurWidth");


// Variables useful for Painting brush size, opacity, for painting
    mBrushcolor         = GETUNIFORM_GLSL(mProgram, "brushcolor");
    mOpacity            = GETUNIFORM_GLSL(mProgram, "opacity");
    mBrushsizestart     = GETUNIFORM_GLSL(mProgram, "brushsizestart");
    mBrushsizeend       = GETUNIFORM_GLSL(mProgram, "brushsizeend");
    mBrushpressure      = GETUNIFORM_GLSL(mProgram, "brushpressure");
    mEffectStrength     = GETUNIFORM_GLSL(mProgram, "effectstrength");
    mEffectScale        = GETUNIFORM_GLSL(mProgram, "effectstrength");
    mFadeout            = GETUNIFORM_GLSL(mProgram, "fadeout");
    mFadetime           = GETUNIFORM_GLSL(mProgram, "fadetime");
    mFadein             = GETUNIFORM_GLSL(mProgram, "fadein");
    mFadeintime         = GETUNIFORM_GLSL(mProgram, "fadeintime");

// Uniforms for display ProcAmp controls
    mBrightness         = GETUNIFORM_GLSL(mProgram, "brightness");
    mHue                = GETUNIFORM_GLSL(mProgram, "hue");
    mSaturation         = GETUNIFORM_GLSL(mProgram, "saturation");
    mContrast           = GETUNIFORM_GLSL(mProgram, "contrast");

    mExposure           = GETUNIFORM_GLSL(mProgram, "exposure");
    mGamma              = GETUNIFORM_GLSL(mProgram, "gamma");
    mDefog              = GETUNIFORM_GLSL(mProgram, "defog");
    mFogColor           = GETUNIFORM_GLSL(mProgram, "fogColor");

// PostProcessing effects (sephia & edge detection)
    mDesaturate         = GETUNIFORM_GLSL(mProgram, "desaturate");
    mToning             = GETUNIFORM_GLSL(mProgram, "toning");
    mDarkColor          = GETUNIFORM_GLSL(mProgram, "darkColor");
    mGrayTransfer       = GETUNIFORM_GLSL(mProgram, "grayTransfer");

    mGlowness           = GETUNIFORM_GLSL(mProgram, "Glowness");
    mEdgePixelSteps     = GETUNIFORM_GLSL(mProgram, "NPixels");
    mEdgeThreshold      = GETUNIFORM_GLSL(mProgram, "Threshold");

// Old Camera Effects
    mOverExposure       = GETUNIFORM_GLSL(mProgram, "OverExposure");
    mFrameJitter        = GETUNIFORM_GLSL(mProgram, "FrameJitter");
    mMaxFrameJitter     = GETUNIFORM_GLSL(mProgram, "MaxFrameJitter");
    mDustAmount         = GETUNIFORM_GLSL(mProgram, "DustAmount");
    mGrainThickness     = GETUNIFORM_GLSL(mProgram, "GrainThickness");
    mScratchesAmount    = GETUNIFORM_GLSL(mProgram, "ScratchesAmount");

    mGrainAmount        = GETUNIFORM_GLSL(mProgram, "GrainAmount");
    mScratchesLevel     = GETUNIFORM_GLSL(mProgram, "ScratchesLevel");

// Initialize Perlin Noise Constants
	initPerlinNoiseConstants(mProgram);
}

void UniformsCG::setTextureID(GLuint & tex_dust,
                              GLuint & tex_lines,
                              GLuint & tex_tv,
                              GLuint & tex_noise)
{
    mPOW2Texture[0] = GETUNIFORM_CG(mFragment, "pow2tex0");
    cgGLEnableTextureParameter(mPOW2Texture[0]);
    cgGLSetTextureParameter(mPOW2Texture[0], tex_dust);

    mPOW2Texture[1] = GETUNIFORM_CG(mFragment, "pow2tex1");
    cgGLEnableTextureParameter(mPOW2Texture[1]);
    cgGLSetTextureParameter(mPOW2Texture[1], tex_lines);

    mPOW2Texture[2] = GETUNIFORM_CG(mFragment, "pow2tex2");
    cgGLEnableTextureParameter(mPOW2Texture[2]);
    cgGLSetTextureParameter(mPOW2Texture[2], tex_tv);

    mPOW2Texture[3] = GETUNIFORM_CG(mFragment, "pow2tex3");
    cgGLEnableTextureParameter(mPOW2Texture[3]);
    cgGLSetTextureParameter(mPOW2Texture[3], tex_noise);

}

void UniformsCG::initUniforms(const CGprogram & vertex, const CGprogram & fragment)
{
  char tmp[64];
  int i=0;

  mVertex = vertex;
  mFragment = fragment;

// Common to both Vertex and Fragment Programs
    mTime            = GETUNIFORM_CG(fragment, "Time");
    mSpeed           = GETUNIFORM_CG(vertex,   "Speed");
    mScanlines       = GETUNIFORM_CG(vertex,   "Scanlines");
    mSamples         = GETUNIFORM_CG(fragment, "Samples");
    mTiles           = GETUNIFORM_CG(fragment, "Tiles");
    mEdgeWidth		 = GETUNIFORM_CG(fragment, "EdgeWidth");
    mOffsets         = GETUNIFORM_CG(fragment, "Offsets");
    mUseYUV          = GETUNIFORM_CG(fragment, "UseYUV");

    mDisplacement    = GETUNIFORM_CG(fragment, "Displacement");
    mCurrentAngle    = GETUNIFORM_CG(fragment, "CurrentAngle");
    mRadius          = GETUNIFORM_CG(fragment, "Radius");

    mFrequency       = GETUNIFORM_CG(fragment, "Freq");
    mAmplitude       = GETUNIFORM_CG(fragment, "Amplitude");
    mStartRad        = GETUNIFORM_CG(fragment, "StartRad");

// Uniforms for vertex light, world position, worldview matrix
    mWorldViewMatrix = GETUNIFORM_CG(vertex, "worldViewMatrix");
    mProjMatrix      = GETUNIFORM_CG(vertex, "projMatrix");
    mEyePosition     = GETUNIFORM_CG(vertex, "eyePosition");
    mLightVector     = GETUNIFORM_CG(vertex, "lightVector");
    mLightPos        = GETUNIFORM_CG(vertex, "lightPos");

    mClearColor      = GETUNIFORM_CG(vertex, "clearColor");
    mAmbientColor    = GETUNIFORM_CG(vertex, "ambientColor");
    mDiffuse         = GETUNIFORM_CG(vertex, "diffuseColor");
    mKd              = GETUNIFORM_CG(vertex, "Kd");
    mKs              = GETUNIFORM_CG(vertex, "Ks");
    mLightColor      = GETUNIFORM_CG(vertex, "lightColor");
    mSpecColor       = GETUNIFORM_CG(vertex, "specColor");
    mSpecPower       = GETUNIFORM_CG(vertex, "specPower");

    mTexSize         = GETUNIFORM_CG(fragment, "texsize");
    mWinSize         = GETUNIFORM_CG(fragment, "winsize");

    mEffectScale     = GETUNIFORM_CG(fragment, "effectScale");


// for Pixel Shaders
    for (i=0; i< 4; i++) {
        sprintf(tmp, "tex%d\0", i);
        mTexture[i]     = GETUNIFORM_CG(fragment, tmp);
        sprintf(tmp, "sceneMap%d\0", i);
        mSceneMap[i]    = GETUNIFORM_CG(fragment, tmp);
    }

    mHBlurSampler       = GETUNIFORM_CG(fragment, "HBlurSampler");
    mFinalBlurSampler   = GETUNIFORM_CG(fragment, "FinalBlurSampler");

    mBlurWidth          = GETUNIFORM_CG(fragment, "BlurWidth");
    mBlurStart          = GETUNIFORM_CG(fragment, "BlurStart");
    mBlurCenter         = GETUNIFORM_CG(fragment, "BlurCenter");

    mClearDepth         = GETUNIFORM_CG(fragment, "ClearDepth");
    mSceneIntensity     = GETUNIFORM_CG(fragment, "SceneIntensity");
    mGlowColor          = GETUNIFORM_CG(fragment, "GlowColor");
    mGlowIntensity      = GETUNIFORM_CG(fragment, "GlowIntensity");
    mHighlightThreshold = GETUNIFORM_CG(fragment, "HighlightThreshold");
    mHighlightIntensity = GETUNIFORM_CG(fragment, "HighlightIntensity");
    mDownsampleScale    = GETUNIFORM_CG(fragment, "DownSampleScale");
    mBloomBlurWidth     = GETUNIFORM_CG(fragment, "BloomBlurWidth");


// Variables useful for Painting brush size, opacity, for painting
    mBrushcolor         = GETUNIFORM_CG(fragment, "brushcolor");
    mOpacity            = GETUNIFORM_CG(fragment, "opacity");
    mBrushsizestart     = GETUNIFORM_CG(fragment, "brushsizestart");
    mBrushsizeend       = GETUNIFORM_CG(fragment, "brushsizeend");
    mBrushpressure      = GETUNIFORM_CG(fragment, "brushpressure");
    mEffectStrength     = GETUNIFORM_CG(fragment, "effectstrength");
    mFadeout            = GETUNIFORM_CG(fragment, "fadeout");
    mFadetime           = GETUNIFORM_CG(fragment, "fadetime");
    mFadein             = GETUNIFORM_CG(fragment, "fadein");
    mFadeintime         = GETUNIFORM_CG(fragment, "fadeintime");

// Uniforms for display ProcAmp controls
    mBrightness         = GETUNIFORM_CG(vertex, "brightness");
    mHue                = GETUNIFORM_CG(vertex, "hue");
    mSaturation         = GETUNIFORM_CG(vertex, "saturation");
    mContrast           = GETUNIFORM_CG(vertex, "contrast");

    mExposure           = GETUNIFORM_CG(fragment, "exposure");
    mGamma              = GETUNIFORM_CG(fragment, "gamma");
    mDefog              = GETUNIFORM_CG(fragment, "defog");
    mFogColor           = GETUNIFORM_CG(fragment, "fogColor");

// PostProcessing effects (sephia)
    mDesaturate         = GETUNIFORM_CG(fragment, "desaturate");
    mToning             = GETUNIFORM_CG(fragment, "toning");
    mDarkColor          = GETUNIFORM_CG(fragment, "darkColor");
    mGrayTransfer       = GETUNIFORM_CG(fragment, "grayTransfer");
    mGlowness           = GETUNIFORM_CG(fragment, "Glowness");
    mEdgePixelSteps     = GETUNIFORM_CG(fragment, "NPixels");
    mEdgeThreshold      = GETUNIFORM_CG(fragment, "Threshold");

// Old Camera Effects
    mOverExposure       = GETUNIFORM_CG(vertex, "OverExposure");
    mFrameJitter        = GETUNIFORM_CG(vertex, "FrameJitter");
    mMaxFrameJitter     = GETUNIFORM_CG(vertex, "MaxFrameJitter");
    mDustAmount         = GETUNIFORM_CG(vertex, "DustAmount");
    mGrainThickness     = GETUNIFORM_CG(vertex, "GrainThickness");
    mScratchesAmount    = GETUNIFORM_CG(vertex, "ScratchesAmount");

    mGrainAmount        = GETUNIFORM_CG(fragment, "GrainAmount");
    mScratchesLevel     = GETUNIFORM_CG(fragment, "ScratchesLevel");
}