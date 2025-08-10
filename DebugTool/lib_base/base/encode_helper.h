#ifndef YOTTA_PUBLIC_COMMON_ENCODE_HELPER_H
#define YOTTA_PUBLIC_COMMON_ENCODE_HELPER_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

namespace encode_helper {
__inline std::string Unicode2Utf8(const wchar_t *wstr, int cp = CP_UTF8) {
  if (!wstr || !wstr[0])
    return "";

  int len = WideCharToMultiByte(cp, 0, wstr, -1, NULL, 0, NULL, NULL);
  if (len <= 0)
    return "";

  char *buf = new (std::nothrow) char[len + 1];
  if (buf == NULL)
    return "";

  len = WideCharToMultiByte(cp, 0, wstr, -1, buf, len, NULL, NULL);
  if (len <= 0) {
    delete[] buf;
    return "";
  }
  buf[len] = '\0';

  std::string str = buf;
  delete[] buf;

  return str;
}

__inline std::wstring Utf82Unicode(const char *str, int cp = CP_UTF8) {
  if (!str || !str[0])
    return L"";

  int len = MultiByteToWideChar(cp, 0, str, -1, NULL, 0);
  if (len <= 0)
    return L"";

  WCHAR *buf = new (std::nothrow) WCHAR[len + 1];
  if (buf == NULL)
    return L"";
  memset(buf, 0, (len + 1) * sizeof(WCHAR));

  len = MultiByteToWideChar(cp, 0, str, -1, buf, len);
  if (len <= 0) {
    delete[] buf;
    return L"";
  }
  buf[len] = 0;

  std::wstring wstr = buf;
  delete[] buf;

  return wstr;
}
} // namespace encode_helper

#endif // YOTTA_PUBLIC_COMMON_ENCODE_HELPER_H
