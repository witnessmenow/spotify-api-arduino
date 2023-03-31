/*******************************************************************
    Prints song information from a Spotify search query to the
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

    Search functionality by Jeremiah Ukwela
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

#define MAX_RESULTS 2

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

  //spotify.searchDetailsBufferSize = 3000; //can be adjusted, needs about 500 per result, defaults to 3000

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }
  
  // If you get an out of memory error, you can increase the search buffer size, default is 3000
  // spotify.searchDetailsBufferSize = 3000;
}

bool getResultsCallback(SearchResult result, int index, int numResults)
{

  Serial.println("--------- Song Details ---------");

  Serial.print("Song Index: ");
  Serial.println(index);

  Serial.print("Song ID: ");
  Serial.println(result.trackUri);

  Serial.print("Song Name: ");
  Serial.println(result.trackName);

  Serial.print("Song Artists: ");
  for (int i = 0; i < result.numArtists; i++){
    Serial.println(result.artists[i].artistName);
  }

  Serial.print("Song Album: ");
  Serial.println(result.albumName);

  Serial.print("Song Images (url): ");
  for (int i = 0; i < result.numImages; i++){
    Serial.println(result.albumImages[i].url);
  }

  Serial.println("-------------------------------");

  return true;
}

void loop()
{
  if (millis() > requestDueTime)
  {
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());

    Serial.println("Making Search Request:");

    String query = "/?q=artist:Toto&type=track&market=US&offset=1";
    
    SearchResult results[MAX_RESULTS];
    int status = spotify.searchForSong(query, MAX_RESULTS, getResultsCallback, results);
    //spotify.searchForSong(query, limit, callback, array);
    
    if (status == 200)
    {
      Serial.println("Successfully got results: printing information");
      for (int i = 0; i < MAX_RESULTS; i++)
      {
        //Plays 30 seconds of each song from the search
        char body[100];
        sprintf(body, "{\"uris\" : [\"%s\"]}", results[i].trackUri);
        if (spotify.playAdvanced(body))
        {
            Serial.println("sent!");
        }
        delay(30*1e3);
      }
    }

    requestDueTime = millis() + delayBetweenRequests;
  }
}
