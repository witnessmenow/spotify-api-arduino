/*******************************************************************
    Prints info about your currently active spotify device
    on the serial monitor using an ES32

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

// including a "spotify_server_cert" variable
// header is included as part of the ArduinoSpotify libary
#include <ArduinoSpotifyCert.h>

WiFiClientSecure client;
ArduinoSpotify spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

unsigned long delayBetweenRequests = 60000; // Time between requests (1 minute)
unsigned long requestDueTime;               //time when request due


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

    Serial.println("Refreshing Access Tokens");
    if(!spotify.refreshAccessToken()){
        Serial.println("Failed to get access tokens");
    }
}

void printPlayerDetailsToSerial(PlayerDetails playerDetails)
{
    if (!playerDetails.error)
    {
        Serial.println("--------- Player Details ---------");

        Serial.print("Device ID: ");
        Serial.println(playerDetails.device.id);

        Serial.print("Device Name: ");
        Serial.println(playerDetails.device.name);

        Serial.print("Device Type: ");
        Serial.println(playerDetails.device.type);

        Serial.print("Is Active: ");
        if (playerDetails.device.isActive)
        {
            Serial.println("Yes");
        } else {
            Serial.println("No");
        }

        Serial.print("Is Resticted: ");
        if (playerDetails.device.isRestricted)
        {
            Serial.println("Yes, from API docs \"no Web API commands will be accepted by this device\"");
        } else {
            Serial.println("No");
        }

        Serial.print("Is Private Session: ");
        if (playerDetails.device.isPrivateSession)
        {
            Serial.println("Yes");
        } else {
            Serial.println("No");
        }

        Serial.print("Volume Precent: ");
        Serial.println(playerDetails.device.volumePrecent);

        Serial.print("Progress (Ms): ");
        Serial.println(playerDetails.progressMs);

        Serial.print("Is Playing: ");
        if (playerDetails.isPlaying)
        {
            Serial.println("Yes");
        } else {
            Serial.println("No");
        }

        Serial.print("Shuffle State: ");
        if (playerDetails.shuffleState)
        {
            Serial.println("On");
        } else {
            Serial.println("Off");
        }

        Serial.print("Repeat State: ");
        switch(playerDetails.repeateState)
        {
            case repeat_track  : Serial.println("track");   break;
            case repeat_context: Serial.println("context"); break;
            case repeat_off : Serial.println("off");  break;
        }

        Serial.println("------------------------");
    }
}

void loop() {
  if (millis() > requestDueTime)
    {
        Serial.print("Free Heap: ");
        Serial.println(ESP.getFreeHeap());

        Serial.println("Getting player Details:");
        // Market can be excluded if you want e.g. spotify.getPlayerDetails()
        PlayerDetails playerDetails = spotify.getPlayerDetails(SPOTIFY_MARKET);

        printPlayerDetailsToSerial(playerDetails);

        requestDueTime = millis() + delayBetweenRequests;
    }

}
