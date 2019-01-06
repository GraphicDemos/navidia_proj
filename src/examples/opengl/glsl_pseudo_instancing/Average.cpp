
#include "Average.h"


Average::Average(void)
{
  reset();
}

void Average::addSample(float sample)
{
  mSum += sample;
  mCount++;
  mAverage = mSum / float(mCount);
}

float Average::average(void) const
{
  return(mAverage);
}

int Average::count(void) const
{
  return(mCount);
}

void Average::reset(void)
{
  mSum = 0.0f;
  mAverage = 0.0f;
  mCount = 0;  
}

float Average::sum(void) const
{
  return(mSum);
}
