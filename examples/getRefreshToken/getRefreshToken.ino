/*******************************************************************
    Get Refresh Token from spotify, this is needed for the other
    examples.

    Instructions:

    - Put in your Wifi details, Client ID, Client secret and flash to the board
    - Do one of the following

    --- IF USING IP (ESP32 MDNS does not work for me)
    
    - Get the Ip Address from the serial monitor
    - Add the following to Redirect URI on your Spotify app "http://[ESP_IP]/callback/"
    e.g. "http://192.168.1.20/callback/" (don't forget the last "/")
    - Open browser to esp using the IP 
    
    --- IF USING MDNS

    - Search for "#define USE_IP_ADDRESS" and comment it out 
    - Add the following to Redirect URI on your Spotify app "http://arduino.local/callback/" 
    (don't forget the last "/")
    - Open browser to esp: http://arduino.local 
    
    -----------
    
    - Click the link on the webpage
    - The Refresh Token will be printed to screen, use this
      for SPOTIFY_REFRESH_TOKEN in other examples.

    Compatible Boards:
	  - Any ESP8266 or ESP32 board

    Parts:
    ESP32 D1 Mini style Dev board* - http://s.click.aliexpress.com/e/C6ds4my

 *  * = Affiliate

    If you find what I do useful and would like to support me,
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

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#endif

#include <WiFiClient.h>
#include <WiFiClientSecure.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <SpotifyArduino.h>
// Library for connecting to the Spotify API

// Install from Github
// https://github.com/witnessmenow/spotify-api-arduino

// including a "spotify_server_cert" variable
// header is included as part of the SpotifyArduino libary
#include <SpotifyArduinoCert.h>

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses

// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

//------- Replace the following! ------

char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network password

char clientId[] = "56t4373258u3405u43u543";     // Your client ID of your spotify APP
char clientSecret[] = "56t4373258u3405u43u543"; // Your client Secret of your spotify APP (Do Not share this!)

char scope[] = "user-read-playback-state%20user-modify-playback-state";

#define USE_IP_ADDRESS 1 //comment this out if you want to use MDNS

#ifdef USE_IP_ADDRESS
char callbackURItemplate[] = "%s%s%s";
char callbackURIProtocol[] = "http%3A%2F%2F"; // "http://"
char callbackURIAddress[] = "%2Fcallback%2F"; // "/callback/"
char callbackURI[100];
#else
char callbackURI[] = "http%3A%2F%2Farduino.local%2Fcallback%2F";
#endif

//------- ---------------------- ------

#if defined(ESP8266)
ESP8266WebServer server(80);
#elif defined(ESP32)
WebServer server(80);
#endif

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret);

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

void handleRoot()
{
  char webpage[800];
  sprintf(webpage, webpageTemplate, clientId, callbackURI, scope);
  server.send(200, "text/html", webpage);
}

void handleCallback()
{
  String code = "";
  const char *refreshToken = NULL;
  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "code")
    {
      code = server.arg(i);
      refreshToken = spotify.requestAccessTokens(code.c_str(), callbackURI);
    }
  }

  if (refreshToken != NULL)
  {
    server.send(200, "text/plain", refreshToken);
  }
  else
  {
    server.send(404, "text/plain", "Failed to load token, check serial monitor");
  }
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  Serial.print(message);
  server.send(404, "text/plain", message);
}

void setup()
{

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  IPAddress ipAddress = WiFi.localIP();
  Serial.println(ipAddress);

  if (MDNS.begin("arduino"))
  {
    Serial.println("MDNS responder started");
  }

  // Handle HTTPS Verification
#if defined(ESP8266)
  client.setFingerprint(SPOTIFY_FINGERPRINT); // These expire every few months
#elif defined(ESP32)
  client.setCACert(spotify_server_cert);
#endif
  // ... or don't!
  //client.setInsecure();

  // If you want to enable some extra debugging
  // uncomment the "#define SPOTIFY_DEBUG" in SpotifyArduino.h

#ifdef USE_IP_ADDRESS
  // Building up callback URL using IP address.
  sprintf(callbackURI, callbackURItemplate, callbackURIProtocol, ipAddress.toString().c_str(), callbackURIAddress);
#endif

  server.on("/", handleRoot);
  server.on("/callback/", handleCallback);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
#if defined(ESP8266)
  MDNS.update();
#endif

  server.handleClient();
}
