#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <DNSServer.h>
#define relay_pin 4
const char* ssid = "PowerBar";
const char* password = "22222222";
unsigned long t_on = 0;
unsigned long t_off = 0;
int cycle = 0;
String data = "";

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer server(80);

String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><style> body { background-color: #1a1a1a; color: #fff; font-size: 20px; } h1 { text-align: center; } form { display: flex; flex-direction: column; align-items: center; margin-top: 50px; } input[type=number], input[type=submit] { background-color: #000000; color: #fff; border: none; padding: 12px 24px; margin: 8px; border-radius: 4px; width: 100%; max-width: 400px; box-sizing: border-box; } input[type=submit]:hover { background-color: #090835; border: 2px solid #ffffff; } </style></head><body><h1>Set Timer for Your PowerBar</h1><form method='post'><label for='t_on'>On time (minutes):</label><input type='number' name='t_on' id='t_on'><label for='t_off'>Off time (minutes):</label><input type='number' name='t_off' id='t_off' value='0'><label for='cycle'>Cycle count:</label><input type='number' name='cycle' id='cycle' value='1'><input type='submit' value='Submit'></form></body></html>";

void runrelaytime() {
  if (server.method() == HTTP_GET) {
        server.send(200, "text/html", html);
  } else if (server.method() == HTTP_POST) {
    // Set the relay time based on the submitted form data
    t_on = server.arg("t_on").toInt();
    t_off = server.arg("t_off").toInt();
    cycle = server.arg("cycle").toInt();
    data = "";
    for (int i = 0; i < cycle; i++) {
      digitalWrite(relay_pin, HIGH);
      delay(t_on * 60000);

      digitalWrite(relay_pin, LOW);
      delay(t_off * 60000);
    }
    server.send(200, "text/html", "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'></head><body><h1>Done ...!</h1>");
    delay (5000);
    WiFi.softAPdisconnect(true);
    delay (60000);
    WiFi.softAP(ssid, password);
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) {
    path += "index.html";
  }
  String contentType;
  if (path.endsWith(".html")) {
    contentType = "text/html";
  } else if (path.endsWith(".css")) {
    contentType = "text/css";
  } else if (path.endsWith(".js")) {
    contentType = "application/javascript";
  } else if (path.endsWith(".png")) {
    contentType = "image/png";
  } else if (path.endsWith(".jpg") || path.endsWith(".jpeg")) {
    contentType = "image/jpeg";
  } else if (path.endsWith(".gif")) {
    contentType = "image/gif";
  } else {
    return false;
  }
  File file = SPIFFS.open(path, "r");
  if (!file) {
    return false;
  }
  server.streamFile(file, contentType);
  file.close();
  return true;
}

void setup() {
  pinMode(relay_pin, OUTPUT);
  Serial.begin(115200);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  dnsServer.start(DNS_PORT, "*", apIP);
  
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  
  server.on("/", runrelaytime);
  
  server.onNotFound([]() {

        server.sendHeader("Location", "http://192.168.1.1");
        server.send(301);
        
    });
  
  server.begin();
  
}

void loop() {
    dnsServer.processNextRequest();
    server.handleClient();
} 