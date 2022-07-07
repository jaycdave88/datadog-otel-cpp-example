# Opentelemetry C++ Datadog example
A sample source code showing how to instrument C++ Microservices for distributed tracing with Jaeger and Datadog. The local Jaeger spans (in the thrift UDP compact) are collected by the OpenTelemetry collector, and exported to Datadog.

# Instructions
Provide your Datadog API key under the config.yml. 

Then, type:
```
docker compose up --build
```

Once this is done, you can send GET request to one of the microservices:
```
curl http://localhost:8081/ping
```
or
```
curl http://localhost:8082/ping
```
Navigate to https://app.datadoghq.com/apm/traces to visualize the trace telemetry. 
