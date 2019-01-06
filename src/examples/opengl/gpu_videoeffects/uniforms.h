#ifndef UNIFORMS_H
#define UNIFORMS_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#define UNIFORMT_CG   CGparameter 
#define GETUNIFORM_CG cgGetNamedParameter

#define SETUNIFORM_CG(uniformID, value) if (cgIsParameter(uniformID)) cgGLSetParameter1f(uniformID, value);
#define SETUNIFORM_CG_1I(uniformID, v1) if (cgIsParameter(uniformID)) cgGLSetParameter1d(uniformID, v1);
#define SETUNIFORM_CG_2F(uniformID, v1, v2) if (cgIsParameter(uniformID)) cgGLSetParameter2f(uniformID, v1, v2);

#define SET_UNIFORM_CG_1F(program, subvalue, v1)             cgGLSetParameter1f(program->gUniforms.subvalue, v1)
#define SET_UNIFORM_CG_2F(program, subvalue, v1, v2)         cgGLSetParameter2f(program->gUniforms.subvalue, v1, v2)
#define SET_UNIFORM_CG_3F(program, subvalue, v1, v2, v3)     cgGLSetParameter3f(program->gUniforms.subvalue, v1, v2, v3)
#define SET_UNIFORM_CG_4F(program, subvalue, v1, v2, v3, v4) cgGLSetParameter4f(program->gUniforms.subvalue, v1, v2, v3, v4)
#define SET_UNIFORM_CG_MAT4F(program, subvalue, flag,matrix) cgGLSetParameter4fv(program->gUniforms.subvalue, matrix)
#define SET_UNIFORM_CG_1I(program, subvalue, v1)             cgGLSetParameter1d(program->gUniforms.subvalue, v1)

#define UNIFORMT_GLSL   GLint
#define GETUNIFORM_GLSL glGetUniformLocationARB

#define SETUNIFORM_GLSL(uniformID, value) if (uniformID != -1)        glUniform1fARB(uniformID, value);
#define SETUNIFORM_GLSL_1I(uniformID, v1) if (uniformID != -1)        glUniform1iARB(uniformID, v1);
#define SETUNIFORM_GLSL_2F(uniformID, v1, v2) if (uniformID != -1)    glUniform2fARB(uniformID, v1, v2);
#define SETUNIFORM_GLSL_4FV(uniformID, count, v) if (uniformID != -1) glUniform4fvARB(uniformID, count, v);

#define SET_UNIFORM_GLSL_1F(program, subvalue, v1)             glUniform1fARB(program->gUniforms.subvalue, v1)
#define SET_UNIFORM_GLSL_2F(program, subvalue, v1, v2)         glUniform2fARB(program->gUniforms.subvalue, v1, v2)
#define SET_UNIFORM_GLSL_3F(program, subvalue, v1, v2, v3)     glUniform3fARB(program->gUniforms.subvalue, v1, v2, v3)
#define SET_UNIFORM_GLSL_4F(program, subvalue, v1, v2, v3, v4) glUniform4fARB(program->gUniforms.subvalue, v1, v2, v3, v4)
#define SET_UNIFORM_GLSL_MAT4F(program, subvalue, flag,matrix) glUniformMatrix4fvARB(program->gUniforms.subvalue, 1, flag, matrix);
#define SET_UNIFORM_GLSL_1I(program, subvalue, v1)             glUniform1iARB(program->gUniforms.subvalue, v1)


class UniformsGLSL
{
public:
    void setTextureID(GLuint & texture, int tex_type, int tex_offset);

  	void initPerlinNoiseConstants(const GLhandleARB & mProgram);
    void initUniforms  (const GLhandleARB & mProgram);

    void setTime(float value) { time = value; }
    float getTime() { return time; }
    void updateTime() {
        if (mTime != -1)
            glUniform1fARB(mTime, time);
    }
    void updateSpeed(float value)           { SETUNIFORM_GLSL(mSpeed, value) }
    void updateScanlines(float value)       { SETUNIFORM_GLSL(mScanlines, value) }
    void updateSamples(float value)         { SETUNIFORM_GLSL(mSamples, value) }
    void updateTiles(float value)           { SETUNIFORM_GLSL(mTiles, value) }
    void updateEdgeWidth(float value)       { SETUNIFORM_GLSL(mEdgeWidth, value) }
    void updateUseYUV(int value)			{ SETUNIFORM_GLSL_1I(mUseYUV, value) }

