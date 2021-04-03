/**
 * ______________________________________________________
 * This file is part of ko-smd-generator project.
 * 
 * @author       Mustafa Kemal Gılor <mustafagilor@gmail.com> (2016)
 * .
 * SPDX-License-Identifier:	MIT
 * ______________________________________________________
 */

#include "tstring.h"
#include <cstdarg>
#include <cctype>
#include <boost/algorithm/string.hpp>

void _string_format(const std::string fmt, std::string * result, va_list args) {
	char buffer[1024];
	_vsnprintf(buffer, sizeof(buffer), fmt.c_str(), args);
	*result = buffer;
}

std::string string_format(const std::string fmt, ...) {
	std::string result;
	va_list ap;

	va_start(ap, fmt);
	_string_format(fmt, &result, ap);
	va_end(ap);

	return result;
}

bool string_isvalidalpha(std::string_view string) {
	for (auto c : string) {
		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
			/*
			Allow this :) I like it personally.
			*/
			if (c == '_')
				continue;
			return false;
		}
	}
	return true;
}


template<int(&F)(int)> int safe_ctype(unsigned char c) { return F(c); }
static int safe_isspace(int c) { return safe_ctype<std::isspace>(c); }

bool is_whitespace_only(const std::string & value) {
	return value.find_first_not_of(' ') == std::string::npos;
}

// trim from end
std::string & rtrim(std::string &s) {
	boost::algorithm::trim_right(s);
	return s;
}

// trim from start
std::string & ltrim(std::string &s) {
	boost::algorithm::trim_left(s);
	return s;
}

void tstrcpy(char *dst, size_t len, std::string & src) {
	memset(dst, 0x00, len);
	memcpy(dst, src.c_str(), src.length() > len ? len : src.length());
}

#define _tstrcpy(dst, src) tstrcpy(dst, sizeof(dst), src)

#ifdef UNICODE
string_format(const tstring fmt, ...) {
	TCHAR buffer[1024];
	va_list ap;

	va_start(ap, fmt);
	_vsntprintf(buffer, sizeof(buffer), fmt.c_str(), ap);
	va_end(ap);

	return buffer;
}

tstring & rtrim(tstring &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(safe_isspace))).base(), s.end());
	return s;
}

void tstrcpy(TCHAR *dst, size_t len, tstring & src) {
	memset(dst, 0x00, len);
	memcpy(dst, src.c_str(), src.length() > len ? len : src.length());
}
#endif
