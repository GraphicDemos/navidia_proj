#include "shared\NV_StringFuncs.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"
#pragma warning(disable : 4786)
#include <string>
#include <vector>
#include <list>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
namespace NVStringConv
{
	// use as an alternative to wcstombs()
	std::string WStringToString(const std::wstring * in_pwstring)
	{
		if (in_pwstring == NULL)
			return("");
		return(lpcwstrToString(in_pwstring->c_str()));
	}
	std::string WStringToString(std::wstring & in_wstring)
	{
		return(lpcwstrToString(in_wstring.c_str()));
	}
	std::string lpcwstrToString(LPCWSTR in_lpcwstr)
	{
		//@ consider using windows.h WideCharToMultiByte(..)
		char * mbBuf;
		size_t sz;
		sz = 2 * wcslen(in_lpcwstr);
		mbBuf = new char[sz];
		if (mbBuf == NULL)
			return("");
		wcstombs(mbBuf, in_lpcwstr, sz);	// convert the string
		std::string outstr;
		outstr = mbBuf;
		delete[] mbBuf;
		return(outstr);
	}
	std::wstring StringToWString(const std::string * in_pString)
	{
		std::wstring wstr = L"";
		if (in_pString != NULL)
			wstr = lpcstrToWString(in_pString->c_str());
		return(wstr);
	}
	std::wstring StringToWString(std::string & in_String)
	{
		return(lpcstrToWString(in_String.c_str()));
	}
	std::wstring lpcstrToWString(LPCSTR lpstr)
	{
		int nLen = MultiByteToWideChar(CP_ACP, 0, lpstr, -1, NULL, NULL);
		LPWSTR lpszW = new WCHAR[nLen + 1];
		MultiByteToWideChar(CP_ACP, 0, lpstr, -1, lpszW, nLen);
		lpszW[nLen] = '\0';
		std::wstring wstr;
		wstr.reserve(nLen);
		wstr.insert(0, lpszW);
		//	wstr = lpszW;
		delete[] lpszW;
		return(wstr);
	}
};	// namespace NVStringConv
	//---------------------------------------------------------------------------
	// Create a string using va_args, just like printf, sprintf, etc.
std::string StrPrintf(const char * szFormat, ...)
{
	std::string out;
	const int bufsz = 2048;
	char buffer[bufsz];
	va_list args;
	va_start(args, szFormat);
	_vsnprintf(buffer, bufsz, szFormat, args);
	va_end(args);
	buffer[bufsz - 1] = '\0';	// terminate in case of overflow
	out = buffer;
	return(out);
}

