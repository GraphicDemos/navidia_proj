#ifndef FILE_H
#define FILE_H

#include <vector>

class File
{
public:
  File(void);
  File(const char* filename);
  
  static void addPath(const char* path);
  int         length(void) const;
  bool        load(const char* filename);

  operator const char* (void) const;

private:
  std::vector<char> mData;
  int               mLength;
};


#endif
