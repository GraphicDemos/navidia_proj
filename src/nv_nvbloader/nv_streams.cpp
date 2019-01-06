//
// Includes
//

#include "nv_streams.h"
#include "nv_attribute.h"

#include <assert.h>
        /// Change byte order according to endianness.
        inline
        void
fixByteOrder(void * pData, int nBytes)
{
#ifdef MACOS
    unsigned char nTemp;

                                // We don't support any native types
                                // with more then 8 bytes.
    assert(nBytes <= 8);

    unsigned char * pLower = static_cast<unsigned char *>(pData);
    unsigned char * pUpper = pLower + nBytes-1;
    while (pLower < pUpper)
    {
         nTemp  = *pUpper;
        *pUpper = *pLower;
        *pLower =  nTemp;

        pLower++;
        pUpper--;
    }
#endif
}

//
// Inserting functions
//

        /// Write a boolean value.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, bool bBool)
{
#ifdef MACOS
    assert(sizeof(unsigned char) == 1);

    unsigned char nBool;

    if (bBool)
        nBool = 1;
    else
        nBool = 0;

    rOutputStream << nBool;
#else
    assert(1 == sizeof(bBool));

    rOutputStream.write(&bBool, 1);
#endif // MACOS

    return rOutputStream;
}

        /// Write an unsigned char.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, unsigned char nChar)
{
    assert(1 == sizeof(nChar));

    rOutputStream.write(&nChar, 1);

    return rOutputStream;
}

        /// Write a char.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, char nChar)
{
    assert(1 == sizeof(nChar));

    rOutputStream.write(&nChar, 1);

    return rOutputStream;
}

        /// Write an unsigned short.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, unsigned short nShort)
{
    assert(2 == sizeof(nShort));

    fixByteOrder(&nShort, 2);

    rOutputStream.write(&nShort, 2);

    return rOutputStream;
}

        /// Write a short.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, short nShort)
{
    assert(2 == sizeof(nShort));

    fixByteOrder(&nShort, 2);

    rOutputStream.write(&nShort, 2);

    return rOutputStream;
}

        /// Write an unsigned int.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, unsigned int nInt)
{
    assert(4 == sizeof(nInt));

    fixByteOrder(&nInt, 4);

    rOutputStream.write(&nInt, 4);

    return rOutputStream;
}

        /// Write an int.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, int nInt)
{
    assert(4 == sizeof(nInt));

    fixByteOrder(&nInt, 4);

    rOutputStream.write(&nInt, 4);

    return rOutputStream;
}

        /// Write a float.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, float nFloat)
{
    assert(4 == sizeof(nFloat));

    fixByteOrder(&nFloat, 4);

    rOutputStream.write(&nFloat, 4);

    return rOutputStream;
}

        /// Write a double.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, double nDouble)
{
    assert(8 == sizeof(nDouble));

    fixByteOrder(&nDouble, 8);
    rOutputStream.write(&nDouble, 8);

    return rOutputStream;
}

        /// Write a vec2
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const vec2 & rVector)
{
    rOutputStream << rVector.x << rVector.y;

    return rOutputStream;
}
        
        /// Write a vec3
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const vec3 & rVector)
{
    rOutputStream << rVector.x << rVector.y << rVector.z;

    return rOutputStream;
}

        /// Write a vec4
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const vec4 & rVector)
{
    rOutputStream << rVector.x << rVector.y << rVector.z << rVector.w;

    return rOutputStream;
}

        /// Write a mat3
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const mat3 & rMatrix)
{
    for (int i = 0; i < 9; i++)
        rOutputStream << rMatrix.mat_array[i];

    return rOutputStream;
}

        /// Write a mat4
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const mat4 & rMatrix)
{
    for (int i = 0; i < 16; i++)
        rOutputStream << rMatrix.mat_array[i];

    return rOutputStream;
}

        /// Write a quaternion.
        nv_output_stream &
