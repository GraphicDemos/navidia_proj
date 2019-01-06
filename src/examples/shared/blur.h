



float *generateGaussianWeights(float s, int &n);
const unsigned char *generate1DConvolutionFP(float *weights, int n, bool vertical, bool tex2D, int img_width, int img_height);
const unsigned char *generate1DConvolutionFP_filter(float *weights, int width, bool vertical, bool tex2D, int img_width, int img_height);