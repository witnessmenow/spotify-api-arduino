/*******************************************************************
    Prints info about the devices currently connected to your spotify account
    and transfer playback between them.
    (useful for switching between your phone to your PC for example)

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

// This is potentially optional depending on how you want to do it,
// but we are going to store the important information the library returns here
struct SimpleDevice
{
  char name[SPOTIFY_DEVICE_NAME_CHAR_LENGTH];
  char id[SPOTIFY_DEVICE_ID_CHAR_LENGTH];
};

#define MAX_DEVICES 6

SimpleDevice deviceList[MAX_DEVICES];
int numberOfDevices = -1;

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

  //spotify.getDevicesBufferSize = 3000; //can be adjusted, needs about 300 per device, defaults to 3000
  // Remember this is all devices returned by spotify, not just the max you want to return

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }
}

void printDeviceToSerial(SpotifyDevice device)
{

  Serial.println("--------- Device Details ---------");

  Serial.print("Device ID: ");
  Serial.println(device.id);

  Serial.print("Device Name: ");
  Serial.println(device.name);

  Serial.print("Device Type: ");
  Serial.println(device.type);

  Serial.print("Is Active: ");
  if (device.isActive)
  {
    Serial.println("Yes");
  }
  else
  {
    Serial.println("No");
  }

  Serial.print("Is Resticted: ");
  if (device.isRestricted)
  {
    Serial.println("Yes, from API docs \"no Web API commands will be accepted by this device\"");
  }
  else
  {
    Serial.println("No");
  }

  Serial.print("Is Private Session: ");
  if (device.isPrivateSession)
  {
    Serial.println("Yes");
  }
  else
  {
    Serial.println("No");
  }

  Serial.print("Volume Percent: ");
  Serial.println(device.volumePercent);

  Serial.println("------------------------");
}

bool getDeviceCallback(SpotifyDevice device, int index, int numDevices)
{
  if (index == 0)
  {
    // This is a first device from this batch
    // lets set the number of devices we got back
    if (numDevices < MAX_DEVICES)
    {
      numberOfDevices = numDevices;
    }
    else
    {
      numberOfDevices = MAX_DEVICES;
    }
  }

  // We can't handle anymore than we can fit in our array
  if (index < MAX_DEVICES)
  {
    printDeviceToSerial(device);

    strncpy(deviceList[index].name, device.name, sizeof(deviceList[index].name)); //DO NOT use deviceList[index].name = device.name, it won't work as you expect!
    deviceList[index].name[sizeof(deviceList[index].name) - 1] = '\0';            //ensures its null terminated

    strncpy(deviceList[index].id, device.id, sizeof(deviceList[index].id));
    deviceList[index].id[sizeof(deviceList[index].id) - 1] = '\0';

    if (index == MAX_DEVICES - 1)
    {
      return false; //returning false stops it processing any more
    }
    else
    {
      return true;
    }
  }

  // We should never get here
  return false; //returning false stops it processing any more
}

void loop()
{
  if (millis() > requestDueTime)
  {
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());

    Serial.println("Getting devices:");
    int status = spotify.getDevices(getDeviceCallback);
    if (status == 200)
    {
      Serial.println("Successfully got devices, tranfering playback between them");
      for (int i = 0; i < numberOfDevices; i++)
      {
        // You could do this transfer play back in the callback
        // But this example is more to simulate grabbing the devices
        // and having a UI to change between them
        spotify.transferPlayback(deviceList[i].id, true); //true means to play after transfer
        delay(5000);
      }
    }

    requestDueTime = millis() + delayBetweenRequests;
  }
}
