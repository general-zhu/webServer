cc_library(
    name = "muduo_base",
    srcs = [
        "AsyncLogging.cc",
        "Condition.cc",
        "CountDownLatch.cc",
        "CurrentThread.cc",
        "Date.cc",
        "Exception.cc",
        "FileUtil.cc",
        "LogFile.cc",
        "LogStream.cc",
        "Logging.cc",
        "ProcessInfo.cc",
        "Thread.cc",
        "ThreadPool.cc",
        "TimeZone.cc",
        "Timestamp.cc",
    ],
    deps = [
        "#pthread",
    ],
    hdrs = ([
        "Thread.h",
        "AsyncLogging.h",
        "Condition.h",
        "CountDownLatch.h",
        "CurrentThread.h",
        "Date.h",
        "Exception.h",
        "FileUtil.h",
        "LogFile.h",
        "LogStream.h",
        "Logging.h",
        "ProcessInfo.h",
        "ThreadPool.h",
        "TimeZone.h",
        "Timestamp.h",
        "noncopyable.h",
        "WeakCallback.h"
    ]),
    extra_cppflags = [
        "-Wno-format-nonliteral",
        "-Wno-vla",
    ],
    visibility = ['PUBLIC'],
)

cc_binary(
  name = 'muduo_test',
  srcs = 'muduo_test.cc',
  deps = [
      ":muduo_base"
  ],
)
