///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Contains file related code.
 *  \file       NVBFile.h
 *  \author     Pierre Terdiman
 *  \date       April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#include "nv_nvb_loader_decl.h"
#include "NVBTypes.h"
#include "NVBString.h"
#include <stdio.h>

#include <windows.h>

#ifndef __NVBFILE_H__
#define __NVBFILE_H__

    #define FILE_SAVE   false       //!< Fileselect parameter.
    #define FILE_LOAD   true        //!< Fileselect parameter.

    // File access
    #define FILE_READ               0x1
    #define FILE_WRITE              0x2
    #define FILE_SHAREREAD          0x4
    #define FILE_SHAREWRITE         0x8

    // File attrib
    #define FILE_ATTRB_READONLY     0x00000001  
    #define FILE_ATTRB_HIDDEN       0x00000002  
    #define FILE_ATTRB_SYSTEM       0x00000004  
    #define FILE_ATTRB_DIRECTORY    0x00000008  
    #define FILE_ATTRB_ARCHIVE      0x00000010  
    #define FILE_ATTRB_NORMAL       0x00000020  

    // File open flags
    enum FILE_CREATEATTRB
    {
        FILE_CREATE_NEW             = 1,                            // Create the file, fails if the same filename already exists
        FILE_CREATE_ALWAYS          = 2,                            // Create or open an existing file (with truncation to zero)
        FILE_OPEN_EXISTING          = 3,                            // Open an existing file, fails if the file doesn't exist
        FILE_OPEN_ALWAYS            = 4,                            // Open an existing file, or create one if it doens't exist
        FILE_OPFORCEDW              = 0x7FFFFFFF,
    };

    // File seek flags
    enum FILE_SEEK
    {
        FILE_FROM_BEGIN             = 1,
        FILE_FROM_CURRENT           = 2, 
        FILE_FROM_END               = 4,
        FILE_FROM_FORCE             = 0x7FFFFFFF,
    };


    // Misc functions

    //NVBCORE_API   bool        FileSelect(const char* type, const char* destname, const char* alias, bool mode, char* filename);

     NVBCORE_API    udword      GetFileSize(const char* name);
     NVBCORE_API    bool        FileExists(const char* filename);
     NVBCORE_API    const char* ChangeExtension(const char* filename, const char* newext);
    
     NVBCORE_API    udword      CreateFile(String& filepathname, udword fileaccess, FILE_CREATEATTRB fileattrib);
     NVBCORE_API    bool        CloseFile(udword handle);
     NVBCORE_API    udword      SeekFilePtr(udword handle, udword length, FILE_SEEK seek);
     NVBCORE_API    bool        WriteFile(udword handle, const void* buffer, udword length, udword* writtenlength=null);
     NVBCORE_API    bool        ReadFile(udword handle, void* buffer, udword length, udword* readlength);
     NVBCORE_API    udword      GetFilePosition(udword handle);
     NVBCORE_API  udword      GetFileLength(udword handle);
     NVBCORE_API    bool        DeleteFile(String& filepathname);
     NVBCORE_API    bool        CopyFile(String& existingfile, String& newfile, bool overwrite);
     NVBCORE_API    bool        MoveFile(String& existingfile, String& newfile);
     NVBCORE_API    udword      GetFileAttributes(String& filepathname);

#ifndef PATH_MAX
    #define PATH_MAX    _MAX_PATH
#endif

    // Paths
     NVBCORE_API    bool        RegisterPath(const char* path);
     NVBCORE_API    bool        ReleasePaths();
     NVBCORE_API    bool        FindFile(const char* filename, String* file=null);
     NVBCORE_API    const char* GetCurrentDirectory();

    #define FINDFILE_DIR            0x01
    #define FINDFILE_HIDDEN         0x02
    #define FINDFILE_NORMAL         0x04
    #define FINDFILE_READONLY       0x08

    class NVBCORE_API NVBFile
    {
        public:
        // Constructor/Destructor
                                            NVBFile(const String& filename, const char* access);
                                            ~NVBFile();

                        ubyte               GetByte();
                        uword               GetWord();
                        udword              GetDword();
                        float               GetFloat();
                        const char*         GetString();
                        bool                GetBuffer(ubyte* dest, udword size);
                        bool                Seek(udword pos);

                        bool                GetLine(char* buffer);
                        ubyte*              Load(udword& length);
        private:
                        String              mName;
                        FILE*               mFp;

                        ubyte*              mBuffer;
                        udword              mBufferLength;
    };

    class NVBCORE_API FileFinder
    {
        public:
        // Constructor/Destructor
                                            FileFinder(const char* mask);
                        bool                FindNext();
#ifdef WIN32
                        bool                IsValid()                               { return (mHandle==(void*)-1)==0; }
#else
                        bool                IsValid()                               { return true; }
#endif
        public:
                        String              mFile;
                        udword              mAttribs;
                        udword              mSize;
        private:
#ifdef WIN32
                        WIN32_FIND_DATAA     mFindData;
                        HANDLE              mHandle;
#else

#endif
    };

    #define FILEFOUND       __ffnvb__.mFile;

    #define STARTFINDFILES(x)               \
    FileFinder  __ffnvb__(x);               \
    while(__ffnvb__.IsValid())              \
    {

    #define ENDFINDFILES                    \
        __ffnvb__.FindNext();               \
    }

   NVBCORE_API void UnixToDosName(char* name);
   NVBCORE_API void DosToUnixName(char* name);

   NVBCORE_API bool RemovePath(String& filename);
   NVBCORE_API bool GetPath(const String& filename, String& path);

#endif // __NVBFILE_H__


