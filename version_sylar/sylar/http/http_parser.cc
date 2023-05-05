#include "http_parser.h"
#include "sylar/config.h"
#include "sylar/log.h"

namespace sylar {
namespace http {

static sylar::Logger::Ptr g_logger = LOG_NAME("system");
static sylar::ConfigVar<uint64_t>::Ptr g_http_request_buffer_size = sylar::Config::Lookup(
    "http.request.buffer_size", (uint64_t)(4 * 1024), "http request buffer size");
static sylar::ConfigVar<uint64_t>::Ptr g_http_request_max_body_size = sylar::Config::Lookup(
    "http.request.max_body_size", (uint64_t)(64 * 1024 * 1024), "http request max body size");
static sylar::ConfigVar<uint64_t>::Ptr g_http_response_buffer_size = sylar::Config::Lookup(
    "http.response.buffer_size", (uint64_t)(4 * 1024), "http response buffer size");
static sylar::ConfigVar<uint64_t>::Ptr g_http_response_max_body_size = sylar::Config::Lookup(
    "http.response.max_body_size", (uint64_t)(64 * 1024 * 1024), "http response max body size");

static uint64_t s_http_request_buffer_size = 0;
static uint64_t s_http_request_max_body_size = 0;
static uint64_t s_http_response_buffer_size = 0;
static uint64_t s_http_response_max_body_size = 0;


uint64_t HttpRequestParser::GetHttpRequestBufferSize() {
  return s_http_request_buffer_size;
}

uint64_t HttpRequestParser::GetHttpRequestMaxBodySize() {
  return s_http_request_max_body_size;
}

uint64_t HttpResponseParser::GetHttpResponseBufferSize() {
  return s_http_response_buffer_size;
}

uint64_t HttpResponseParser::GetHttpResponseMaxBodySize() {
  return s_http_response_max_body_size;
}

namespace {
struct _RequestSizeIniter {
  _RequestSizeIniter() {
    s_http_request_buffer_size = g_http_request_buffer_size->GetValue();
    s_http_request_max_body_size = g_http_request_max_body_size->GetValue();
    s_http_response_buffer_size = g_http_response_buffer_size->GetValue();
    s_http_response_max_body_size = g_http_response_max_body_size->GetValue();

    g_http_request_buffer_size->AddListener([](const uint64_t& ov, const uint64_t& nv) {
        s_http_request_buffer_size = nv;
        });
    g_http_request_max_body_size->AddListener([](const uint64_t& ov, const uint64_t& nv) {
        s_http_request_max_body_size = nv;
        });
    g_http_response_buffer_size->AddListener([](const uint64_t& ov, const uint64_t& nv) {
        s_http_response_buffer_size = nv;
        });
    g_http_response_max_body_size->AddListener([](const uint64_t& ov, const uint64_t& nv) {
        s_http_response_max_body_size = nv;
        });
  }
};
static _RequestSizeIniter _init;
}

void on_request_method(void* data, const char* at, size_t length) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  HttpMethod m = CharsToHttpMethod(at);

  if (m == HttpMethod::INVALID_METHOD) {
    LOG_WARN(g_logger) << "invalid http request method: " << std::string(at, length);
    parser->SetError(1000);
    return;
  }
  parser->GetData()->SetMethod(m);
}

void on_request_uri(void* data, const char* at, size_t length) {
}

void on_request_fragment(void* data, const char* at, size_t length) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  parser->GetData()->SetFragment(std::string(at, length));
}

void on_request_path(void* data, const char* at, size_t length) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  parser->GetData()->SetPath(std::string(at, length));
}

void on_request_query(void* data, const char* at, size_t length) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  parser->GetData()->SetQuery(std::string(at, length));
}

void on_request_version(void* data, const char* at, size_t length) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  uint8_t v = 0;
  if (strncmp(at, "HTTP/1.1", length) == 0) {
    v = 0x11;
  } else if (strncmp(at, "HTTP/1.0", length) == 0) {
    v = 0x10;
  } else {
    LOG_WARN(g_logger) << "invalid http request version: " << std::string(at, length);
    parser->SetError(1001);
    return;
  }
  parser->GetData()->SetVersion(v);
}

