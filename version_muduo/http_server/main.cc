/**
 * Copyright(c) All rights reserved
 * Author: 244553899@qq.com
 * Time:   2022-06-28 15:01
 */

#include "http_server/inspector.h"
#include "thirdparty/muduo/net/EventLoop.h"
#include "thirdparty/muduo/net/EventLoopThread.h"

int main(int argc, char** argv) {
  muduo::net::EventLoop loop;
  muduo::net::EventLoopThread et;
  http::Inspector inspector(et.startLoop(), muduo::net::InetAddress(12345), "main");
  loop.loop();
}
