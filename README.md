# Otel.logz.io
A sample source code showing how to instrument c++ Microservices for distributed tracing with Jaeger and Logz.io. The local Jaeger spans (in the thrift UDP compact) are collected by the OpenTelemetry collector, and exported to Logz.io Jaeger Backend.

# Instructions
Provide your Logz.io account token, and region under the config.yml. 

Then, type:
```
docker compose up --build
```

The build will begin: 
![Figure](/images/01.png)

Once this is done, you can send GET request to one of the microservices:
```
curl http://localhost:8081/ping
```
or
```
curl http://localhost:8082/ping
```

As shown in the figure below:

![Figure](/images/03.png)

You will then see logs: 

![Figure](/images/04.png)

and spans in the Logz.io Jaeger UI:

![Figure](/images/07.png)

