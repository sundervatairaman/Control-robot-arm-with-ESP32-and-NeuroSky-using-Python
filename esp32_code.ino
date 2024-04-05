#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <AsyncWebSocket.h>
#include <iostream>
#include <sstream>
#include <SPIFFS.h> // Include the SPIFFS library
#include <WebSocketsServer.h>
#include "index.h"


const char* ssid = "username";
const char* password = "password";




AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);  // WebSocket server on port 81


const int servoPins[3] = {14, 26, 27}; // Pins to which the servos are connected (shoulder, elbow, base)
const float shoulderArmLength = 10.0;   // Define the shoulder arm length
const float elbowArmLength = 8.0;       // Define the elbow arm length

int xyz_positions[3]; // Array to store servo angles
Servo servos[3];      // Servo objects




void updateServoAngles() {
  xyz_positions[0] = xyz_positions[0] - 11; // Subtracting constant from X
  xyz_positions[1] = xyz_positions[1];      // Assuming Y is not modified
  xyz_positions[2] = xyz_positions[2] - 2;  // Subtracting constant from Z

  

  // Additional features: squares of X, Y, Z, and sums of squares
  float x_squared = pow(xyz_positions[0], 2);
  float y_squared = pow(xyz_positions[1], 2);
  float z_squared = pow(xyz_positions[2], 2);
  float xy_sum_squared = x_squared + y_squared;
  float xz_sum_squared = x_squared + z_squared;
  float yz_sum_squared = y_squared + z_squared;

  float calcAngle = 180 - (atan2(xyz_positions[1], xyz_positions[0]) * RAD_TO_DEG);
  float distanceFromOrigin = sqrt(xy_sum_squared);

  float theta = atan2(xyz_positions[2], distanceFromOrigin) * RAD_TO_DEG;
  float distanceFromOrigin_z = sqrt(pow(distanceFromOrigin, 2) + pow(xyz_positions[2], 2));

  float alpha = ((pow(elbowArmLength, 2) + pow(shoulderArmLength, 2) - pow(distanceFromOrigin_z, 2)) / (2 * elbowArmLength * shoulderArmLength));
  float beta = ((pow(shoulderArmLength, 2) + pow(distanceFromOrigin_z, 2) - pow(elbowArmLength, 2)) / (2 * shoulderArmLength * distanceFromOrigin_z));

  // Ensure alpha and beta are within valid range
  alpha = min(max(alpha, -1.0f), 1.0f); // Make sure both arguments are of type float
  beta = min(max(beta, -1.0f), 1.0f);   // Make sure both arguments are of type float

  float calcAngle1 = theta + (acos(beta) * RAD_TO_DEG);
  float calcAngle2 = acos(alpha) * RAD_TO_DEG;

  // Map angles to servo values
  float a1 = (calcAngle / 180) * 3495;
  float b1 = ((180 - calcAngle1) / 180) * 3495 + 700;
  float c1 = (calcAngle2 / 180) * 3495 + 400;

  // Use a1, b1, c1 for servo control
  a1 = (a1/3495)*180;
  b1 = (b1/3495)*180;
  c1 = (c1/3495)*180;
  
  //Serial.println(b1,c1,a1);
  // Write angles to servos
  servos[0].write(b1); // Write angle a1 to the first servo
  servos[1].write(c1); // Write angle b1 to the second servo
  servos[2].write(a1); // Write angle c1 to the third servo
  delay(1000);
  

}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        Serial.printf("[%u] Received text: %s\n", num, payload);
        //String xyz1 = String((char*)payload);
        //Serial.println(xyz1);
      }
      break;
    case WStype_TEXT:
      {
        Serial.printf("[%u] Received text: %s\n", num, payload);

        // Ensure payload is null-terminated
        payload[length] = '\0';

        // Deserialize JSON
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, (char*)payload);
        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          return;
        }

        // Extract x, y, z values if they exist in the JSON
        if (doc.containsKey("x") && doc.containsKey("y") && doc.containsKey("Z") ) {
          xyz_positions[0] = doc["x"];
          xyz_positions[1] = doc["y"];
          xyz_positions[2] = doc["z"];
          updateServoAngles();
          Serial.println(xyz_positions[0]);
        }
      }
      break;
  }
}




void setup() {
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");
  // Print the IP address
  

  // Attach servos to pins
  for (int i = 0; i < 3; i++) {
    servos[i].attach(servoPins[i]);
  }
  // Initialize WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Serve a basic HTML page with JavaScript to create the WebSocket connection
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("Web Server: received a web page request");
    String html = HTML_CONTENT;  // Use the HTML content from the index.h file
    request->send(200, "text/html", html);
  });

  server.begin();
  Serial.print("ESP32 Web Server's IP address: ");
  Serial.println(WiFi.localIP());
}



void loop() {
  // Handle WebSocket events
  webSocket.loop();  // Clean up disconnected clients
  // Your existing code here (potentiometer reading, angle calculations, etc.)
}
