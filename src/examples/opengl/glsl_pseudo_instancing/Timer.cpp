
#include "Timer.h"

static const int CALIBRATION_CALL_COUNT = 100;

Timer::Timer(void)
{
  LARGE_INTEGER frequency;
  double        t;
  int           i;

  QueryPerformanceFrequency(&frequency);
  _frequency = convert(frequency);
  _lastTime = 0.0;

  // Calibrate the Performance Counter
  t = 0.0;
  tick();
  for(i=0; i<CALIBRATION_CALL_COUNT; i++)
    {
      t += tick();
    }
  _callTime = t / CALIBRATION_CALL_COUNT;

  clearTotalTime();
}

double Timer::convert(const LARGE_INTEGER& value)
{
  return(((double)value.HighPart * 4294967296.0) + ((double)value.LowPart));
}

double Timer::begin(void)
{
  return(tick());
}

double Timer::end(void)
{
  double dt;

  dt = tick() - 2.0 * _callTime;
  if(dt < 0.0)
    {
      dt = 0.0;
    }

  _totalTime += dt;

  return(dt);
}

void Timer::clearTotalTime(void)
{
  _totalTime = 0.0;
}

double Timer::getTotalTime(void) const
{
  return(_totalTime);
}

double Timer::tick(void)
{
  double currentTime;
  double delta;

  currentTime = time();

  delta = currentTime - _lastTime;
  _lastTime = currentTime;

  return(delta);
}

double Timer::time(void)
{
  LARGE_INTEGER currentTimeLargeInteger;
  double        currentTime;

  QueryPerformanceCounter(&currentTimeLargeInteger);
  currentTime = convert(currentTimeLargeInteger) / _frequency;

  return(currentTime);
}