operator << (nv_output_stream & rOutputStream, const quat & rQuaternion)
{
    rOutputStream << rQuaternion.x << rQuaternion.y << rQuaternion.z << rQuaternion.w;

    return rOutputStream;
}

         /// Write an nv_attribute
        nv_output_stream & 
operator << (nv_output_stream & rOutputStream, const nv_attribute & oAttribute)
{
                                // Write the dictionary. First its size
    rOutputStream << oAttribute.get_num_attributes();
                                // ... then its elements.
    for (unsigned int iAttribute = 0; iAttribute < oAttribute.get_num_attributes(); ++iAttribute)
    {
        rOutputStream << (unsigned int)strlen(oAttribute.get_attribute_name(iAttribute));
        rOutputStream.write(oAttribute.get_attribute_name(iAttribute), 
                            strlen(oAttribute.get_attribute_name(iAttribute)));

        rOutputStream << *(oAttribute.get_attribute(iAttribute));
    }

    rOutputStream << oAttribute._size;
    rOutputStream << oAttribute._flag;

                                // Now lets check if we have to save arrays...
    switch(oAttribute._flag)
    {
        case nv_attribute::NV_ATTRIBUTE_ARRAY:
        {
            for (unsigned int iAttribute = 0; iAttribute < oAttribute._size; ++iAttribute)
                rOutputStream << oAttribute.as_attribute_array()[iAttribute];
        }
        break;

        case nv_attribute::NV_FLOAT_ARRAY:
        {
            for (unsigned int iAttribute = 0; iAttribute < oAttribute._size; ++iAttribute)
                rOutputStream << oAttribute.as_float_array()[iAttribute];
        }
        break;

        case nv_attribute::NV_CHAR_ARRAY:
        {
            for (unsigned int iAttribute = 0; iAttribute < oAttribute._size; ++iAttribute)
                rOutputStream << oAttribute.as_char_array()[iAttribute];
        
        }
        break;

        case nv_attribute::NV_SHORT_ARRAY:
        {
            for (unsigned int iAttribute = 0; iAttribute < oAttribute._size; ++iAttribute)
                rOutputStream << oAttribute.as_short_array()[iAttribute];
        
        }
        break;

        case nv_attribute::NV_INT_ARRAY:
        {
            for (unsigned int iAttribute = 0; iAttribute < oAttribute._size; ++iAttribute)
                rOutputStream << oAttribute.as_int_array()[iAttribute];
        
        }
        break;

        case nv_attribute::NV_UNSIGNED_INT_ARRAY:
        {
            for (unsigned int iAttribute = 0; iAttribute < oAttribute._size; ++iAttribute)
                rOutputStream << oAttribute.as_unsigned_int_array()[iAttribute];
        
        }
        break;

        case nv_attribute::NV_UNSIGNED_CHAR_ARRAY:
        {
            for (unsigned int iAttribute = 0; iAttribute < oAttribute._size; ++iAttribute)
                rOutputStream << oAttribute.as_unsigned_char_array()[iAttribute];
        
        }
        break;
    }
        
                                // and save the 32-bit data word 
                                // if it's not used as data pointer.
    if (oAttribute._size == 0)
        rOutputStream << oAttribute._udword;

    return rOutputStream;
}


//
// Extracting functions
//

        /// Read a boolean.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, bool & bBool)
{
#ifdef MACOS
    assert(sizeof(unsigned char) == 1);

    unsigned char nBool;

    rInputStream >> nBool;

                                // Jason, you might have to add a cast here to get
                                // rid of warning.
    bBool = nBool;
#else
    assert(1 == sizeof(bBool));

    char * pTemp = reinterpret_cast<char *>(&bBool);

    rInputStream.read(pTemp, 1);
		bBool = ((*pTemp) ? true : false);
#endif // MACOS

    return rInputStream;
}

        /// Read an unsigned char.
        nv_input_stream & 
