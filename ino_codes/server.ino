#include <WiFi.h>
// Use this specific header file instead of WebServer.h
#include <ESPWebServerSecure.hpp> 

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Define an API security key for an extra layer of protection (optional)
const char* apiKey = "MySecretAPIKey123"; 

// Create a secure server instance on default HTTPS port 443
ESPWebServerSecure server(443);

void handleGetData() {
  // Optional Header API Key security check
  if (server.hasHeader("X-API-KEY") && server.header("X-API-KEY") != apiKey) {
    server.send(401, "application/json", "{\"error\":\"Unauthorized\"}");
    return;
  }

  // Serve your endpoint payload
  float temperature = 26.4;
  String jsonResponse = "{\"status\":\"secure_success\", \"temperature\":" + String(temperature) + "}";
  server.send(200, "application/json", jsonResponse);
}

void handleNotFound() {
  server.send(404, "application/json", "{\"error\":\"Endpoint not found\"}");
}

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected!");
  Serial.print("Secure API URL: https://");
  Serial.println(WiFi.localIP());

  // Tell the server to watch for your custom authentication header
  const char * headerKeys[] = {"X-API-KEY"};
  size_t headerKeysSize = sizeof(headerKeys)/sizeof(char*);
  server.collectHeaders(headerKeys, headerKeysSize);

  // Setup API routing endpoints
  server.on("/data", HTTP_GET, handleGetData);
  server.onNotFound(handleNotFound);

  // Fire up the HTTPS engine
  server.begin();
  Serial.println("HTTPS API Engine is active.");
}

void loop() {
  // Hand off processor to listen for incoming secure clients
  server.handleClient();
}