    void setOffsets(float v1, float v2)     { offsets[0] = v1; offsets[1] = v2; }
    void updateOffsets(float v1, float v2)  { SETUNIFORM_GLSL_2F(mDisplacement, v1, v1) }
    void updateDisplacement(float value)    { SETUNIFORM_GLSL(mDisplacement, value) }
    void updateCurrentAngle(float value)    { SETUNIFORM_GLSL(mCurrentAngle, value) }
    void updateRadius(float value)          { SETUNIFORM_GLSL(mRadius, value) }
    void updateEffectScale(float value)     { SETUNIFORM_GLSL(mEffectScale, value) }
    void updateFrequency(float v1, float v2){ SETUNIFORM_GLSL_2F(mFrequency, v1, v2) }
    void updateAmplitude(float v1, float v2){ SETUNIFORM_GLSL_2F(mAmplitude, v1, v2) }
    void updateStartRad(float value)        { SETUNIFORM_GLSL(mStartRad, value) }

    void updateBlurStart(float value)       { SETUNIFORM_GLSL(mBlurStart, value) }
    void updateBlurWidth(float value)       { SETUNIFORM_GLSL(mBlurWidth, value) }
    void updateBlurCenter(float v1, float v2) { SETUNIFORM_GLSL_2F(mBlurCenter, v1, v2) }

    void updateClearDepth(float value)      { SETUNIFORM_GLSL(mClearDepth, value) }
    void updateSceneIntensity(float value)  { SETUNIFORM_GLSL(mSceneIntensity, value) }
    void updateGlowIntensity(float value)   { SETUNIFORM_GLSL(mGlowIntensity, value) }
    void updateHighlightThreshold(float value)  { SETUNIFORM_GLSL(mHighlightThreshold, value) }
    void updateHighlightIntensity(float value)  { SETUNIFORM_GLSL(mHighlightIntensity, value) }
    void updateDownsampleScale(float value) { SETUNIFORM_GLSL(mDownsampleScale, value) }
    void updateBloomBlurWidth(float value)  { SETUNIFORM_GLSL(mBloomBlurWidth, value) };

    void updateDesaturate(float value)      { SETUNIFORM_GLSL(mDesaturate, value) }
    void updateToning(float value)          { SETUNIFORM_GLSL(mToning, value) }
    void updateGlowness(float value)        { SETUNIFORM_GLSL(mGlowness, value) }
    void updateEdgePixelSteps(float value)  { SETUNIFORM_GLSL(mEdgePixelSteps, value) }
    void updateEdgeThreshold(float value)   { SETUNIFORM_GLSL(mEdgeThreshold, value) }

    void updateOpacity(float value)         { SETUNIFORM_GLSL(mOpacity, value) }
    void updateBrushsizestart(float value)  { SETUNIFORM_GLSL(mBrushsizestart, value) }
    void updateBrushsizeend(float value)    { SETUNIFORM_GLSL(mBrushsizeend, value) }
    void updateBrushpressure(float value)   { SETUNIFORM_GLSL(mBrushpressure, value) }
    void updateEffectStrength(float value)  { SETUNIFORM_GLSL(mEffectStrength, value) }
    void updateFadeout(float value)         { SETUNIFORM_GLSL(mFadeout, value) }
    void updateFadetime(float value)        { SETUNIFORM_GLSL(mFadetime, value) }
    void updateFadein(float value)          { SETUNIFORM_GLSL(mFadein, value) }
    void updateFadeintime(float value)      { SETUNIFORM_GLSL(mFadeintime, value) }

    void updateBrightness(float value)      { SETUNIFORM_GLSL(mBrightness, value) }
    void updateContrast(float value)        { SETUNIFORM_GLSL(mContrast, value) }
    void updateHue(float value)             { SETUNIFORM_GLSL(mHue, value) }
    void updateSaturation(float value)      { SETUNIFORM_GLSL(mSaturation, value) }
    void updateExposure(float value)        { SETUNIFORM_GLSL(mExposure, value) }
    void updateGamma(float value)           { SETUNIFORM_GLSL(mGamma, value) }
    void updateDefog(float value)           { SETUNIFORM_GLSL(mDefog, value) }