void on_request_header_done(void* data, const char* at, size_t length) {
}

void on_request_http_field(void* data, const char*field, size_t flen,
    const char* value, size_t vlen) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  if (flen == 0) {
    LOG_WARN(g_logger) << "invalid http request field length == 0";
    return;
  }
  parser->GetData()->SetHeader(std::string(field, flen), std::string(value, vlen));
}

HttpRequestParser::HttpRequestParser() : error_(0) {
  data_.reset(new sylar::http::HttpRequest);
  http_parser_init(&parser_);
  parser_.request_method = on_request_method;
  parser_.request_uri = on_request_uri;
  parser_.fragment = on_request_fragment;
  parser_.request_path = on_request_path;
  parser_.query_string = on_request_query;
  parser_.http_version = on_request_version;
  parser_.header_done = on_request_header_done;
  parser_.http_field = on_request_http_field;
  parser_.data = this;
}

// 1: 成功
// -2: 有错误
// >0: 已处理的字节数，且data有效数据为len - v
size_t HttpRequestParser::Execute(char* data, size_t len) {
  size_t offset = http_parser_execute(&parser_, data, len, 0);
  memmove(data, data + offset, (len - offset));
  return offset;
}

int HttpRequestParser::IsFinished() {
  return http_parser_finish(&parser_);
}

int HttpRequestParser::HasError() {
  return error_ || http_parser_has_error(&parser_);
}

uint64_t HttpRequestParser::GetContentLength() {
  return data_->GetHeaderAs<uint64_t>("content-length", 0);
}

void on_response_reason(void* data, const char* at, size_t length) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  parser->GetData()->SetReason(std::string(at, length));
}

void on_response_status(void* data, const char* at, size_t length) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  HttpStatus status = (HttpStatus)(atoi(at));
  parser->GetData()->SetStatus(status);
}

void on_response_chunk(void* data, const char* at, size_t length) {
}

void on_response_version(void* data, const char* at, size_t length) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  uint8_t v = 0;
  if (strncmp(at, "HTTP/1.1", length) == 0) {
    v = 0x11;
  } else if (strncmp(at, "HTTP/1.0", length) == 0) {
    LOG_WARN(g_logger) << "invalid http response version: " << std::string(at, length);
    parser->SetError(1001);
    return;
  }
  parser->GetData()->SetVersion(v);
}

void on_response_header_done(void* data, const char* at, size_t length) {
}

void on_response_last_chunk(void* data, const char* at, size_t length) {
}

void on_response_http_field(void* data, const char* field, size_t flen,
    const char* value, size_t vlen) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  if (flen == 0) {
    LOG_WARN(g_logger) << "invalid http response field length == 0";
    return;
  }
  parser->GetData()->SetHeader(std::string(field, flen), std::string(value, vlen));
}

HttpResponseParser::HttpResponseParser() : error_(0) {
  data_.reset(new sylar::http::HttpResponse);
  httpclient_parser_init(&parser_);
  parser_.reason_phrase = on_response_reason;
  parser_.status_code = on_response_status;
  parser_.chunk_size = on_response_chunk;
  parser_.http_version = on_response_version;
  parser_.header_done = on_response_header_done;
  parser_.last_chunk = on_response_last_chunk;
  parser_.http_field = on_response_http_field;
  parser_.data = this;
}

size_t HttpResponseParser::Execute(char* data, size_t len, bool chunck) {
  if (chunck) {
    httpclient_parser_init(&parser_);
  }
  size_t offset = httpclient_parser_execute(&parser_, data, len, 0);
  memmove(data, data + offset, (len - offset));
  return offset;
}

int HttpResponseParser::IsFinished() {
  return httpclient_parser_finish(&parser_);
}

int HttpResponseParser::HasError() {
  return error_ || httpclient_parser_has_error(&parser_);
}

uint64_t HttpResponseParser::GetContentLength() {
  return data_->GetHeaderAs<uint64_t>("content-length", 0);
}
}  // namespace http
}  // namespace sylar
