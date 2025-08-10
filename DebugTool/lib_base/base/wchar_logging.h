#ifndef WCHAR_LOGGING_H
#define WCHAR_LOGGING_H

// 把wchar_t转换为char*，宽字符转换为UTF8输出

#include <wchar.h>

#include <iostream>
#include <string>

#include "encode_helper.h"

// 转换为本地ANSIC编码
// char* old_locale = _strdup(setlocale(LC_CTYPE, NULL));  //store the old
// locale setlocale(LC_CTYPE,
//          setlocale(LC_ALL, ""));  // using the locale of the user env.
// mbstate_t state;
// memset(&state, 0, sizeof state);
// size_t len = wcsrtombs(NULL, &str, 0, &state);
// char* buf = (char*)malloc(len + 1);
// buf[len] = 0;
// wcsrtombs(buf, &str, len, &state);
// out << buf;
// free(buf);
// setlocale(LC_CTYPE, old_locale);  // restore the old locale.
// free(old_locale);

// 转utf8
inline std::ostream &operator<<(std::ostream &out, const wchar_t *str) {
  if (str == nullptr)
    return out;

  std::string narrowStr = encode_helper::Unicode2Utf8(str);
  out << narrowStr;
  return out;
}

inline std::ostream &operator<<(std::ostream &out, const std::wstring &str) {
  return operator<<(out, str.c_str());
}

#ifdef QSTRING_H
inline std::ostream &operator<<(std::ostream &out, const QString &str) {
  return operator<<(out, str.toStdString());
}

#endif // QSTRING_H

#endif // WCHAR_LOGGING_H_