    void updateOverExposure(float value)    { SETUNIFORM_GLSL(mOverExposure, value) }
    void updateFrameJitter(float value)     { SETUNIFORM_GLSL(mFrameJitter, value) }
    void updateMaxFrameJitter(float value)  { SETUNIFORM_GLSL(mMaxFrameJitter, value) }
    void updateDustAmount(float value)      { SETUNIFORM_GLSL(mDustAmount, value) }
    void updateGrainThickness(float value)  { SETUNIFORM_GLSL(mGrainThickness, value) }
    void updateScratchesAmount(float value) { SETUNIFORM_GLSL(mScratchesAmount, value) }

    void updateGrainAmount(float value)     { SETUNIFORM_GLSL(mGrainAmount, value) }
    void updateScratchesLevel(float value)  { SETUNIFORM_GLSL(mScratchesLevel, value) }

    void updateTexSize(float w, float h)    { SETUNIFORM_GLSL_2F(mTexSize, w, h) }
    void updateWinSize(float w, float h)    { SETUNIFORM_GLSL_2F(mWinSize, w, h) }

public: 
    // For GL, UniformT = GLint
    // For Cg, UniformT = CGparameter
    float time;
    float speed;
    float offsets[2];

	GLuint         _hPermGradTexture;

// Common to both Vertex and Fragment Programs
    UNIFORMT_GLSL  mTime;
    UNIFORMT_GLSL  mSpeed;
    UNIFORMT_GLSL  mScanlines;
    UNIFORMT_GLSL  mSamples;
    UNIFORMT_GLSL  mTiles;
    UNIFORMT_GLSL  mEdgeWidth;
	UNIFORMT_GLSL  mUseYUV;

    UNIFORMT_GLSL  mOffsets;
    UNIFORMT_GLSL  mDisplacement;
    UNIFORMT_GLSL  mCurrentAngle;
    UNIFORMT_GLSL  mRadius;

    UNIFORMT_GLSL  mFrequency;
    UNIFORMT_GLSL  mAmplitude;
    UNIFORMT_GLSL  mStartRad;

	// Vertex Shaders
// Uniforms for vertex light, world position, worldview matrix
    UNIFORMT_GLSL  mWorldViewMatrix;
    UNIFORMT_GLSL  mProjMatrix;
    UNIFORMT_GLSL  mEyePosition;
    UNIFORMT_GLSL  mLightVector;
    UNIFORMT_GLSL  mLightPos;

    UNIFORMT_GLSL  mClearColor;
    UNIFORMT_GLSL  mAmbientColor;
    UNIFORMT_GLSL  mDiffuse;
    UNIFORMT_GLSL  mKd;
    UNIFORMT_GLSL  mKs;
    UNIFORMT_GLSL  mLightColor;
    UNIFORMT_GLSL  mSpecColor;
    UNIFORMT_GLSL  mSpecPower;

    UNIFORMT_GLSL  mTexSize;  // size of texture sampler 
    UNIFORMT_GLSL  mWinSize;  // size of window sampler

// for Pixel Shaders
    UNIFORMT_GLSL  mTexture[4];  // texture sampler
    UNIFORMT_GLSL  mPOW2Texture[4]; // power of 2 texture sampler
    UNIFORMT_GLSL  mSceneMap[4]; // scene sampler (post processing)

    UNIFORMT_GLSL  mHBlurSampler;     // used for blur sampler
    UNIFORMT_GLSL  mFinalBlurSampler; // final blur sampler

    UNIFORMT_GLSL  mBlurWidth;
    UNIFORMT_GLSL  mBlurStart;
    UNIFORMT_GLSL  mBlurCenter;

    UNIFORMT_GLSL  mClearDepth;
    UNIFORMT_GLSL  mSceneIntensity;
    UNIFORMT_GLSL  mGlowColor;
    UNIFORMT_GLSL  mGlowIntensity;
    UNIFORMT_GLSL  mHighlightThreshold;
    UNIFORMT_GLSL  mHighlightIntensity;
    UNIFORMT_GLSL  mDownsampleScale;
    UNIFORMT_GLSL  mBloomBlurWidth;

