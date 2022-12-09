/*******************************************************************
    Displays Album Art on an 128 x 160 display. ESP32 Only

    There is two approaches to this demoed in this example
      - "displayImage" uses a memory buffer, it should be the fastest but possible uses the most memory.
      - "displayImageUsingFile" uses a File reference

    All references to SPIFFS are only required for the "displayImageUsingFile" path.

    This example could easily be adapted to any TFT
    based screen.

    The library for the display will need to be modified to work
    with a 128x160 display:
    https://github.com/Bodmer/TFT_eSPI

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

    Parts:
    ESP32 D1 Mini stlye Dev board* - http://s.click.aliexpress.com/e/C6ds4my
    ESP32 I2S Matrix Shield (From my Tindie) = https://www.tindie.com/products/brianlough/esp32-i2s-matrix-shield/
    128x160 LCD Screen - shorturl.at/elKQV

 *******************************************************************/

// ----------------------------
// Standard Libraries
// ----------------------------
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <FS.h>
#include "SPIFFS.h"

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

#include <SpotifyArduino.h>
#include <ArduinoJson.h>

char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network password

char clientId[] = "56t4373258u3405u43u543";     // Your client ID of your spotify APP
char clientSecret[] = "56t4373258u3405u43u543"; // Your client Secret of your spotify APP (Do Not share this!)

// Country code, including this is advisable
#define SPOTIFY_MARKET "IE"

#define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"

// including a "spotify_server_cert" variable
// header is included as part of the SpotifyArduino libary
#include <SpotifyArduinoCert.h>

// file name for where to save the image.
#define ALBUM_ART "/album.jpg"

// so we can compare and not download the same image if we already have it.
String lastAlbumArtUrl;

// Variable to hold image info
SpotifyImage smallestImage;

// so we can store the song name and artist name
char *songName;
char *songArtist;

WiFiClientSecure client;
SpotifyArduino spotify(client, client_id, client_secret, SPOTIFY_REFRESH_TOKEN);

// You might want to make this much smaller, so it will update responsively

unsigned long delayBetweenRequests = 30000; // Time between requests (30 seconds)
unsigned long requestDueTime;               // time when request due

TFT_eSPI tft = TFT_eSPI();

// This next function will be called during decoding of the jpeg file to
// render each block to the Matrix.  If you use a different display
// you will need to adapt this function to suit.
bool displayOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    // Stop further decoding as image is running off bottom of screen
    if (y >= tft.height())
        return 0;

    tft.pushImage(x, y, w, h, bitmap);

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

    // Start the tft display and set it to black
    tft.init();
    tft.fillScreen(TFT_BLACK);

    // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(4);

    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(displayOutput);

    // The byte order can be swapped (set true for TFT_eSPI)
    TJpgDec.setSwapBytes(true);

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
    // uncomment the "#define SPOTIFY_DEBUG" in SpotifyArduino.h

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
        int jpegStatus = TJpgDec.drawJpg(28, 40, imageFile, imageSize);
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
    // Use the details in this method or if you want to store them
    // make sure you copy them (using something like strncpy)
    // const char* artist =

    // Clear the Text every time a new song is created
    tft.fillRect(0, 120, 128, 130, TFT_BLACK);
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
    // Save the song name to a variable
    songName = const_cast<char *>(currentlyPlaying.trackName);
    drawMessage(0, 120, songName);
    Serial.print("Track URI: ");
    Serial.println(currentlyPlaying.trackUri);
    Serial.println();

    Serial.println("Artists: ");
    for (int i = 0; i < currentlyPlaying.numArtists; i++)
    {
        Serial.print("Name: ");
        // Save the song artist name to a variable
        Serial.println(currentlyPlaying.artists[i].artistName);
        songArtist = const_cast<char *>(currentlyPlaying.artists[0].artistName);
        drawMessage(0, 130, songArtist);
        Serial.print("Artist URI: ");
        Serial.println(currentlyPlaying.artists[i].artistUri);
        Serial.println();
    }

    Serial.print("Album: ");
    Serial.println(currentlyPlaying.albumName);
    Serial.print("Album URI: ");
    Serial.println(currentlyPlaying.albumUri);
    Serial.println();

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
        // Save the second album image into the smallestImage Variable above.
        smallestImage = currentlyPlaying.albumImages[1];
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
        // Check if music is playing currently on the account.
        int status = spotify.getCurrentlyPlaying(printCurrentlyPlayingToSerial, SPOTIFY_MARKET);
        if (status == 200)
        {
            Serial.println("Successfully got currently playing");
            String newAlbum = String(smallestImage.url);
            if (newAlbum != lastAlbumArtUrl)
            {
                Serial.println("Updating Art");
                char *my_url = const_cast<char *>(smallestImage.url);
                int displayImageResult = displayImage(my_url);

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
}

// Method to draw messages at a certain point on a TFT Display.
void drawMessage(int x, int y, char *message)
{
    tft.setTextColor(TFT_WHITE);
    tft.drawString(message, x, y);
}