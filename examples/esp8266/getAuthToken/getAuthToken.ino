/*******************************************************************
    Get Auth Token from spotify, this is needed for the other
    examples. It will store the authToken in littleFS (Spiffs replacement)

    - Put in your Wifi details and Client ID and flash to the board
    - Open browser to esp: "esp8266.local"
    - Click the link
    - Authorization Code will be printed to screen, use this
      for AUTH_CODE in other examples.

    - Make sure to whitelist http://arduino.local/callback/


    Parts:
    D1 Mini ESP8266 * - http://s.click.aliexpress.com/e/uzFUnIe

 *  * = Affilate

    If you find what I do usefuland would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/


// ----------------------------
// Standard Libraries
// ----------------------------

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <FS.h>
#include <LittleFS.h>

//------- Replace the following! ------

char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network password
char clientId[] = "56t4373258u3405u43u543"; // Your client ID of your spotify APP

char scope[] = "user-read-private%20user-read-email";
char callbackURI[] = "http%3A%2F%2Farduino.local%2Fcallback%2F";

char fileName[] = "/spotifyAuth";

//------- ---------------------- ------

ESP8266WebServer server(80);

const char *webpageTemplate =
  R"(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
  </head>
  <body>
    <div>
     <a href="https://accounts.spotify.com/authorize?client_id=%s&response_type=code&redirect_uri=%s&scope=%s">spotify Auth</a>
    </div>
  </body>
</html>
)";

void handleRoot() {
  char webpage[800];
  sprintf(webpage, webpageTemplate, clientId, callbackURI, scope);
  server.send(200, "text/html", webpage);
}

bool writeCodeToFS(String code) {
  File file = LittleFS.open(fileName, "w");
  if (!file) {
    Serial.printf("Unable to open file for writing\n");
    return false;
  } else {
    file.write(code.c_str());
  }
  file.close();
  return true;
}

void handleCallback() {
  String code = "";
  bool wroteCodeToFS = false;
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "code") {
      code = server.arg(i);
      wroteCodeToFS = writeCodeToFS(code);
    }
  }

  if(wroteCodeToFS){
    code = "Code written to FS: " + code;
    server.send(200, "text/plain", code);
  } else {
    code = "Failed to write code to FS: " + code;
    server.send(404, "text/plain", code); 
  }
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  Serial.print(message);
  server.send(404, "text/plain", message);
}

void setup() {

  Serial.begin(115200);

  if (!LittleFS.begin()) {
    Serial.printf("Unable to begin FS, aborting\n");
    return;
  }

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  if (MDNS.begin("arduino")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/callback/", handleCallback);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}



void loop() {
  server.handleClient();
  MDNS.update();
}