	UNIFORMT_GLSL  mClipNoise;
	UNIFORMT_GLSL  mColorNoise;
	UNIFORMT_GLSL  mOffset;
	UNIFORMT_GLSL  mPermGradTableParam;

// Variables useful for Painting brush size, opacity, for painting
    UNIFORMT_GLSL  mBrushcolor;
    UNIFORMT_GLSL  mOpacity;
    UNIFORMT_GLSL  mBrushsizestart;
    UNIFORMT_GLSL  mBrushsizeend;
    UNIFORMT_GLSL  mBrushpressure;
    UNIFORMT_GLSL  mEffectStrength;
    UNIFORMT_GLSL  mEffectScale;
    UNIFORMT_GLSL  mFadeout;
    UNIFORMT_GLSL  mFadetime;
    UNIFORMT_GLSL  mFadein;
    UNIFORMT_GLSL  mFadeintime;

// Uniforms for display ProcAmp controls
    UNIFORMT_GLSL  mExposure;
    UNIFORMT_GLSL  mGamma;
    UNIFORMT_GLSL  mDefog;
    UNIFORMT_GLSL  mFogColor;

    UNIFORMT_GLSL  mBrightness;
    UNIFORMT_GLSL  mHue;
    UNIFORMT_GLSL  mSaturation;
    UNIFORMT_GLSL  mContrast;

// PostProcessing effects (sephia)
    UNIFORMT_GLSL  mGlowness;
    UNIFORMT_GLSL  mDesaturate;
    UNIFORMT_GLSL  mToning;
    UNIFORMT_GLSL  mDarkColor;
    UNIFORMT_GLSL  mGrayTransfer;
    UNIFORMT_GLSL  mEdgePixelSteps;
    UNIFORMT_GLSL  mEdgeThreshold;

// Old Camera Effects
    UNIFORMT_GLSL  mOverExpAmount;
    UNIFORMT_GLSL  mOverExposure;
    UNIFORMT_GLSL  mFrameJitter;
    UNIFORMT_GLSL  mMaxFrameJitter;
    UNIFORMT_GLSL  mDustAmount;
    UNIFORMT_GLSL  mGrainThickness;
    UNIFORMT_GLSL  mGrainAmount;
    UNIFORMT_GLSL  mScratchesAmount;
    UNIFORMT_GLSL  mScratchesLevel;
};

class UniformsCG
{
public:
    void setTextureID(  GLuint & tex_dust,
                        GLuint & tex_lines,
                        GLuint & tex_tv,
                        GLuint & tex_noise);

//	void initPerlinNoiseConstants(const GLhandleARB & mProgram);
	void initUniforms(const CGprogram & vertex, const CGprogram & fragment);

    void setTime(float value) { time = value; }
    float getTime() { return time; }
    void updateTime() {
        if (cgIsParameter(mTime))
            cgGLSetParameter1f(mTime, time);
    }
    void updateSpeed(float value)           { SETUNIFORM_CG(mSpeed, value) }
    void updateScanlines(float value)       { SETUNIFORM_CG(mScanlines, value) }
	void updateSamples(float value)         { SETUNIFORM_CG(mSamples, value) }
    void updateTiles(float value)           { SETUNIFORM_CG(mTiles, value) }

    void updateEdgeWidth(float value)       { SETUNIFORM_CG(mEdgeWidth, value) }
    void setOffsets(float v1, float v2)     { offsets[0] = v1; offsets[1] = v2; }
    void updateOffsets(float v1, float v2)  { SETUNIFORM_CG_2F(mDisplacement, v1, v2) }
    void updateUseYUV(int value)			{ SETUNIFORM_CG_1I(mUseYUV, value) };

	void updateDisplacement(float value)    { SETUNIFORM_CG(mDisplacement, value) }
    void updateCurrentAngle(float value)    { SETUNIFORM_CG(mCurrentAngle, value) }
    void updateRadius(float value)          { SETUNIFORM_CG(mRadius, value) }
    void updateEffectScale(float value)     { SETUNIFORM_CG(mEffectScale, value) }
    void updateFrequency(float v1, float v2){ SETUNIFORM_CG_2F(mFrequency, v1, v2) }
    void updateAmplitude(float v1, float v2){ SETUNIFORM_CG_2F(mAmplitude, v1, v2) }
    void updateStartRad(float value)        { SETUNIFORM_CG(mStartRad, value) }

    void updateBlurStart(float value)       { SETUNIFORM_CG(mBlurStart, value) }
    void updateBlurWidth(float value)       { SETUNIFORM_CG(mBlurWidth, value) }
    void updateBlurCenter(float v1, float v2) { SETUNIFORM_CG_2F(mBlurCenter, v1, v2) }

