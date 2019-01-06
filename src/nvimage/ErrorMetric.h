
#include "nvimage.h"


namespace nv
{
    class FloatImage;

  NVIMAGE_API float rmsColorError(const FloatImage * ref, const FloatImage * img, bool alphaWeight);
  NVIMAGE_API float rmsAlphaError(const FloatImage * ref, const FloatImage * img);
  NVIMAGE_API
  NVIMAGE_API float cieLabError(const FloatImage * ref, const FloatImage * img);
  NVIMAGE_API float cieLab94Error(const FloatImage * ref, const FloatImage * img);
  NVIMAGE_API float spatialCieLabError(const FloatImage * ref, const FloatImage * img);
  NVIMAGE_API
  NVIMAGE_API float averageColorError(const FloatImage * ref, const FloatImage * img, bool alphaWeight);
  NVIMAGE_API float averageAlphaError(const FloatImage * ref, const FloatImage * img);
  NVIMAGE_API
  NVIMAGE_API float averageAngularError(const FloatImage * img0, const FloatImage * img1);
  NVIMAGE_API float rmsAngularError(const FloatImage * img0, const FloatImage * img1);

} // nv namespace
