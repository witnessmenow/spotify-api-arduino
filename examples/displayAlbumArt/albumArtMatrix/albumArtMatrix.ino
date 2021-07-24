/*******************************************************************
    Displays Album Art on an 64 x 64 RGB LED Matrix. ESP32 Only

    There is two approaches to this demoed in this example
      - "displayImage" uses a memory buffer, it should be the fastest but possible uses the most memory.
      - "displayImageUsingFile" uses a File reference

    All references to SPIFFS are only required for the "displayImageUsingFile" path.

    This example could easily be adapted to any Adafruit GFX
    based screen.

    The library for the display will need to be modified to work
    with a 64x64 matrix:
    https://github.com/witnessmenow/ESP32-i2s-Matrix-Shield#using-a-64x64-display

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

    Parts:
    ESP32 D1 Mini stlye Dev board* - http://s.click.aliexpress.com/e/C6ds4my
    ESP32 I2S Matrix Shield (From my Tindie) = https://www.tindie.com/products/brianlough/esp32-i2s-matrix-shield/
    64 x 64 RGB LED Matrix* - https://s.click.aliexpress.com/e/_BfjY0wfp

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

#define FS_NO_GLOBALS
#include <FS.h>
#include "SPIFFS.h"

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <ESP32-RGB64x32MatrixPanel-I2S-DMA.h>
// This is the library for interfacing with the display

// Can be installed from the library manager (Search for "ESP32 64x32 LED MATRIX")
// https://github.com/mrfaptastic/ESP32-RGB64x32MatrixPanel-I2S-DMA

#include <ArduinoSpotify.h>
// Library for connecting to the Spotify API

// Install from Github
// https://github.com/witnessmenow/arduino-spotify-api

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses

// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

#include <TJpg_Decoder.h>
// Library for decoding Jpegs from the API responses

// Search for "tjpg" in the Arduino Library manager
// https://github.com/Bodmer/TJpg_Decoder

//------- Replace the following! ------

char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network password

char clientId[] = "56t4373258u3405u43u543";     // Your client ID of your spotify APP
char clientSecret[] = "56t4373258u3405u43u543"; // Your client Secret of your spotify APP (Do Not share this!)

// Country code, including this is advisable
#define SPOTIFY_MARKET "IE"

#define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"

//------- ---------------------- ------

// including a "spotify_server_cert" variable
// header is included as part of the ArduinoSpotify libary
#include <ArduinoSpotifyCert.h>

// file name for where to save the image.
#define ALBUM_ART "/album.jpg"

// so we can compare and not download the same image if we already have it.
String lastAlbumArtUrl;

WiFiClientSecure client;
ArduinoSpotify spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

// You might want to make this much smaller, so it will update responsively

unsigned long delayBetweenRequests = 30000; // Time between requests (30 seconds)
unsigned long requestDueTime;               //time when request due

RGB64x32MatrixPanel_I2S_DMA dma_display;

// This next function will be called during decoding of the jpeg file to
// render each block to the Matrix.  If you use a different display
// you will need to adapt this function to suit.
bool displayOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if (y >= dma_display.height())
    return 0;

  dma_display.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

void setup()
{

  Serial.begin(115200);

  // Initialise SPIFFS, if this fails try .begin(true)
  // NOTE: I believe this formats it though it will erase everything on
  // spiffs already! In this example that is not a problem.
  // I have found once I used the true flag once, I could use it
  // without the true flag after that.

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS initialisation failed!");
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

  dma_display.begin();
  dma_display.fillScreen(dma_display.color565(255, 0, 0));

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(displayOutput);

  // The byte order can be swapped (set true for TFT_eSPI)
  //TJpgDec.setSwapBytes(true);

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

  client.setCACert(spotify_server_cert);

  // If you want to enable some extra debugging
  // uncomment the "#define SPOTIFY_DEBUG" in ArduinoSpotify.h

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }
}
int displayImageUsingFile(char *albumArtUrl)
{

  // In this example I reuse the same filename
  // over and over, maybe saving the art using
  // the album URI as the name would be better
  // as you could save having to download them each
  // time, but this seems to work fine.
  if (SPIFFS.exists(ALBUM_ART) == true)
  {
    Serial.println("Removing existing image");
    SPIFFS.remove(ALBUM_ART);
  }

  fs::File f = SPIFFS.open(ALBUM_ART, "w+");
  if (!f)
  {
    Serial.println("file open failed");
    return -1;
  }

  bool gotImage = spotify.getImage(albumArtUrl, &f);

  // Make sure to close the file!
  f.close();

  if (gotImage)
  {
    return TJpgDec.drawFsJpg(0, 0, ALBUM_ART);
  }
  else
  {
    return -2;
  }
}

int displayImage(char *albumArtUrl)
{

  uint8_t *imageFile; // pointer that the library will store the image at (uses malloc)
  int imageSize;      // library will update the size of the image
  bool gotImage = spotify.getImage(albumArtUrl, &imageFile, &imageSize);

  if (gotImage)
  {
    Serial.print("Got Image");
    delay(1);
    int jpegStatus = TJpgDec.drawJpg(0, 0, imageFile, imageSize);
    free(imageFile); // Make sure to free the memory!
    return jpegStatus;
  }
  else
  {
    return -2;
  }
}

void printCurrentlyPlayingToSerial(CurrentlyPlaying currentlyPlaying)
{
  if (!currentlyPlaying.error)
  {
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

    Serial.print("Artist: ");
    Serial.println(currentlyPlaying.firstArtistName);
    Serial.print("Artist URI: ");
    Serial.println(currentlyPlaying.firstArtistUri);
    Serial.println();

    Serial.print("Album: ");
    Serial.println(currentlyPlaying.albumName);
    Serial.print("Album URI: ");
    Serial.println(currentlyPlaying.albumUri);
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
}

void loop()
{
  if (millis() > requestDueTime)
  {
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());

    Serial.println("getting currently playing song:");
    // Market can be excluded if you want e.g. spotify.getCurrentlyPlaying()
    CurrentlyPlaying currentlyPlaying = spotify.getCurrentlyPlaying(SPOTIFY_MARKET);
    if (!currentlyPlaying.error)
    {
      printCurrentlyPlayingToSerial(currentlyPlaying);

      // Smallest (narrowest) image will always be last.
      SpotifyImage smallestImage = currentlyPlaying.albumImages[currentlyPlaying.numImages - 1];
      String newAlbum = String(smallestImage.url);
      if (newAlbum != lastAlbumArtUrl)
      {
        Serial.println("Updating Art");
        //int displayImageResult = displayImageUsingFile(smallestImage.url); File reference example
        int displayImageResult = displayImage(smallestImage.url); // Memory Buffer Example - should be much faster
        if (displayImageResult == 0)
        {
          lastAlbumArtUrl = newAlbum;
        }
        else
        {
          Serial.print("failed to display image: ");
          Serial.println(displayImageResult);
        }
      }
    }

    requestDueTime = millis() + delayBetweenRequests;
  }
}
