cc_library(
    name = 'http_request',
    srcs = [
        'http_request.cc'
    ],
    hdrs = [
        'http_request.h'
    ],
    deps = [
        '//thirdparty/muduo:muduo_base',
    ],
)

cc_library(
    name = 'http_response',
    srcs = [
        'http_response.cc',
    ],
    hdrs = [
        'http_response.h',
    ],
    deps = [
        '//thirdparty/muduo:muduo_net',
    ]
)

cc_library(
    name = 'http_context',
    srcs = [
        'http_context.cc',
    ],
    hdrs = [
        'http_context.h',
    ],
    deps = [
        ':http_request',
        '//thirdparty/muduo:muduo_base',
        '//thirdparty/muduo:muduo_net',
    ],
)

cc_library(
    name = 'http_server',
    srcs = [
        'http_server.cc'
    ],
    hdrs = [
        'http_server.h'
    ],
    deps = [
        ':http_request',
        ':http_context',
        ':http_response',
        '//thirdparty/muduo:muduo_base',
    ],
)

cc_binary(
    name = 'muduo_test',
    srcs = 'muduo_test.cc',
    deps = [
      '//thirdparty/muduo:muduo_base',
    ],
)

cc_binary(
    name = 'test_http_server',
    srcs = 'test_http_server.cc',
    deps = [
        ':http_server',
        ':http_request',
        ':http_response',
        '//thirdparty/muduo:muduo_net',
    ],
)

cc_library(
    name = 'inspector',
    srcs = [
        'inspector.cc',
        'process_inspector.cc',
    ],
    hdrs = [
        'inspector.h',
        'process_inspector.h',
    ],
    deps = [
        ':http_server',
        ':http_response',
        ':http_request',
        '//thirdparty/muduo:muduo_base',
        '//thirdparty/muduo:muduo_net',
    ],
)

cc_binary(
    name = 'main',
    srcs = 'main.cc',
    deps = [
        ':inspector',
        '//thirdparty/muduo:muduo_net',
    ]
)

cc_binary(
    name = 'request_test',
    srcs = 'request_test.cc',
    deps = ':http_request',
)