operator >> (nv_input_stream & rInputStream, unsigned char & nChar)
{
    assert(1 == sizeof(nChar));

    rInputStream.read(&nChar, 1);

    return rInputStream;
}

        /// Read a char.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, char & nChar)
{
    assert(1 == sizeof(nChar));

    rInputStream.read(&nChar, 1);

    return rInputStream;
}

        /// Read an unsigned short.
        nv_input_stream & 
operator >> (nv_input_stream & rInputStream, unsigned short & nShort)
{
    assert(2 == sizeof(nShort));

    rInputStream.read(&nShort, 2);

    fixByteOrder(&nShort, 2);

    return rInputStream;
}

        /// Read a short.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, short & nShort)
{
    assert(2 == sizeof(nShort));

    rInputStream.read(&nShort, 2);

    fixByteOrder(&nShort, 2);

    return rInputStream;
}

        /// Read an unsigned int.
        nv_input_stream & 
operator >> (nv_input_stream & rInputStream, unsigned int & nInt)
{
    assert(4 == sizeof(nInt));

    rInputStream.read(&nInt, 4);

    fixByteOrder(&nInt, 4);

    return rInputStream;
}

        /// Read an int.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, int & nInt)
{
    assert(4 == sizeof(nInt));

    rInputStream.read(&nInt, 4);

    fixByteOrder(&nInt, 4);

    return rInputStream;
}

        /// Read a float.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, float & nFloat)
{
    assert(4 == sizeof(nFloat));

    rInputStream.read(&nFloat, 4);

    fixByteOrder(&nFloat, 4);

    return rInputStream;
}

        /// Read a double.
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, double & nDouble)
{
    assert(8 == sizeof(nDouble));

    rInputStream.read(&nDouble, 8);

    fixByteOrder(&nDouble, 8);

    return rInputStream;
}

        /// Read a vec2
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, vec2 & rVector)
{
    rInputStream.read(rVector.vec_array, 2*sizeof(nv_scalar));

    fixByteOrder(&rVector.x, sizeof(nv_scalar));
    fixByteOrder(&rVector.y, sizeof(nv_scalar));
        
    return rInputStream;
}

        /// Read a vec3
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, vec3 & rVector)
{
    rInputStream.read(rVector.vec_array, 3*sizeof(nv_scalar));

    fixByteOrder(&rVector.x, sizeof(nv_scalar));
    fixByteOrder(&rVector.y, sizeof(nv_scalar));
    fixByteOrder(&rVector.z, sizeof(nv_scalar));

    return rInputStream;
}

        /// Read a vec4
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, vec4 & rVector)
{
    rInputStream.read(rVector.vec_array, 4*sizeof(nv_scalar));

    fixByteOrder(&rVector.x, sizeof(nv_scalar));
    fixByteOrder(&rVector.y, sizeof(nv_scalar));
    fixByteOrder(&rVector.z, sizeof(nv_scalar));
    fixByteOrder(&rVector.w, sizeof(nv_scalar));

    return rInputStream;
}

        /// Read a mat3
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, mat3 & rMatrix)
{
    rInputStream.read(rMatrix.mat_array, 9*sizeof(nv_scalar));

    for (int i = 0; i < 9; i++)
        fixByteOrder(&rMatrix.mat_array[i], sizeof(nv_scalar));

    return rInputStream;
}

        /// Read a mat4
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, mat4 & rMatrix)
{
    rInputStream.read(rMatrix.mat_array, 16*sizeof(nv_scalar));

    for (int i = 0; i < 16; i++)
        fixByteOrder(&rMatrix.mat_array[i], sizeof(nv_scalar));

    return rInputStream;
}

        /// Read a quaternion
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, quat & rQuaternion)
{
    rInputStream.read(rQuaternion.comp, 4*sizeof(nv_scalar));

    fixByteOrder(&rQuaternion.x, sizeof(nv_scalar));
    fixByteOrder(&rQuaternion.y, sizeof(nv_scalar));
    fixByteOrder(&rQuaternion.z, sizeof(nv_scalar));
    fixByteOrder(&rQuaternion.w, sizeof(nv_scalar));

    return rInputStream;
}


        /// Read an nv_attribute
        nv_input_stream &
