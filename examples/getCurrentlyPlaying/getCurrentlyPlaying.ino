/*******************************************************************
    Prints your currently playing track on spotify to the
    serial monitor using an ES32 or ESP8266

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

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
#elif defined(ESP32)
#include <WiFi.h>
#endif

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

// Country code, including this is advisable
#define SPOTIFY_MARKET "IE"

#define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"

//------- ---------------------- ------

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

unsigned long delayBetweenRequests = 60000; // Time between requests (1 minute)
unsigned long requestDueTime;               //time when request due

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
    Serial.println(WiFi.localIP());

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

    Serial.println("Refreshing Access Tokens");
    if (!spotify.refreshAccessToken())
    {
        Serial.println("Failed to get access tokens");
    }
}

void printCurrentlyPlayingToSerial(CurrentlyPlaying currentlyPlaying)
{
    // Use the details in this method or if you want to store them
    // make sure you copy them (using something like strncpy)
    // const char* artist =

    Serial.println("--------- Currently Playing ---------");

    Serial.print("Is Playing: ");
    if (currentlyPlaying.isPlaying)
    {
        Serial.println("Yes");
    }
    else
    {
        Serial.println("No");
    }

    Serial.print("Track: ");
    Serial.println(currentlyPlaying.trackName);
    Serial.print("Track URI: ");
    Serial.println(currentlyPlaying.trackUri);
    Serial.println();

    Serial.println("Artists: ");
    for (int i = 0; i < currentlyPlaying.numArtists; i++)
    {
        Serial.print("Name: ");
        Serial.println(currentlyPlaying.artists[i].artistName);
        Serial.print("Artist URI: ");
        Serial.println(currentlyPlaying.artists[i].artistUri);
        Serial.println();
    }

    Serial.print("Album: ");
    Serial.println(currentlyPlaying.albumName);
    Serial.print("Album URI: ");
    Serial.println(currentlyPlaying.albumUri);
    Serial.println();

    if (currentlyPlaying.contextUri != NULL)
    {
        Serial.print("Context URI: ");
        Serial.println(currentlyPlaying.contextUri);
        Serial.println();
    }

    long progress = currentlyPlaying.progressMs; // duration passed in the song
    long duration = currentlyPlaying.durationMs; // Length of Song
    Serial.print("Elapsed time of song (ms): ");
    Serial.print(progress);
    Serial.print(" of ");
    Serial.println(duration);
    Serial.println();

    float percentage = ((float)progress / (float)duration) * 100;
    int clampedPercentage = (int)percentage;
    Serial.print("<");
    for (int j = 0; j < 50; j++)
    {
        if (clampedPercentage >= (j * 2))
        {
            Serial.print("=");
        }
        else
        {
            Serial.print("-");
        }
    }
    Serial.println(">");
    Serial.println();

    // will be in order of widest to narrowest
    // currentlyPlaying.numImages is the number of images that
    // are stored
    for (int i = 0; i < currentlyPlaying.numImages; i++)
    {
        Serial.println("------------------------");
        Serial.print("Album Image: ");
        Serial.println(currentlyPlaying.albumImages[i].url);
        Serial.print("Dimensions: ");
        Serial.print(currentlyPlaying.albumImages[i].width);
        Serial.print(" x ");
        Serial.print(currentlyPlaying.albumImages[i].height);
        Serial.println();
    }
    Serial.println("------------------------");
}

void loop()
{
    if (millis() > requestDueTime)
    {
        Serial.print("Free Heap: ");
        Serial.println(ESP.getFreeHeap());

        Serial.println("getting currently playing song:");
        // Market can be excluded if you want e.g. spotify.getCurrentlyPlaying()
        int status = spotify.getCurrentlyPlaying(printCurrentlyPlayingToSerial, SPOTIFY_MARKET);
        if (status == 200)
        {
            Serial.println("Successfully got currently playing");
        }
        else if (status == 204)
        {
            Serial.println("Doesn't seem to be anything playing");
        }
        else
        {
            Serial.print("Error: ");
            Serial.println(status);
        }
        requestDueTime = millis() + delayBetweenRequests;
    }
}
