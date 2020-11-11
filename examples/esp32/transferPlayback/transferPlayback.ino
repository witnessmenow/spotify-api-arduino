/*******************************************************************
    Shows how to use spotify devices using an ESP32

    Supports:
        - Starts playback on all devices, one after another

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

    Parts:
    ESP32 D1 Mini stlye Dev board* - http://s.click.aliexpress.com/e/C6ds4my

 *  * = Affilate

    If you find what I do usefuland would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    This example is written by Claus Näveke
    Twitter: https://twitter.com/Nitek
 *******************************************************************/


// ----------------------------
// Standard Libraries
// ----------------------------

#include <WiFi.h>
#include <WiFiClientSecure.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses

// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

#include <ArduinoSpotify.h>
// Library for connecting to the Spotify API

// Install from Github
// https://github.com/witnessmenow/arduino-spotify-api

//------- Replace the following! ------

char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network password

char clientId[] = "56t4373258u3405u43u543"; // Your client ID of your spotify APP
char clientSecret[] = "56t4373258u3405u43u543"; // Your client Secret of your spotify APP (Do Not share this!)

#define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"


//------- ---------------------- ------

// including a "spotify_server_cert" variable
// header is included as part of the ArduinoSpotify libary
#include <ArduinoSpotifyCert.h>

WiFiClientSecure client;
ArduinoSpotify spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);


void setup() {

  Serial.begin(115200);

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

    client.setCACert(spotify_server_cert);

    // If you want to enable some extra debugging
    // uncomment the "#define SPOTIFY_DEBUG" in ArduinoSpotify.h

    Serial.println("Refreshing Access Tokens");
    if(!spotify.refreshAccessToken()){
        Serial.println("Failed to get access tokens");
        return;
    }

    SpotifyDevice devices[10];
    uint8_t numDevices = spotify.getDevices(devices, 10);

    Serial.printf("Number of devices: %d\n", numDevices);

    for(uint8_t i = 0; i < numDevices; i++) {
        Serial.printf("Going to start of track on device %s (%d, %d, %d)\n", devices[i].name, devices[i].isActive, devices[i].isPrivateSession, devices[i].isRestricted);
        if(spotify.transferPlayback(devices[i].id, true)){
            Serial.println("done!");
        }
        delay(5000);
    }
}


// Example code is at end of setup
void loop() {
  
}