operator >> (nv_input_stream & rInputStream, nv_attribute & rAttribute)
{
                                // Read the dictionary. First its size
    unsigned int nEntries;
    rInputStream >> nEntries;

                                // ... then its elements.
    for (unsigned int iAttribute = 0; iAttribute < nEntries; ++iAttribute)
    {
        int nNameLength;
        rInputStream >> nNameLength;
        char * zName = new char[nNameLength + 1];
        zName[nNameLength] = '\0';
        rInputStream.read(zName, nNameLength);

        nv_attribute oAttribute;
        rInputStream >> oAttribute;

        rAttribute[zName] = oAttribute;

        delete zName;
    }

    rInputStream >> rAttribute._size;
    rInputStream >> rAttribute._flag;

                                // Now lets check if we have to save arrays...
    switch(rAttribute._flag)
    {
        case nv_attribute::NV_ATTRIBUTE_ARRAY:
        {
            nv_attribute * aAttributes = new nv_attribute[rAttribute._size];
            
            for (unsigned int iAttribute = 0; iAttribute < rAttribute._size; ++iAttribute)
                rInputStream >> aAttributes[iAttribute];

            rAttribute.array(aAttributes, rAttribute._size);

            delete aAttributes;
        }
        break;

        case nv_attribute::NV_FLOAT_ARRAY:
        {
            float * aFloats = new float[rAttribute._size];

            for (unsigned int iFloat = 0; iFloat < rAttribute._size; ++iFloat)
                rInputStream >> aFloats[iFloat];

            rAttribute.array(aFloats, rAttribute._size);

            delete aFloats;
        }
        break;

        case nv_attribute::NV_CHAR_ARRAY:
        {
            char * aChars = new char[rAttribute._size];

            for (unsigned int iChar = 0; iChar < rAttribute._size; ++iChar)
                rInputStream >> aChars[iChar];

            rAttribute.array(aChars, rAttribute._size);

            delete aChars;
        }
        break;

        case nv_attribute::NV_SHORT_ARRAY:
        {
            short * aShorts = new short[rAttribute._size];

            for (unsigned int iShort = 0; iShort < rAttribute._size; ++iShort)
                rInputStream >> aShorts[iShort];

            rAttribute.array(aShorts, rAttribute._size);

            delete aShorts;
        }
        break;

        case nv_attribute::NV_INT_ARRAY:
        {
            int * aInts = new int[rAttribute._size];

            for (unsigned int iInt = 0; iInt < rAttribute._size; ++iInt)
                rInputStream >> aInts[iInt];
                
            rAttribute.array(aInts, rAttribute._size);

            delete aInts;
        }
        break;

        case nv_attribute::NV_UNSIGNED_INT_ARRAY:
        {
            unsigned int * aInts = new unsigned int[rAttribute._size];

            for (unsigned int iInt = 0; iInt < rAttribute._size; ++iInt)
                rInputStream >> aInts[iInt];
            rAttribute.array(aInts, rAttribute._size);

            delete aInts;
        }
        break;

        case nv_attribute::NV_UNSIGNED_CHAR_ARRAY:
        {
            unsigned char * aChars = new unsigned char[rAttribute._size];

            for (unsigned int iChar = 0; iChar < rAttribute._size; ++iChar)
                rInputStream >> aChars[iChar];

            rAttribute.array(aChars, rAttribute._size);

            delete aChars;
        }
        break;
    }
        
                                // and save the 32-bit data word 
                                // if it's not used as data pointer.
    if (rAttribute._size == 0)
        rInputStream >> rAttribute._udword;

    return rInputStream;
}



