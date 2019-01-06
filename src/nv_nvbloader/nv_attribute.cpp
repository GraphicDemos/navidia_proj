/*********************************************************************NVMH3****
Path:  E:\nvidia\devrel\NVSDK\Common\src\nv_core
File:  nv_attribute.cpp

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:




******************************************************************************/

#include "nv_attribute.h"

nv_attribute::nv_attribute()
{
    _pdata = 0;
    _size = 0;
    _flag = NV_UNASSIGNED;
}

nv_attribute::nv_attribute(const nv_attribute & attr)
{
    _pdata = 0;
    _size = 0;
    copy_from(&attr);
}

nv_attribute::~nv_attribute()
{
    erase();

    _dico_it it = _attr.begin();
    while (it != _attr.end())
    {
        delete (*it).second;
        ++it;
    }
    _attr.clear();
    _flag = NV_UNASSIGNED;
}

nv_attribute & nv_attribute::operator= (const char * str)
{
    array(str, strlen(str) + 1);
    _flag = NV_STRING;
    return *this;
}

nv_attribute & nv_attribute::operator= (const float & val)
{
    _float = val;
    _flag = NV_FLOAT;
    return *this;
}

nv_attribute & nv_attribute::operator= (const unsigned int & val)
{
    _udword = val;
    _flag = NV_UNSIGNED_INT;
    return *this;
}

nv_attribute & nv_attribute::operator= (const int & val)
{
    _sdword = val;
    _flag = NV_INT;
    return *this;
}

nv_attribute & nv_attribute::operator= (const unsigned char & val)
{
    _ubyte = val;
    _flag = NV_UNSIGNED_CHAR;
    return *this;
}

nv_attribute & nv_attribute::operator= (const char & val)
{
    _byte = val;
    _flag = NV_CHAR;
    return *this;
}

nv_attribute & nv_attribute::operator= (const short & val)
{
    _word = val;
    _flag = NV_SHORT;
    return *this;
}

bool nv_attribute::is_null () const
{
    return _flag == NV_UNASSIGNED;
}

bool nv_attribute::is_valid () const
{
    return _flag != NV_UNASSIGNED;
}

nv_attribute & nv_attribute::operator= (const nv_attribute & attr)
{
    erase();
    // delete the dictionnary
    _dico_it it = _attr.begin();
    while (it != _attr.end())
    {
        delete (*it).second;
        ++it;
    }
    _attr.clear();
    copy_from(&attr);
    return *this;
}

nv_attribute & nv_attribute::operator[] (const char * str)
{
    std::string name(str);
    _dico_it it = _attr.find(name);
    if (it != _attr.end())
        return *(*it).second;

    nv_attribute * attrib = new nv_attribute;
    _attr.insert(_dico_pair(name,attrib));
    return *attrib;
}

// array of structs replacement?
void nv_attribute::array(const nv_attribute * attribs, unsigned int size)
{
    erase();
    if (size)
    {
        _pdata = reinterpret_cast<void*>(new nv_attribute[size]);
        // do the copy...
        for (unsigned int i = 0; i < size; ++i)
            (reinterpret_cast<nv_attribute*>(_pdata))[i] = attribs[i];
    }
    _size = size;
    _flag = NV_ATTRIBUTE_ARRAY;
}

void nv_attribute::array(const float * fvector, unsigned int size)
{
    erase();
    if (size)
    {
        _pdata = reinterpret_cast<void*>(new float[size]);
        memcpy(_pdata,fvector,size * sizeof(float));
    }
    _size = size;
    _flag = NV_FLOAT_ARRAY;
}

void nv_attribute::array(const char * str, unsigned int size)
{
    erase();
    if (size)
    {
        _pdata = reinterpret_cast<void*>(new char[size]);
        memcpy(_pdata,str,size * sizeof(char));
    }
    _size = size;
    _flag = NV_CHAR_ARRAY;
}

void nv_attribute::array(const unsigned char * str, unsigned int size)
{
    erase();
    if (size)
    {
        _pdata = reinterpret_cast<void*>(new unsigned char[size]);
        memcpy(_pdata,str,size * sizeof(unsigned char));
    }
    _size = size;
    _flag = NV_UNSIGNED_CHAR_ARRAY;
}

void nv_attribute::array(const unsigned int * array, unsigned int size)
{
    erase();
    if (size)
    {
        _pdata = reinterpret_cast<void*>(new unsigned int[size]);
        memcpy(_pdata,array,size * sizeof(unsigned int));
    }
    _size = size;
    _flag = NV_UNSIGNED_INT_ARRAY;
}

void nv_attribute::array(const int * array, unsigned int size)
{
    erase();
    if (size)
    {
        _pdata = reinterpret_cast<void*>(new int[size]);
        memcpy(_pdata,array,size * sizeof(int));
    }
    _size = size;
    _flag = NV_INT_ARRAY;
}

