#ifndef INSTANCE_CONFIGURATION_H
#define INSTANCE_CONFIGURATION_H

class InstanceConfiguration
{
public:
  InstanceConfiguration(void);

  void changeState(int delta);
  int  columns(void) const;
  int  configurationCount(void) const;
  int  count(void) const;
  int  current(void) const;
  void setState(int state);
  int  rows(void) const;

private:
  int mCurrent;
  int mRows;
  int mColumns;
  int mCount;
  int mConfigurationCount;
};

#endif
