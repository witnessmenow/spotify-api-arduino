/*******************************************************************
    Controls spotify player using an WiFiNINA based Arduino

    Supports:
        - Next Track
        - Previous Track
        - Seek
        - play & pause
        - set volume
        - shuffle
        - repeat

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

    Parts:
    Arduino Nano 33 IOT - https://store.arduino.cc/arduino-nano-33-iot

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
#include <SPI.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <WiFiNINA.h>
// Library for using network deatures of the official Arudino
// Wifi Boards (MKR WiFi 1010, Nano 33 IOT etc)

// Search for "nina" in the Arduino Library Manager
// https://github.com/arduino-libraries/WiFiNINA

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


int status = WL_IDLE_STATUS;

WiFiSSLClient client;
ArduinoSpotify spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

unsigned long delayBetweenRequests = 60000; // Time between requests (1 minute)
unsigned long requestDueTime;               //time when request due

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, password);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();

  // If you want to enable some extra debugging
  // uncomment the "#define SPOTIFY_DEBUG" in ArduinoSpotify.h

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken()) {
    Serial.println("Failed to get access tokens");
    return;
  }

  delay(1000);
  Serial.print("Going to start of track...");
  if (spotify.seek(0)) {
    Serial.println("done!");
  }
  delay(2000);
  Serial.print("Going to previous track...");
  if (spotify.previousTrack()) {
    Serial.println("done!");
  }
  delay(2000);
  Serial.print("Skipping to next track...");
  if (spotify.nextTrack()) {
    Serial.println("done!");
  }

  // Setting volume doesn't seem to work on my Android Phone
  // It does work on my Desktop client
  delay(2000);
  Serial.print("set Volume 10%...");
  if (spotify.setVolume(10)) {
    Serial.println("done!");
  }
  delay(2000);
  Serial.print("set Volume 70%...");
  if (spotify.setVolume(70)) {
    Serial.println("done!");
  }
  delay(2000);
  Serial.print("Pausing...");
  if (spotify.pause()) {
    Serial.println("done!");
  }
  delay(2000);
  Serial.print("Playing...");
  if (spotify.play()) {
    Serial.println("done!");
  }

  delay(3000);
  Serial.print("enabling shuffle...");
  if (spotify.toggleShuffle(true)) {
    Serial.println("done!");
  }
  delay(3000);
  Serial.print("disabling shuffle...");
  if (spotify.toggleShuffle(false)) {
    Serial.println("done!");
  }

  delay(3000);
  Serial.print("Setting repeat mode to 'track'...");
  if (spotify.setRepeatMode(repeat_track)) {
    Serial.println("done!");
  }
  delay(3000);
  Serial.print("Setting repeat mode to 'context'...");
  if (spotify.setRepeatMode(repeat_context)) {
    Serial.println("done!");
  }
  delay(3000);
  Serial.print("Setting repeat mode to 'off'...");
  if (spotify.setRepeatMode(repeat_off)) {
    Serial.println("done!");
  }

}


// Example code is at end of setup
void loop() {

}


void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}