void nv_attribute::array(const short * array, unsigned int size)
{
    erase();
    if (size)
    {
        _pdata = reinterpret_cast<void*>(new short[size]);
        memcpy(_pdata,array,size * sizeof(short));
    }
    _size = size;
    _flag = NV_SHORT_ARRAY;
}
    
// value accessors...
float nv_attribute::as_float() const
{
    return _float;
}

unsigned int nv_attribute::as_unsigned_int() const
{
    return _udword;
}

unsigned int nv_attribute::as_int() const
{
    return _sdword;
}

short nv_attribute::as_word() const
{
    return _word;
}

unsigned char nv_attribute::as_uchar() const
{
    return _ubyte;
}

char nv_attribute::as_char() const
{
    return _byte;
}

const char * nv_attribute::as_char_array() const
{
    return reinterpret_cast<const char*>(_pdata);
}

const unsigned char * nv_attribute::as_unsigned_char_array() const
{
    return reinterpret_cast<unsigned char*>(_pdata);
}

const int * nv_attribute::as_int_array() const
{
    return reinterpret_cast<int*>(_pdata);
}

const unsigned int * nv_attribute::as_unsigned_int_array() const
{
    return reinterpret_cast<unsigned int*>(_pdata);
}

const float * nv_attribute::as_float_array() const
{
    return reinterpret_cast<const float*>(_pdata);
}

const short * nv_attribute::as_short_array() const
{
    return reinterpret_cast<const short*>(_pdata);
}

const char * nv_attribute::as_string() const
{
    return reinterpret_cast<const char*>(_pdata);
}

const nv_attribute * nv_attribute::as_attribute_array() const
{
    return reinterpret_cast<const nv_attribute*>(_pdata);
}

const unsigned int nv_attribute::get_size() const
{
    return _size;
}

const unsigned int nv_attribute::get_type() const
{
    return _flag;
}

void nv_attribute::erase()
{
    if (_size)
    {
        switch(_flag)
        {
            case NV_FLOAT_ARRAY:
                delete [] (float*)_pdata;
                break;
            case NV_CHAR_ARRAY:
                delete [] (char*)_pdata;
                break;
            case NV_UNSIGNED_CHAR_ARRAY:
                delete [] (unsigned char*)_pdata;
                break;
            case NV_UNSIGNED_INT_ARRAY:
                delete [] (unsigned int*)_pdata;
                break;
            case NV_INT_ARRAY:
                delete [] (int*)_pdata;
                break;
            case NV_ATTRIBUTE_ARRAY:
                delete [] (nv_attribute*)_pdata;
                break;
            case NV_SHORT_ARRAY:
                delete [] (short*)_pdata;
                break;
            default:
                break;
        }
    }
    _pdata = 0;
    _size = 0;
}

void nv_attribute::copy_from(const nv_attribute * attr)
{
    _flag = attr->_flag;

    if (attr->_size)
    {
        _size = attr->_size;
        if (_flag == NV_ATTRIBUTE_ARRAY)
        {
            _pdata = reinterpret_cast<void*>(new nv_attribute[_size]);
            for (unsigned int i = 0; i < _size; ++i)
                (reinterpret_cast<nv_attribute*>(_pdata))[i] = attr->as_attribute_array()[i];
        }
        else
        {
            unsigned int byte_size;
            unsigned int flag = _flag;
            flag &= ~nv_attribute::NV_ARRAY;
            if (flag & NV_FLOAT)
            {
                _pdata = reinterpret_cast<void*>(new float[attr->_size]);
                byte_size = _size * sizeof(float);
            }

            if (flag & NV_CHAR)
            {
                _pdata = reinterpret_cast<void*>(new char[attr->_size]);
                byte_size = _size * sizeof(char);
            }

            if (flag & NV_SHORT)
            {
                _pdata = reinterpret_cast<void*>(new short[attr->_size]);
                byte_size = _size * sizeof(short);
            }
                
            if (flag & NV_INT)
            {
                _pdata = reinterpret_cast<void*>(new int[attr->_size]);
                byte_size = _size * sizeof(int);
            }

            if (flag & NV_UNSIGNED_INT)
            {
                _pdata = reinterpret_cast<void*>(new unsigned int[attr->_size]);
                byte_size = _size * sizeof(unsigned int);
            }

            if (flag & NV_UNSIGNED_CHAR)
            {
                _pdata = reinterpret_cast<void*>(new unsigned char[attr->_size]);
                byte_size = _size * sizeof(unsigned char);
            }

            memcpy(_pdata, attr->_pdata, byte_size);
        }
    }
    else
        _udword = attr->_udword;

    _dico_cit it = attr->_attr.begin();
    while (it != attr->_attr.end())
    {
        nv_attribute* attrib = new nv_attribute(*(*it).second);
        _attr.insert(_dico_pair((*it).first,attrib));
        ++it;
    }
}

