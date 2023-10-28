#include "hv/HttpServer.h"
using namespace hv;
#include <chrono>   //chrono C++20
#include <pqxx/pqxx> //postgres libpqxx C++

int main() {
  HttpService router;
  router.GET("/ping", [](HttpRequest* req, HttpResponse* resp) {
    return resp->String("pong");
    });

  router.GET("/data", [](HttpRequest* req, HttpResponse* resp) {
    static char data[] = "0123456789";
    return resp->Data(data, 10);
    });

  router.GET("/paths", [&router](HttpRequest* req, HttpResponse* resp) {
    return resp->Json(router.Paths());
    });

  router.GET("/get", [](HttpRequest* req, HttpResponse* resp) {
    resp->json["origin"] = req->client_addr.ip;
    resp->json["url"] = req->url;
    resp->json["args"] = req->query_params;
    resp->json["headers"] = req->headers;
    return 200;
    });

  router.POST("/echo", [](const HttpContextPtr& ctx) {
    return ctx->send(ctx->body(), ctx->type());
    });

  router.GET("/api", [](HttpRequest* req, HttpResponse* resp) {
    auto start = std::chrono::steady_clock::now();
    try
    {
      pqxx::connection conn("user=pxmcea host=166.99.230.91 password=Security*8 dbname=telemetry");
      pqxx::transaction tx{ conn };
      for (auto const& [k, c] : tx.stream<std::string_view, long>("SELECT 'count', count(*) c FROM telemetry"))
      {
        //std::cout << att << ": " << count << '\n';
        resp->json["count"] = c;
      }
      conn.close();
    }
    catch (std::exception const& e)
    {
      std::cerr << "ERROR: " << e.what() << '\n';
      return 400;
    }
    resp->json["origin"] = req->client_addr.ip;
    resp->json["url"] = req->url;
    resp->json["args"] = req->query_params;
    resp->json["headers"] = req->headers;
    auto t0 = std::chrono::steady_clock::now() - start;
    resp->json["timer_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(t0).count();
    return 200;
    });

  HttpServer server = HttpServer();
  server.registerHttpService(&router);
  server.setPort(8080);
  server.setThreadNum(4);
  server.run();
  return 0;
}

//sample 1
/*
pqxx::result r{ txn.exec("SELECT name, salary FROM Employee") };
for (auto row : r)
std::cout << row["name"].c_str() << " makes " // Address column by name.  Use c_str() to get C-style string.
<< row[1].as<int>() << "$." << std::endl; // Address column by zero-based index.  Use as<int>() to parse as int.
*/
//sample 2
/*
  long total = 0;
  for (auto [salary] : txn.query("SELECT salary FROM Employee"))
    total += salary;
  std::cout << "Total salary: " << total << '\n';
*/