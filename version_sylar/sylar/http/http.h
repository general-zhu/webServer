#ifndef SYLAR_HTTP_HTTP_H_
#define SYLAR_HTTP_HTTP_H_
#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>

namespace sylar {
namespace http {

/* Request Methods */
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       \

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \

enum class HttpMethod {
#define XX(num, name, string) name = num,
  HTTP_METHOD_MAP(XX)
#undef XX
  INVALID_METHOD
};

enum class HttpStatus {
#define XX(code, name, desc) name = code,
  HTTP_STATUS_MAP(XX)
#undef XX
};

HttpMethod StringToHttpMethod(const std::string& m);
HttpMethod CharsToHttpMethod(const char* m);
const char* HttpMethodToString(const HttpMethod& m);
const char* HttpStatusToString(const HttpStatus& s);

struct CaseInsensitiveLess {
  bool operator()(const std::string& lhs, const std::string& rhs) const;
};

template<class MapType, class T>
bool CheckGetAs(const MapType& m, const std::string& key, T& val, const T& def = T()) {
  auto it = m.find(key);
  if (it == m.end()) {
    val = def;
    return false;
  }
  try {
    val = boost::lexical_cast<T>(it->second);
    return true;
  } catch (...) {
    val = def;
  }
  return false;
}

template<class MapType, class T>
T GetAs(const MapType& m, const std::string& key, const T& def = T()) {
  auto it = m.find(key);
  if (it == m.end()) {
    return def;
  }
  try {
    return boost::lexical_cast<T>(it->second);
  } catch (...) {
  }
  return def;
}

class HttpResponse;

class HttpRequest {
 public:
  typedef std::shared_ptr<HttpRequest> Ptr;
  typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;

  HttpRequest(uint8_t version = 0x11, bool close = true);
  std::shared_ptr<HttpResponse> CreateResponse();

  HttpMethod GetMethod() const { return method_; }
  uint8_t GetVersion() const { return version_; }
  const std::string& GetPath() const { return path_; }
  const std::string& GetQuery() const { return query_; }
  const std::string& GetBody() const { return body_; }
  const MapType& GetHeaders() const { return headers_; }
  const MapType& GetParams() const { return params_; }
  const MapType& GetCookies() const { return cookies_; }

  void SetMethod(HttpMethod v) { method_ = v; }
  void SetVersion(uint8_t v) { version_ = v; }
  void SetPath(const std::string& v) { path_ = v; }
  void SetQuery(const std::string& v) { query_ = v; }
  void SetFragment(const std::string& v) { fragment_ = v; }
  void SetBody(const std::string& v) { body_ = v; }
  bool IsClose() const { return close_; }
  void SetClose(bool v) { close_ = v; }
  bool IsWebsocket() const { return websocket_; }
  void SetWebsocket(bool v) { websocket_ = v; }
  void SetHeaders(const MapType& v) { headers_ = v; }
  void SetParams(const MapType& v) { params_ = v; }
  void SetCookies(const MapType& v) { cookies_ = v; }

  std::string GetHeader(const std::string& key, const std::string& def = "") const;
  std::string GetParam(const std::string& key, const std::string& def = "");
  std::string GetCookies(const std::string& key, const std::string& def = "");
  void SetHeader(const std::string& key, const std::string& val);
  void SetParam(const std::string& key, const std::string& val);
  void SetCookies(const std::string& key, const std::string& val);
  void DelHeader(const std::string& key);
  void DelParam(const std::string& key);
  void DelCookie(const std::string& key);
  bool HasHeader(const std::string& key, std::string* val = nullptr);
  bool HasParam(const std::string& key, std::string* val = nullptr);
  bool HasCookie(const std::string& key, std::string* val = nullptr);

  template<class T>
  bool CheckGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
    return CheckGetAs(headers_, key, val, def);
  }

  template<class T>
  T GetHeaderAs(const std::string& key, const T& def = T()) {
    return GetAs(headers_, key, def);
  }

  template<class T>
  bool CheckGetParamAs(const std::string& key, T& val, const T& def = T()) {
    InitQueryParam();
    InitBodyParam();
    return CheckGetAs(params_, key, val, def);
  }

  template<class T>
  bool GetParamAs(const std::string& key, const T& def = T()) {
    InitQueryParam();
    InitBodyParam();
    return GetAs(params_, key, def);
  }

  template<class T>
  bool CheckGetCookieAs(const std::string& key, T& val, const T& def = T()) {
    InitCookies();
    return CheckGetAs(cookies_, key, val, def);
  }

  template<class T>
  bool GetCookieAs(const std::string& key, const T& def = T()) {
    InitCookies();
    return GetAs(cookies_, key, def);
  }

  std::ostream& Dump(std::ostream& os) const;
  std::string ToString() const;
  void Init();
  void InitParam();
  void InitQueryParam();
  void InitBodyParam();
  void InitCookies();

 private:
  HttpMethod method_;
  uint8_t version_;
  bool close_;
  bool websocket_;

  uint8_t parser_param_flag;
  std::string path_;
  std::string query_;
  std::string fragment_;
  std::string body_;
  MapType headers_;
  MapType params_;
  MapType cookies_;
};

class HttpResponse {
 public :
  typedef std::shared_ptr<HttpResponse> Ptr;
  typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;

  HttpResponse(uint8_t version = 0x11, bool close = true);
  HttpStatus GetStatus() const { return status_; }
  uint8_t GetVersion() const { return version_; }
  const std::string& GetBody() const { return body_; }
  const std::string& GetReason() const { return reason_; }
  const MapType& GetHeaders() const { return headers_; }

  void SetStatus(HttpStatus v) { status_ = v; }
  void SetVersion(uint8_t v) { version_ = v; }
  void SetBody(const std::string& v) { body_ = v; }
  void SetReason(const std::string& v) { reason_ = v; }
  void SetHeaders(const MapType& v) { headers_ = v; }
  bool IsClose() const { return close_; }
  void SetClose(bool v) { close_ = v; }
  bool IsWebsocket() const { return websocket_; }
  void SetWebsocket(bool v) { websocket_ = v; }

  std::string GetHeader(const std::string& key, const std::string& def = "") const;
  void SetHeader(const std::string& key, const std::string& val);
  void DelHeader(const std::string& key);

  template<class T>
  bool CheckGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
    return CheckGetAs(headers_, key, val, def);
  }

  template<class T>
  T GetHeaderAs(const std::string& key, const T& def = T()) {
    return GetAs(headers_, key, def);
  }

  std::ostream& Dump(std::ostream& os) const;
  std::string ToString() const;
  void SetRedirect(const std::string& url);
  void SetCookie(const std::string& key, const std::string& val, time_t expired = 0,
      const std::string& path = "", const std::string& domain = "", bool secure = false);

 private:
  HttpStatus status_;
  uint8_t version_;
  bool close_;
  bool websocket_;
  std::string body_;
  std::string reason_;
  MapType headers_;
  std::vector<std::string> cookies_;
};

std::ostream& operator<<(std::ostream& os, const HttpRequest& req);
std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp);

}  // namespace http
}  // namespace sylar
#endif  // SYLAR_HTTP_HTTP_H_
