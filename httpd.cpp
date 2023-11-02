#include <iostream>    //cout
#include <chrono>      //chrono C++20
#include <string_view>
#include <pqxx/pqxx>   //postgres libpqxx C++
#include <string>      //std::string

#include "hv/HttpServer.h"
#include "hv/hv.h"     //get_ncpu
//#include "hv/json.hpp"
using namespace hv;

int main(int argc, char* argv[]) {
  uint16_t i = 1, k = 0, id = 0, assetid = 0, postgres = 0;
  char hhmm[6] = "00:02";
  char f[128] = "../../../config-httpd.json";
  char ip[16] = "0.0.0.0";
  int port = 0;

  while (i < argc) {
    if (strcmp(argv[i], "--runfor") == 0) { ++i; if (strlen(argv[i]) != 5) break; else { if (strlen(argv[i]) < sizeof(hhmm)) { strcpy(hhmm, argv[i]); k++; } } }
    if (strcmp(argv[i], "--port") == 0) { ++i; port = atoi(argv[i]); k++; }
    if (strcmp(argv[i], "--ip") == 0) { ++i; if (strlen(argv[i]) < sizeof(ip)) { strcpy(ip, argv[i]); k++; } }
    if (strcmp(argv[i], "--config") == 0) { ++i; if (strlen(argv[i]) < sizeof(f)) { strcpy(f, argv[i]); k++; } }
    ++i;
  }
  if ((argc - 1) / 2 != k || k == 0) { std::cout << "httpd - by Guy Francoeur (c) 2023\nUsage : " << argv[0] << "\n\t{--runfor 00:30}\n\t{--ip 0.0.0.0}\n\t{--port 502}\n\t{--config ./file.json}\n--runfor\tHourMinute the application will be running. It stops after HH:MM human time;\n--id    \tThe configuration ID to use;\n--assetid \tThe asset ID to use;\n--config\tThe json configuration file to use;\n\n"; return 0; }

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
    pqxx::connection conn("user=pxmcea host=166.99.230.91 password=Security*8 dbname=telemetry");
    pqxx::work tx{ conn };
    try
    {
      pqxx::result rows{ tx.exec("SELECT 'rows' as k, count(*) c FROM telemetry") };
      for (auto row : rows) {
        resp->json["rows"] = row[1].as<long>();
      }
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
    resp->json["ztimer_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(t0).count();
    std::cout << "/api\t" << "in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << "ms\n";
    return 200;
    });

  router.GET("/live", [](HttpRequest* req, HttpResponse* resp) {
    auto start = std::chrono::steady_clock::now();
    auto t0 = std::chrono::steady_clock::now() - start;
    pqxx::connection conn("user=pxmcea host=166.99.230.91 password=Security*8 dbname=telemetry");
    pqxx::work tx{ conn };
    try
    {
      pqxx::result rows{ tx.exec("select max(ts) ts, att, round(avg(val),5) val from telemetry where ts > (now() at time zone 'UTC' - interval '30' day) and att in(25,23,37,35,33,107,105) group by att") };
      t0 = std::chrono::steady_clock::now() - start;
      std::string s;
      uint32_t i = 0;
      s.append("[");
      for (auto row : rows) {
        if (i++ != 0) { s.append(","); }

        s.append("{\"ts\":\""); s.append(row[0].c_str()); s.append("\",");
        s.append("\"att\":"); s.append(row[1].c_str()); s.append(",");
        s.append("\"val\":"); s.append(row[2].c_str()); s.append("}");
      }
      s.append("]"); //std::cout << s;
      auto j1 = nlohmann::json::parse(s);
      resp->json["points"] = j1;
    }
    catch (std::exception const& e)
    {
      std::cerr << "ERROR: " << e.what() << '\n';
      return 400;
    }

    //req->ParseBody();
    //resp->content_type = APPLICATION_JSON;
    //resp->json["req"] = req->json;
    //resp->json["body"] = req->body;
    resp->json["origin"] = req->client_addr.ip;
    resp->json["url"] = req->url;
    resp->json["args"] = req->query_params;
    resp->json["headers"] = req->headers;
    auto t1 = std::chrono::steady_clock::now() - start;
    resp->json["zquery_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(t0).count();
    resp->json["zparser_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(t1).count() - std::chrono::duration_cast<std::chrono::milliseconds>(t0).count();
    std::cout << "/live\t" << "in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << "ms\n";
    return 200;
    });

  HttpServer server = HttpServer();
  server.registerHttpService(&router);
  server.setPort(port);
  server.setHost(ip);
  server.setThreadNum(get_ncpu()); //get_ncpu()
  std::cout << "listening on " << ip << ":" << port << " (" << get_ncpu() << ")\n";
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

// Sample 3
      //for (auto const& [k, c] : tx.query("SELECT 'rows' as k, count(*) c FROM telemetry")) {
      //  resp->json["rows"] = c;
      //}

      //for (auto const& [k, c] : tx.stream<std::string_view, long>("SELECT 'count' as k, count(*) c FROM telemetry"))
      //{
      //  //std::cout << att << ": " << count << '\n';
      //  resp->json["count"] = c;
      //}