tstring		tstrPrintf(const TCHAR * format, ...)
{
	_TCHAR str[256];
	va_list argptr;
	va_start(argptr, format);
	_vstprintf(str, format, argptr);
	va_end(argptr);
	return str;
}
std::string StrToUpper(const std::string & strIn)
{
	unsigned int i;
	std::string out;
	out.reserve(strIn.size());
	for (i = 0; i < strIn.size(); i++)
	{
		out += toupper(strIn.at(i));
	}
	return(out);
}
std::string StrToLower(const std::string & strIn)
{
	unsigned int i;
	std::string out;
	out.reserve(strIn.size());
	for (i = 0; i < strIn.size(); i++)
	{
		out += tolower(strIn.at(i));
	}
	return(out);
}
std::string StrsToString(const std::vector< std::string > & vstrIn)
{
	return(StrsToString(vstrIn, " "));
}
// same as above, but lets you specify the separator
std::string StrsToString(const std::vector< std::string > & vstrIn, const std::string & separator)
{
	std::string out;
	out.reserve(vstrIn.size() * 5);	// rough estimate, 5 chars per string
	unsigned int i;
	for (i = 0; i < vstrIn.size(); i++)
	{
		out += vstrIn.at(i) + separator;
	}
	return(out);
}
std::string StrsToString(const std::list< std::string > & lstrIn, const std::string & separator)
{
	std::string out;
	out.reserve(lstrIn.size() * 5);
	std::list< std::string>::const_iterator p;
	for (p = lstrIn.begin(); p != lstrIn.end(); p++)
	{
		out += (*p) + separator;
	}
	return(out);
}
std::string * StrReplace(const std::string & sequence_to_replace,
	const std::string & replacement_string,
	const std::string & strIn,
	std::string * pOut,
	bool bVerbose)
{
	if (pOut == NULL)
		return(pOut);
	if (&strIn == pOut)
	{
		// can't have strIn and pOut point to the same thing
		// have to copy the input string
		std::string incpy = strIn;
		return(StrReplace(sequence_to_replace, replacement_string, incpy, pOut));
	}
	// Similar functionality to CString::Replace(), but no MFC required.
	unsigned int i;
	std::vector< size_t > vpos;
	std::string::size_type pos;
	pos = 0;
	*pOut = "";
	do
	{
		pos = strIn.find(sequence_to_replace, pos);
		if (pos != std::string::npos)
		{
			vpos.push_back(pos);
			pos++;
		}
	} while (pos != std::string::npos);
	if (bVerbose)
	{
		FMsg("found at ");
		for (i = 0; i < vpos.size(); i++)
		{
			FMsg("%d ", vpos.at(i));
		}
		FMsg("\n");
	}
	size_t last_pos = 0;
	size_t repl_size = sequence_to_replace.size();
	pOut->reserve(strIn.size() + vpos.size() * replacement_string.size() - vpos.size() * repl_size);
	// last_pos marks character which should be included
	// in the output string
	for (i = 0; i < vpos.size(); i++)
	{
		*pOut += strIn.substr(last_pos, vpos.at(i) - last_pos);
		last_pos = vpos.at(i) + repl_size;
		*pOut += replacement_string;
	}
	// add remainder of orig string to the end
	*pOut += strIn.substr(last_pos, strIn.size() - last_pos);
	return(pOut);
}
std::string StrReplace(const std::string & sequence_to_replace,
	const std::string & replacement_string,
	const std::string & strIn)
{
	std::string out;
	StrReplace(sequence_to_replace, replacement_string, strIn, &out);
	return(out);
}
std::string StrTokenize(const std::string & strIn, const std::string & delimiters)
{
	// applies strtok repeatedly & acumulates ouput tokens
	// in output string separated by spaces
	return(StrTokenize(strIn, delimiters, " "));
}
std::string StrTokenize(const std::string & strIn,
	const std::string & delimiters,
	const std::string & output_separator)
{
	// Same as above, but allows you to specify the separator between
	// tokens in the output
	// No trailing separator is added
	std::string out;
	char * token;
	char * copy = new char[strIn.size() + 2];	// +2 just to be safe!
	strcpy(copy, strIn.c_str());
	token = strtok(copy, delimiters.c_str());
	while (token != NULL)
	{
		out += token;
		token = strtok(NULL, delimiters.c_str());
		// If there's another token, add the separator
		// This means there is no trailing separator.
		if (token != NULL)
		{
			out += output_separator;
		}
	}
	delete[] copy;
	return(out);
}
sfsizet StrFindCaseless(const std::string & strSearchIn,
	sfsizet start_pos,
	const std::string & strSearchFor)
{
	sfsizet outpos = std::string::npos;
	std::string sin = StrToUpper(strSearchIn);
	std::string sfor = StrToUpper(strSearchFor);
	outpos = sin.find(sfor, start_pos);
	return(outpos);
}
std::vector< sfsizet > StrFindAllCaseless(const std::string & strSearchIn,
	sfsizet start_pos,
	const std::string & strSearchFor)
{
	std::vector< sfsizet > out;
	sfsizet pos = start_pos;
	std::string sin = StrToUpper(strSearchIn);
	std::string sfor = StrToUpper(strSearchFor);
	while (pos != std::string::npos)
	{
		pos = sin.find(sfor, pos);
		if (pos != std::string::npos)
		{
			out.push_back(pos);
			pos++;
		}
	}
	return(out);
}
std::vector< sfsizet > StrFindAll(const std::string & strSearchIn,
	sfsizet start_pos,
	const std::string & strSearchFor)
{
	std::vector< sfsizet > out;
	sfsizet pos = start_pos;
	while (pos != std::string::npos)
	{
		pos = strSearchIn.find(strSearchFor, pos);
		if (pos != std::string::npos)
		{
			out.push_back(pos);
			pos++;
		}
	}
	return(out);
}
std::vector< std::string> StrSort(const std::vector< std::string > & vStrIn)
{
	std::vector< std::string > vOut;
	std::list< std::string > lstr;
	unsigned int i;
	for (i = 0; i < vStrIn.size(); i++)
	{
		lstr.push_back(vStrIn.at(i));
	}
	lstr.sort();
	std::list< std::string>::const_iterator p;
	for (p = lstr.begin(); p != lstr.end(); p++)
	{
		vOut.push_back(*p);
	}
	return(vOut);
}
//-------------------------------------------------------------------
// Reduce a full or relative path name to just a lone filename.
// Example:
// name = GetFilenameFromFullPath( "C:\MyFiles\project1\texture.dds" );
// name is "texture.dds"
// Returns the input string if no path info was found & removed.
//-------------------------------------------------------------------
tstring GetFilenameFromFullPath(const tstring & full_path)
{
	size_t pos;
	pos = full_path.find_last_of('\\', full_path.length());
	if (pos == tstring::npos)
		pos = full_path.find_last_of('/', full_path.length());
	if (pos == tstring::npos)
		return(full_path);
	tstring name;
	name = full_path.substr(pos + 1, full_path.length());
	return(name);
}
