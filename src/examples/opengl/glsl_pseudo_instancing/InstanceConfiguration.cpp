
#include "InstanceConfiguration.h"

#include <assert.h>

static int sInstanceConfigurations[][2] = {{8, 8}, {16, 16}, {32, 32}, {64, 32}, {64, 64}, {128, 64}, 
                                           {128, 128}, {256, 128}, {256, 256}, {512, 256}, {512, 512}};
                                           


InstanceConfiguration::InstanceConfiguration(void)
{
  mCurrent = 7;
  mConfigurationCount = sizeof(sInstanceConfigurations) / sizeof(sInstanceConfigurations[0]);
  assert(mConfigurationCount > 0);
  changeState(0);
}

void InstanceConfiguration::changeState(int delta)
{
  assert(mConfigurationCount > 0);
    
  mCurrent = mCurrent + delta;
  
  if(mCurrent < 0)
    {
      mCurrent = 0;
    }
  else if(mCurrent >= mConfigurationCount)
    {
      mCurrent = mConfigurationCount - 1;
    }
  
  mRows = sInstanceConfigurations[mCurrent][0];
  mColumns = sInstanceConfigurations[mCurrent][1];
  mCount = mRows * mColumns;
}

int InstanceConfiguration::columns(void) const
{
  return(mColumns);
}

int InstanceConfiguration::configurationCount(void) const
{
  return(mConfigurationCount);
}

int InstanceConfiguration::current(void) const
{
  return(mCurrent);
}

int InstanceConfiguration::count(void) const
{
  return(mCount);
}

void InstanceConfiguration::setState(int state)
{
  assert(state >= 0);
  assert(state < mConfigurationCount);

  changeState(state - mCurrent);
}

int InstanceConfiguration::rows(void) const
{
  return(mRows);
}