unsigned int nv_attribute::get_num_attributes() const
{
    return _attr.size();
}

const char * nv_attribute::get_attribute_name(unsigned int i) const
{
    unsigned int idx = 0;
    _dico_cit it = _attr.begin();
    while (it != _attr.end())
    {
        if (idx == i)
            return (*it).first.c_str();
        ++idx;
        ++it;
    }
    return 0;
}

const nv_attribute * nv_attribute::get_attribute(unsigned int i) const
{
    unsigned int idx = 0;
    _dico_cit it = _attr.begin();
    while (it != _attr.end())
    {
        if (idx == i)
            return (*it).second;
        ++idx;
        ++it;
    }
    return 0;
}

std::ostream & operator << (std::ostream& os, const nv_attribute & attr)
{
    static int tab = 0;
    unsigned int type = attr.get_type();
    if (type & nv_attribute::NV_ARRAY && type != nv_attribute::NV_UNASSIGNED)
    {
        os << "(array of ";
        type &= ~nv_attribute::NV_ARRAY;
    }
    else
        os << "(";
        
    switch (type)
    {
        case nv_attribute::NV_FLOAT:
            os << "float)";
            break;
        case nv_attribute::NV_CHAR:
            os << "char)";
            break;
        case nv_attribute::NV_UNSIGNED_INT:
            os << "unsigned int)";
            break;
        case nv_attribute::NV_UNSIGNED_CHAR:
            os << "unsigned char)";
            break;
        case nv_attribute::NV_SHORT:
            os << "short)";
            break;
        case nv_attribute::NV_INT:
            os << "int)";
            break;
        default:
            break;
    }

    if (type == nv_attribute::NV_UNASSIGNED)
        os << "unassigned)";

    os << std::endl;

    nv_attribute::_dico_cit it = attr._attr.begin();
    while (it != attr._attr.end())
    {
        for (int i = 0; i < tab - 1; ++i)
            os << " |   ";
        if (tab > 0)
            os << " o-->";
        os << (*it).first.c_str() << ".";
        ++tab;
        os << *(*it).second;
        --tab;
        ++it;
    }
    return os;
}

/*
int main(int argc, char* argv[])
{
    nv_attribute attrib;

    attrib["toto"] = 'c';
    attrib["test"] = 1.0f;
    attrib["test"]["test2"] = 2.0f;
    attrib["string"] = "blah";
    attrib["string"]["string2"] = "blah_blah";
    attrib["string"]["string2a"] = "blah_blah2a";
    attrib["string"]["string2"]["string3"] = "blah_blah_blah";

    nv_attribute attrib2;
    attrib2 = attrib;

    float u = attrib2["test"].as_float();
    float v = attrib["test"]["test2"].as_float();
    const char * string = attrib2["string"].as_string();
    const char * string2a = attrib2["string"]["string2a"].as_string();
    const char * string2b = attrib["string"]["string2"].as_string();

    float w = -1.0f;
    if (attrib2["test3"].is_valid())
        w = attrib2["test3"].as_float();

    cout << attrib;

    // test typedefs style...
    int j;
    nv_attribute streams[10];
    for (unsigned int i = 0; i < 10 ; ++i)
    {
        streams[i]["num_faces"] = i*100;
        streams[i]["num_verts"] = i*100;
        streams[i]["dim_vertex"] = unsigned int(3);
        streams[i]["dim_face"] = unsigned int(3);
        int alloc_size = streams[i]["num_verts"].as_unsigned_int() * streams[i]["dim_vertex"].as_unsigned_int();
        float * vertices = new float[alloc_size];
        for (j = 0; j < alloc_size; ++j)
            vertices[j] = float(j);

        streams[i]["vertices"].array(vertices, alloc_size );

        alloc_size = streams[i]["num_faces"].as_unsigned_int() * streams[i]["dim_face"].as_unsigned_int();
        unsigned int * faces = new unsigned int[alloc_size];
        for (j = 0; j < alloc_size; ++j)
            faces[j] = j;
        streams[i]["faces"].array(faces, alloc_size );
        delete [] vertices;
        delete [] faces;
    }

    attrib["streams"].array(streams,10);

    const nv_attribute * pstreams = attrib["streams"].as_attribute_array();
    for (i = 0; i < attrib["streams"].get_size() ; ++i)
    {
        cout << pstreams[i];
    }

    attrib["streams"] = attrib2;
    string2b = attrib["streams"]["string"]["string2"].as_string();
	return 0;
}

*/
