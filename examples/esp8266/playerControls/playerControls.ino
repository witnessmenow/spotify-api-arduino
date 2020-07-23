/*******************************************************************
    Controls spotify player using an ESP8266

    Supports:
        - Next Track
        - Previous Track
        - Seek

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

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
#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
#include <time.h>

#define FS_NO_GLOBALS
#include <FS.h>
#include "LittleFS.h" // Required for BearSSL CertStore

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

#define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"

//------- ---------------------- ------

// A single, global CertStore which can be used by all
// connections.  Needs to stay live the entire time any of
// the WiFiClientBearSSLs are present.
BearSSL::CertStore certStore;

// Set time via NTP, as required for x.509 validation
void setClock()
{
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

WiFiClientSecure client;
ArduinoSpotify spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);


void setup() {

  Serial.begin(115200);

  // Initialise LittleFS, if this fails try .begin(true)
  // NOTE: I believe this formats it though it will erase everything on
  // spiffs already! In this example that is not a problem.
  // I have found once I used the true flag once, I could use it
  // without the true flag after that.

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS initialisation failed!");
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

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

    // Only avaible in ESP8266 V2.5 RC1 and above
    // client.setFingerprint(SPOTIFY_FINGERPRINT);

    setClock(); // Required for X.509 validation

    int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
    Serial.printf("Number of CA certs read: %d\n", numCerts);
    if (numCerts == 0)
    {
      Serial.printf("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
      return; // Can't connect to anything w/o certs!
    }

    client.setCertStore(&certStore);

    // If you want to enable some extra debugging
    // uncomment the "#define SPOTIFY_DEBUG" in ArduinoSpotify.h

    Serial.println("Refreshing Access Tokens");
    if(!spotify.refreshAccessToken()){
        Serial.println("Failed to get access tokens");
        return;
    }

    delay(1000);
    Serial.print("Going to start of track...");
    if(spotify.seek(0)){
        Serial.println("done!");
    }
    delay(2000);
    Serial.print("Going to previous track...");
    if(spotify.previousTrack()){
        Serial.println("done!");
    }
    delay(2000);
    Serial.print("Skipping to next track...");
    if(spotify.nextTrack()){
        Serial.println("done!");
    }

    // Setting volume doesn't seem to work on my Android Phone
    // It does work on my Desktop client
    delay(2000);
    Serial.print("set Volume 10%...");
    if(spotify.setVolume(10)){
        Serial.println("done!");
    }
    delay(2000);
    Serial.print("set Volume 70%...");
    if(spotify.setVolume(70)){
        Serial.println("done!");
    }
    delay(2000);
    Serial.print("Pausing...");
    if(spotify.pause()){
        Serial.println("done!");
    }
    delay(2000);
    Serial.print("Playing...");
    if(spotify.play()){
        Serial.println("done!");
    }

    delay(3000);
    Serial.print("enabling shuffle...");
    if(spotify.toggleShuffle(true)){
        Serial.println("done!");
    }
    delay(3000);
    Serial.print("disabling shuffle...");
    if(spotify.toggleShuffle(false)){
        Serial.println("done!");
    }

    delay(3000);
    Serial.print("Setting repeat mode to 'track'...");
    if(spotify.setRepeatMode(repeat_track)){
        Serial.println("done!");
    }
    delay(3000);
    Serial.print("Setting repeat mode to 'context'...");
    if(spotify.setRepeatMode(repeat_context)){
        Serial.println("done!");
    }
    delay(3000);
    Serial.print("Setting repeat mode to 'off'...");
    if(spotify.setRepeatMode(repeat_off)){
        Serial.println("done!");
    }
}


// Example code is at end of setup
void loop() {
  
}
