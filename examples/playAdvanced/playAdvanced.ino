/*******************************************************************
    Allow you to specify a track/artist/album to play.

    This is based on the request described here on the
    spotify documentation:
    https://developer.spotify.com/documentation/web-api/reference/player/start-a-users-playback/

    This example shows some ways of invoking the above, but read the documentation
    for full details.

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

void playSingleTrack()
{
    char sampleTrack[] = "spotify:track:4uLU6hMCjMI75M1A2tKUQC";
    char body[100];
    sprintf(body, "{\"uris\" : [\"%s\"]}", sampleTrack);
    if (spotify.playAdvanced(body))
    {
        Serial.println("sent!");
    }
}

void playMultipleTracks()
{
    char sampleTrack1[] = "spotify:track:6vW1WpedCmV4gtOijSoQV3";
    char sampleTrack2[] = "spotify:track:4dJYjR2lM6SmYfLw2mnHvb";
    char sampleTrack3[] = "spotify:track:4uLU6hMCjMI75M1A2tKUQC";

    char body[200];
    sprintf(body, "{\"uris\" : [\"%s\", \"%s\", \"%s\"]}", sampleTrack1, sampleTrack2, sampleTrack3);
    if (spotify.playAdvanced(body))
    {
        Serial.println("sent!");
    }
}

void playAlbum()
{
    char sampleAlbum[] = "spotify:album:6N9PS4QXF1D0OWPk0Sxtb4";

    char body[100];
    sprintf(body, "{\"context_uri\" : \"%s\"}", sampleAlbum);
    if (spotify.playAdvanced(body))
    {
        Serial.println("sent!");
    }
}

void specifyTrackNumOfAlbum()
{
    char sampleAlbum[] = "spotify:album:2fPcSpVFVo1dXEvarNoFkB";
    // The position has an index of 0, so passing in 2
    // like this will actually play the 3rd song.
    int trackNum = 2;

    char body[200];
    sprintf(body, "{\"context_uri\" : \"%s\", \"offset\": {\"position\": %d}}", sampleAlbum, trackNum);
    if (spotify.playAdvanced(body))
    {
        Serial.println("sent!");
    }
}

void specifyTrackOfAlbum()
{
    char sampleAlbum[] = "spotify:album:2BLjT6yzDdKojUyc3Gi6y2";
    char trackOnAlbum[] = "spotify:track:25IZtuJS77yXPCXMhPa1ze";

    char body[200];
    sprintf(body, "{\"context_uri\" : \"%s\", \"offset\": {\"uri\": \"%s\"}}", sampleAlbum, trackOnAlbum);
    if (spotify.playAdvanced(body))
    {
        Serial.println("sent!");
    }
}

void playArtist()
{
    char sampleArtist[] = "spotify:artist:0gxyHStUsqpMadRV0Di1Qt";

    char body[100];
    sprintf(body, "{\"context_uri\" : \"%s\"}", sampleArtist);
    if (spotify.playAdvanced(body))
    {
        Serial.println("sent!");
    }
}

void playPlaylist()
{
    char samplePlaylist[] = "spotify:playlist:37i9dQZF1DZ06evO05tE88";

    char body[100];
    sprintf(body, "{\"context_uri\" : \"%s\"}", samplePlaylist);
    if (spotify.playAdvanced(body))
    {
        Serial.println("sent!");
    }
}

void specifyTrackNumOfPlaylist()
{
    char samplePlaylist[] = "spotify:playlist:37i9dQZF1DZ06evO05tE88";
    // The position has an index of 0, so passing in 31
    // like this will actually play the 32nd song.
    int playlistTrackNum = 31;

    char body[200];
    sprintf(body, "{\"context_uri\" : \"%s\", \"offset\": {\"position\": %d}}", samplePlaylist, playlistTrackNum);
    if (spotify.playAdvanced(body))
    {
        Serial.println("sent!");
    }
}

void specifyTrackOfPlaylist()
{
    char samplePlaylist[] = "spotify:playlist:37i9dQZF1DZ06evO05tE88";
    char trackOnPlaylist[] = "spotify:track:6vW1WpedCmV4gtOijSoQV3";

    char body[200];
    sprintf(body, "{\"context_uri\" : \"%s\", \"offset\": {\"uri\": \"%s\"}}", samplePlaylist, trackOnPlaylist);
    if (spotify.playAdvanced(body))
    {
        Serial.println("sent!");
    }
}

void setup()
{

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
        return;
    }

    delay(100);
    Serial.println("Playing Single Track");
    playSingleTrack();
    delay(10000);
    Serial.println("Playing Multiple Tracks");
    playMultipleTracks();
    delay(10000);
    Serial.println("Playing Album");
    playAlbum();
    delay(10000);
    Serial.println("Playing track number on Album");
    specifyTrackNumOfAlbum();
    delay(10000);
    Serial.println("Playing specific track on Album");
    specifyTrackOfAlbum();
    delay(10000);
    Serial.println("Playing Artist");
    playArtist();
    delay(10000);
    Serial.println("Playing Playlist");
    playPlaylist();
    delay(10000);
    Serial.println("Playing track number on Playlist");
    specifyTrackNumOfPlaylist();
    delay(10000);
    Serial.println("Playing specific track on Playlist");
    specifyTrackOfPlaylist();
}

// Example code is at end of setup
void loop()
{
}
