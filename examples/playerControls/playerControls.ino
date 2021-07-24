/*******************************************************************
    Controls spotify player using an ESP32 or ESP8266

    Supports:
        - Next Track
        - Previous Track
        - Seek

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
    // uncomment the "#define SPOTIFY_DEBUG" in ArduinoSpotify.h

    Serial.println("Refreshing Access Tokens");
    if (!spotify.refreshAccessToken())
    {
        Serial.println("Failed to get access tokens");
    }

    delay(1000);
    Serial.print("Going to start of track...");
    if (spotify.seek(0))
    {
        Serial.println("done!");
    }
    delay(2000);
    Serial.print("Going to previous track...");
    if (spotify.previousTrack())
    {
        Serial.println("done!");
    }
    delay(2000);
    Serial.print("Skipping to next track...");
    if (spotify.nextTrack())
    {
        Serial.println("done!");
    }

    // Setting volume doesn't seem to work on my Android Phone
    // It does work on my Desktop client
    delay(2000);
    Serial.print("set Volume 10%...");
    if (spotify.setVolume(10))
    {
        Serial.println("done!");
    }
    delay(2000);
    Serial.print("set Volume 70%...");
    if (spotify.setVolume(70))
    {
        Serial.println("done!");
    }
    delay(2000);
    Serial.print("Pausing...");
    if (spotify.pause())
    {
        Serial.println("done!");
    }
    delay(2000);
    Serial.print("Playing...");
    if (spotify.play())
    {
        Serial.println("done!");
    }

    delay(3000);
    Serial.print("enabling shuffle...");
    if (spotify.toggleShuffle(true))
    {
        Serial.println("done!");
    }
    delay(3000);
    Serial.print("disabling shuffle...");
    if (spotify.toggleShuffle(false))
    {
        Serial.println("done!");
    }

    delay(3000);
    Serial.print("Setting repeat mode to 'track'...");
    if (spotify.setRepeatMode(repeat_track))
    {
        Serial.println("done!");
    }
    delay(3000);
    Serial.print("Setting repeat mode to 'context'...");
    if (spotify.setRepeatMode(repeat_context))
    {
        Serial.println("done!");
    }
    delay(3000);
    Serial.print("Setting repeat mode to 'off'...");
    if (spotify.setRepeatMode(repeat_off))
    {
        Serial.println("done!");
    }
}

// Example code is at end of setup
void loop()
{
}
