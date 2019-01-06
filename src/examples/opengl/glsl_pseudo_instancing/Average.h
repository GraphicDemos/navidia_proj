#ifndef AVERAGE_H
#define AVERAGE_H

class Average
{
public:
  Average(void);

  void  addSample(float sample);
  float average(void) const;
  int   count(void) const;
  void  reset(void);
  float sum(void) const;
  
protected:
  float mSum;
  float mAverage;
  int   mCount;
};


#endif

