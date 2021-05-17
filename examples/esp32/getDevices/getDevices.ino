/*******************************************************************
    Prints info about the devices currently connected to your spotify account
    and transfer playback between them.
    (useful for switching between your phone to your PC for example)

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

    Parts:
    ESP32 D1 Mini stlye Dev board* - http://s.click.aliexpress.com/e/C6ds4my

 *  * = Affilate

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

#include <WiFi.h>
#include <WiFiClientSecure.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <ArduinoSpotify.h>
// Library for connecting to the Spotify API

// Install from Github
// https://github.com/witnessmenow/arduino-spotify-api

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses

// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

//------- Replace the following! ------

char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network password

char clientId[] = "56t4373258u3405u43u543"; // Your client ID of your spotify APP
char clientSecret[] = "56t4373258u3405u43u543"; // Your client Secret of your spotify APP (Do Not share this!)

// Country code, including this is advisable
#define SPOTIFY_MARKET "IE"

#define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"


//------- ---------------------- ------

#define SPOTIFY_MAX_DEVICES 5 // Number of devices you want to try parse

// including a "spotify_server_cert" variable
// header is included as part of the ArduinoSpotify libary
#include <ArduinoSpotifyCert.h>

WiFiClientSecure client;
ArduinoSpotify spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

unsigned long delayBetweenRequests = 60000; // Time between requests (1 minute)
unsigned long requestDueTime;               //time when request due

SpotifyDevice* deviceList;

void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setCACert(spotify_server_cert);

  // If you want to enable some extra debugging
  // uncomment the "#define SPOTIFY_DEBUG" in ArduinoSpotify.h

  //spotify.getDevicesBufferSize = 2000; //can be adjusted, needs about 300 per device, defaults to 2000
  // Remember this is all devices returned by spotify, not just the max you want to return

  //inits an empty array of devices, required for the getDevices method
  deviceList = spotify.generateDevicesArray(SPOTIFY_MAX_DEVICES);

  // This allocates a decent chunk of memory
  // If you want to free it up you can use
  // spotify.destroyDevicesArray(deviceList,SPOTIFY_MAX_DEVICES);
  // but you will need generate a new one if you want to get devices again

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken()) {
    Serial.println("Failed to get access tokens");
  }
}

void printDeviceToSerial(SpotifyDevice device)
{

  Serial.println("--------- Device Details ---------");

  Serial.print("Device ID: ");
  Serial.println(device.id);

  Serial.print("Device Name: ");
  Serial.println(device.name);

  Serial.print("Device Type: ");
  Serial.println(device.type);

  Serial.print("Is Active: ");
  if (device.isActive)
  {
    Serial.println("Yes");
  } else {
    Serial.println("No");
  }

  Serial.print("Is Resticted: ");
  if (device.isRestricted)
  {
    Serial.println("Yes, from API docs \"no Web API commands will be accepted by this device\"");
  } else {
    Serial.println("No");
  }

  Serial.print("Is Private Session: ");
  if (device.isPrivateSession)
  {
    Serial.println("Yes");
  } else {
    Serial.println("No");
  }

  Serial.print("Volume Precent: ");
  Serial.println(device.volumePrecent);

  Serial.println("------------------------");
}

void loop() {
  if (millis() > requestDueTime)
  {
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());

    Serial.println("Getting devices:");
    // Market can be excluded if you want e.g. spotify.getPlayerDetails()
    int numDevices = spotify.getDevices(deviceList, SPOTIFY_MAX_DEVICES);
    for (int i = 0; i < numDevices; i++) {
      printDeviceToSerial(deviceList[i]);
      spotify.transferPlayback(deviceList[i].id, true); //true means to play after transfer
      delay(5000);
    }

    requestDueTime = millis() + delayBetweenRequests;
  }

}
