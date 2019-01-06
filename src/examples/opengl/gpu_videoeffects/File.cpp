#include "File.h"

#include <assert.h>
#include <shared/data_path.h>
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

static data_path sPath;

File::File(void)
{
  mLength = 0;
}

File::File(const char* filename)
{
  load(filename);
}

void File::addPath(const char* path)
{
  assert(path != NULL);

  sPath.path.push_back(path);
}

int File::length(void) const
{
  return(mLength);
}

bool File::load(const char* filename)
{
  assert(filename != NULL);

  FILE*  fin;
  string fullpath;

  fullpath = sPath.get_file(filename);
  if(fullpath == "")
    {
      printf("Couldn't find \"%s\"\n", filename);
      return(false);
    }

  if((fin = fopen(fullpath.c_str(), "rb")) == NULL)
  {
    printf("couldn't not open \"%s\"\n", fullpath.c_str());
    return(false);
  }

  //printf("INFO  File::load() Found \"%s\" at\n    \"%s\"\n", filename, fullpath.c_str());

  fseek(fin, 0, SEEK_END);
  mLength = ftell(fin);
  fseek(fin, 0, SEEK_SET);
  
  mData.resize(mLength + 1);
  
  fread(&(mData[0]), mLength, 1, fin);
  mData[mLength] = 0;

  fclose(fin);

  return(true);
}

File::operator const char* (void) const
{
  return(&mData[0]);
}
