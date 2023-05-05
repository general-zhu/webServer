#include "sylar/http/http_server.h"
#include "sylar/log.h"

static sylar::Logger::Ptr g_logger = LOG_ROOT();

void run() {
  sylar::http::HttpServer::Ptr server(new sylar::http::HttpServer);
  sylar::Address::Ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
  // std::cout << addr->ToString() << std::endl;
  while (!server->Bind(addr)) {
    sleep(2);
  }
  auto sd = server->GetServletDispatch();
  sd->AddServlet("/sylar/xx", [](sylar::http::HttpRequest::Ptr req,
                               sylar::http::HttpResponse::Ptr rsp,
                               sylar::http::HttpSession::Ptr session) {
    rsp->SetBody(req->ToString());
    return 0;
  });
  sd->AddGlobServlet("/sylar/*", [](sylar::http::HttpRequest::Ptr req,
                               sylar::http::HttpResponse::Ptr rsp,
                               sylar::http::HttpSession::Ptr session) {
    rsp->SetBody("Glob:\r\n" + req->ToString());
    return 0;
  });
  server->Start();
}

int main() {
  sylar::IOManager iom(2);
  iom.Schedule(run);
  return 0;
}
