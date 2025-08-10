#ifndef LIB_BASE_GLOG_HELPER_H
#define LIB_BASE_GLOG_HELPER_H

// copyright 2024 YottaImage. All rights reserved.
// author panming
// date 2024/10/12 17:50
#ifndef GLOG_GLOG_HELPER_H
#define GLOG_GLOG_HELPER_H

#ifndef GLOG_USE_GLOG_EXPORT
#define GLOG_USE_GLOG_EXPORT
#endif // !GLOG_USE_GLOG_EXPORT
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
// #include <glog/wchar_logging.h>
#include "wchar_logging.h"

namespace glog_helper {

// log_dir：waring和error等日志级别的输出目录
// file_name：INFOG级别的日志输出文件名，不带扩展名，最好和log_dir同样的全路径
__inline void InitGLog(const char *argv0, const std::string &log_dir,
                       const std::string &file_name) {
  using namespace google;
  FLAGS_log_dir = log_dir;
  FLAGS_alsologtostderr = true;
  FLAGS_timestamp_in_logfile_name = false;
  FLAGS_colorlogtostdout = true;
  FLAGS_colorlogtostderr = true;
  InitGoogleLogging(argv0);
  SetLogDestination(GLOG_INFO, file_name.c_str());
  SetLogFilenameExtension(".log");
}
} // namespace glog_helper
#pragma comment(lib, "dbghelp.lib")
#if _MSC_VER >= 1920 // Visual C++ 2019 16.0(1920)-16.9（1929）
#ifdef _WIN64
#ifdef _DEBUG
#ifdef _DLL
#pragma comment(lib, "glog/glog_mdd_64.lib")
#else
#pragma comment(lib, "glog/glog_mtd_64.lib")
#endif
#else
#ifdef _DLL
#pragma comment(lib, "glog/glog_md_64.lib")
#else
#pragma comment(lib, "glog/glog_mt_64.lib")
#endif
#endif
#else
#ifdef _DEBUG
#ifdef _DLL
#pragma comment(lib, "glog/glog_mdd_32.lib")
#else
#pragma comment(lib, "glog/glog_mtd_32.lib")
#endif
#else
#ifdef _DLL
#pragma comment(lib, "glog/glog_md_32.lib")
#else
#pragma comment(lib, "glog/glog_mt_32.lib")
#endif
#endif
#endif //_WIN64
#endif // Visual C++ 2019

#endif // GLOG_GLOG_HELPER_H

#endif