    void updateClearDepth(float value)      { SETUNIFORM_CG(mClearDepth, value) }
    void updateSceneIntensity(float value)  { SETUNIFORM_CG(mSceneIntensity, value) }
    void updateGlowIntensity(float value)   { SETUNIFORM_CG(mGlowIntensity, value) }
    void updateHighlightThreshold(float value)  { SETUNIFORM_CG(mHighlightThreshold, value) }
    void updateHighlightIntensity(float value)  { SETUNIFORM_CG(mHighlightIntensity, value) }
    void updateDownsampleScale(float value) { SETUNIFORM_CG(mDownsampleScale, value) }
    void updateBloomBlurWidth(float value)  { SETUNIFORM_CG(mBloomBlurWidth, value) }

    void updateGlowness(float value)        { SETUNIFORM_CG(mGlowness, value) }
    void updateDesaturate(float value)      { SETUNIFORM_CG(mDesaturate, value) }
    void updateToning(float value)          { SETUNIFORM_CG(mToning, value) }
    void updateEdgePixelSteps(float value)  { SETUNIFORM_CG(mEdgePixelSteps, value) }
    void updateEdgeThreshold(float value)   { SETUNIFORM_CG(mEdgeThreshold, value) }

    void updateOpacity(float value)         { SETUNIFORM_CG(mOpacity, value) }
    void updateBrushsizestart(float value)  { SETUNIFORM_CG(mBrushsizestart, value) }
    void updateBrushsizeend(float value)    { SETUNIFORM_CG(mBrushsizeend, value) }
    void updateBrushpressure(float value)   { SETUNIFORM_CG(mBrushpressure, value) }
    void updateEffectStrength(float value)  { SETUNIFORM_CG(mEffectStrength, value) }
    void updateFadeout(float value)         { SETUNIFORM_CG(mFadeout, value) }
    void updateFadetime(float value)        { SETUNIFORM_CG(mFadetime, value) }
    void updateFadein(float value)          { SETUNIFORM_CG(mFadein, value) }
    void updateFadeintime(float value)      { SETUNIFORM_CG(mFadeintime, value) }

    void updateBrightness(float value)      { SETUNIFORM_CG(mBrightness, value) }
    void updateContrast(float value)        { SETUNIFORM_CG(mContrast, value) }
    void updateHue(float value)             { SETUNIFORM_CG(mHue, value) }
    void updateSaturation(float value)      { SETUNIFORM_CG(mSaturation, value) }
    void updateExposure(float value)        { SETUNIFORM_CG(mExposure, value) }
    void updateGamma(float value)           { SETUNIFORM_CG(mGamma, value) }
    void updateDefog(float value)           { SETUNIFORM_CG(mDefog, value) }

    void updateOverExposure(float value)    { SETUNIFORM_CG(mOverExposure, value) }
    void updateFrameJitter(float value)     { SETUNIFORM_CG(mFrameJitter, value) }
    void updateMaxFrameJitter(float value)  { SETUNIFORM_CG(mMaxFrameJitter, value) }
    void updateDustAmount(float value)      { SETUNIFORM_CG(mDustAmount, value) }
    void updateGrainThickness(float value)  { SETUNIFORM_CG(mGrainThickness, value) }
    void updateScratchesAmount(float value) { SETUNIFORM_CG(mScratchesAmount, value) }

    void updateGrainAmount(float value)     { SETUNIFORM_CG(mGrainAmount, value) }
    void updateScratchesLevel(float value)  { SETUNIFORM_CG(mScratchesLevel, value) }

    void updateTexSize(float w, float h)    { SETUNIFORM_CG_2F(mTexSize, w, h) }
    void updateWinSize(float w, float h)    { SETUNIFORM_CG_2F(mWinSize, w, h) }

public: 
    // For GL, UniformT = GLint
    // For Cg, UniformT = CGparameter
    float time;
    float speed;
    float offsets[2];

// Save a copy of the Fragment/Vertex Shaders
    CGprogram mVertex;
    CGprogram mFragment;

// Common to both Vertex and Fragment Programs
    UNIFORMT_CG  mTime;
    UNIFORMT_CG  mSpeed;
    UNIFORMT_CG  mScanlines;
    UNIFORMT_CG  mSamples;
    UNIFORMT_CG  mTiles;
    UNIFORMT_CG  mEdgeWidth;
	UNIFORMT_CG  mUseYUV;

