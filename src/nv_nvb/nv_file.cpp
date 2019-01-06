/**
 * \file nv_file.cpp
 *
 * Copyright (C) 2003 NVIDIA Corporation
 * 
 * This file is provided without support, instruction, or implied 
 * warranty of any kind.  NVIDIA makes no guarantee of its fitness
 * for a particular purpose and is not liable under any circumstances
 * for any damages or loss whatsoever arising from the use or 
 * inability to use this file or items derived from it.
 *
 */


//
// Includes
//

#include "stdafx.h"

#include "assert.h"




//
// Construction and destruction
//

        /// Default constructor.
nv_file::nv_file(): _sFilename("")
                  , _hFile(0)
                  , _eMode(nv_file::CLOSED_MODE)
{
    ; // empty
}

        /// Construct from filename.
nv_file::nv_file(const char * sFilename, nv_file::teMode eMode): _sFilename(sFilename)
                                                               , _hFile(0)
                                                               , _eMode(eMode)
{
    if (CLOSED_MODE != _eMode)
    {
        _hFile = fopen(_sFilename.c_str(), ModeString(_eMode));
        assert(0 != _hFile);
    }
}

        /// Destructor.
nv_file::~nv_file()
{
    if (CLOSED_MODE != _eMode)
    {
        int nError;

        nError = fflush(_hFile);
        assert(0 == nError);
    
        nError = fclose(_hFile);
        assert(0 == nError);
    }
}


//
// Public methods
//

        /// Get the current file's filename.
        std::string
nv_file::filename()
        const
{
    return _sFilename;
}

        /// Get the current file's mode.
        nv_file::teMode
nv_file::mode()
        const
{
    return _eMode;
}

        /// Open a file by name.
        nv_file::teError
nv_file::open(const char * sFilename, teMode eMode)
{
    if (CLOSED_MODE != _eMode)
    {
        int nError;

        nError = fflush(_hFile);
        if(0 != nError)
            return UNKNOWN_FILE_ERROR;
    
        nError = fclose(_hFile);
        if(0 != nError)
            return UNKNOWN_FILE_ERROR;
    }

    _eMode = eMode;
    _sFilename = sFilename;

    if (CLOSED_MODE != _eMode)
    {
        _hFile = fopen(_sFilename.c_str(), ModeString(_eMode));
        if(0 == _hFile)
            return UNKNOWN_FILE_ERROR;
    }
    
    return NO_FILE_ERROR;
}

        /// Close this file.
        nv_file::teError
nv_file::close()
{
    if (CLOSED_MODE != _eMode)
    {
        int nError;

        nError = fclose(_hFile);
        if(0 != nError)
            return UNKNOWN_FILE_ERROR;
        _eMode = CLOSED_MODE;
    }

    return NO_FILE_ERROR;
}

        /// Reopen the current file.
        nv_file::teError
nv_file::reopen(teMode eMode)
{
    assert(CLOSED_MODE == _eMode);

    
    _hFile = fopen(_sFilename.c_str(), ModeString(eMode));
    if(0 == _hFile)
        return UNKNOWN_FILE_ERROR;
    _eMode = eMode;

    return NO_FILE_ERROR;
}

        /// Read data.
        int
nv_file::read(void * pData, int nBytes)
{
    assert(READ_MODE == _eMode);
    return fread(pData, 1, nBytes, _hFile);
}

        /// Write data.
        int
nv_file::write(const void * pData, int nBytes)
{
    assert(WRITE_MODE == _eMode);
    return fwrite(pData, 1, nBytes, _hFile);
}

        /// Get a modestring from the enumerated mode
        const 
        char *
nv_file::ModeString(nv_file::teMode eMode)
{
    switch(eMode)
    {
        case READ_MODE:
            return "rb";
        case WRITE_MODE:
            return "wb";
        default:
            return "";
    }
}

