#include "http_server.h"
#include "sylar/log.h"

namespace sylar {
namespace http {

static sylar::Logger::Ptr g_logger = LOG_NAME("system");

HttpServer::HttpServer(bool keepalive, sylar::IOManager* worker,
    sylar::IOManager* io_worker, sylar::IOManager* accept_worker) :
    TcpServer(worker, io_worker, accept_worker),
    is_keepalive_(keepalive) {
  dispatch_.reset(new ServletDispatch);
  type_ = "http";
  // dispatch_->AddServlet("/_/status", Servlet::Ptr(new StatusServlet));
  // dispatch_->AddServlet("/_/config", Servlet::Ptr(new ConfigServlet));
}

void HttpServer::SetName(const std::string& v) {
  TcpServer::SetName(v);
  dispatch_->SetDefault(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::HandleClient(Socket::Ptr client) {
  LOG_DEBUG(g_logger) << "HandleClient " << *client;
  HttpSession::Ptr session(new HttpSession(client));
  do {
    auto req = session->RecvRequest();
    if (!req) {
      LOG_DEBUG(g_logger) << "recv http request fail, errno=" << errno << " errstr="
          << strerror(errno) << " client:" << *client << " keep_alive=" << is_keepalive_;
      break;
    }
    HttpResponse::Ptr rsp(new HttpResponse(req->GetVersion(),req->IsClose() || !is_keepalive_));
    rsp->SetHeader("Server", GetName());
    dispatch_->Handle(req, rsp, session);
    session->SendResponse(rsp);
    if (!is_keepalive_ || req->IsClose()) {
      break;
    }
  } while(true);
  session->Close();
}

}  // namespace http
}  // namespace sylar