    UNIFORMT_CG  mOffsets;
    UNIFORMT_CG  mDisplacement;
    UNIFORMT_CG  mCurrentAngle;
    UNIFORMT_CG  mRadius;

    UNIFORMT_CG  mFrequency;
    UNIFORMT_CG  mAmplitude;
    UNIFORMT_CG  mStartRad;

// Vertex Shaders
// Uniforms for vertex light, world position, worldview matrix
    UNIFORMT_CG  mWorldViewMatrix;
    UNIFORMT_CG  mProjMatrix;
    UNIFORMT_CG  mEyePosition;
    UNIFORMT_CG  mLightVector;
    UNIFORMT_CG  mLightPos;

    UNIFORMT_CG  mClearColor;
    UNIFORMT_CG  mAmbientColor;
    UNIFORMT_CG  mDiffuse;
    UNIFORMT_CG  mKd;
    UNIFORMT_CG  mKs;
    UNIFORMT_CG  mLightColor;
    UNIFORMT_CG  mSpecColor;
    UNIFORMT_CG  mSpecPower;

    UNIFORMT_CG  mTexSize;  // size of texture sampler 
    UNIFORMT_CG  mWinSize;  // size of window sampler

// for Pixel Shaders
    UNIFORMT_CG  mTexture[4];  // texture sampler
    UNIFORMT_CG  mPOW2Texture[4]; // power of 2 texture sampler
    UNIFORMT_CG  mSceneMap[4]; // scene sampler (post processing)

    UNIFORMT_CG  mHBlurSampler;     // used for blur sampler
    UNIFORMT_CG  mFinalBlurSampler; // final blur sampler

// Post Processing Bloom
    UNIFORMT_CG  mBlurWidth;
    UNIFORMT_CG  mBlurStart;
    UNIFORMT_CG  mBlurCenter;

    UNIFORMT_CG  mClearDepth;
    UNIFORMT_CG  mSceneIntensity;
    UNIFORMT_CG  mGlowColor;
    UNIFORMT_CG  mGlowIntensity;
    UNIFORMT_CG  mHighlightThreshold;
    UNIFORMT_CG  mHighlightIntensity;
    UNIFORMT_CG  mDownsampleScale;
    UNIFORMT_CG  mBloomBlurWidth;

// PostProcessing effects (sephia)
    UNIFORMT_CG  mDesaturate;
    UNIFORMT_CG  mToning;
    UNIFORMT_CG  mDarkColor;
    UNIFORMT_CG  mGrayTransfer;
    UNIFORMT_CG  mGlowness;
    UNIFORMT_CG  mEdgePixelSteps;
    UNIFORMT_CG  mEdgeThreshold;

// Variables useful for Painting brush size, opacity, for painting
    UNIFORMT_CG  mBrushcolor;
    UNIFORMT_CG  mOpacity;
    UNIFORMT_CG  mBrushsizestart;
    UNIFORMT_CG  mBrushsizeend;
    UNIFORMT_CG  mBrushpressure;
    UNIFORMT_CG  mEffectStrength;
    UNIFORMT_CG  mEffectScale;
    UNIFORMT_CG  mFadeout;
    UNIFORMT_CG  mFadetime;
    UNIFORMT_CG  mFadein;
    UNIFORMT_CG  mFadeintime;

// Uniforms for display ProcAmp controls
    UNIFORMT_CG  mExposure;
    UNIFORMT_CG  mGamma;
    UNIFORMT_CG  mDefog;
    UNIFORMT_CG  mFogColor;

    UNIFORMT_CG  mBrightness;
    UNIFORMT_CG  mHue;
    UNIFORMT_CG  mSaturation;
    UNIFORMT_CG  mContrast;

// Old Camera Effects
    UNIFORMT_CG  mOverExpAmount;
    UNIFORMT_CG  mOverExposure;
    UNIFORMT_CG  mFrameJitter;
    UNIFORMT_CG  mMaxFrameJitter;
    UNIFORMT_CG  mDustAmount;
    UNIFORMT_CG  mGrainThickness;
    UNIFORMT_CG  mGrainAmount;
    UNIFORMT_CG  mScratchesAmount;
    UNIFORMT_CG  mScratchesLevel;
};

#endif