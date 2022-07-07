#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <pistache/client.h>
#include <pistache/net.h>

#include "../common/tracer.hpp"

using namespace Pistache;
using namespace Rest;

class Service
{
  private:
    Router router;
    std::shared_ptr<Http::Endpoint> httpEndpoint;

    Address address;
    Http::Client httpClient;    

    const std::string serviceName = "service-a";

    void setupRoutes()
    {        
        Routes::Get(router, "/ping", Routes::bind(&Service::ping, this));        
    }

    void initEndpoint()
    {
      auto opts = Http::Endpoint::options().threads(4); 
      httpEndpoint->init(opts);
    }

    void initHttpClient() 
    {
      auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
      httpClient.init(opts);
    }     

    void sendPingToAnotherService(std::string hostName, std::string svcName) 
    {
      auto resp = httpClient.get(hostName).send();
      
      resp.then(
        [&](Http::Response response) {
          std::cout << "Response from " << svcName << ":\n";
          std::cout << "Code = " << response.code() << std::endl;
          auto body = response.body();
          if (!body.empty())
              std::cout << "Body = " << body << std::endl;
        }, 
        [&](std::exception_ptr exc) {
          std::cout << "Error..." << std::endl;
      });
    }

  public:
    explicit Service(Address address)       
    {        
      this->address = address;
      httpEndpoint = std::make_shared<Http::Endpoint>(address);
    }

    void initAndStart()
    {
        initEndpoint();
        setupRoutes();
        initHttpClient();
                
        setUpTracer(true);

        std::cout << "Listening at: " << address.host() << 
          ":" << address.port().toString() << std::endl;

        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();        
    }    

    void ping(const Rest::Request& request, Http::ResponseWriter writer)
    {
      std::cout << "\n---=== " << serviceName << "===---\n";

      trace(serviceName, serviceName + ": received ping");

      trace(serviceName, "sending ping to service-b");
     
      sendPingToAnotherService("http://service-b:8082/ping", "service-b");
      
      writer.send(Http::Code::Ok, "Hello from " + serviceName);
    }    
};