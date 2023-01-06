/*
SpotifyArduino - An Arduino library to wrap the Spotify API

Copyright (c) 2020  Brian Lough.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef SpotifyArduino_h
#define SpotifyArduino_h

// I find setting these types of flags unreliable from the Arduino IDE
// so uncomment this if its not working for you.
// NOTE: Do not use this option on live-streams, it will reveal your
// private tokens!

#define SPOTIFY_DEBUG 1

// Comment out if you want to disable any serial output from this library (also comment out DEBUG and PRINT_JSON_PARSE)
#define SPOTIFY_SERIAL_OUTPUT 1

// Prints the JSON received to serial (only use for debugging as it will be slow)
//#define SPOTIFY_PRINT_JSON_PARSE 1

// Use PSRAM for ArduinoJSON
#define SPOTIFY_JSON_PSRAM 1

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Client.h>

#ifdef SPOTIFY_PRINT_JSON_PARSE
#include <StreamUtils.h>
#endif

#define SPOTIFY_HOST "api.spotify.com"
#define SPOTIFY_ACCOUNTS_HOST "accounts.spotify.com"

// Fingerprint for "*.spotify.com", correct as of May 7th, 2022
#define SPOTIFY_FINGERPRINT "4A 44 71 F7 6A 8D D4 BD 54 E9 0E 3D E8 6C A6 E0 00 27 BA D5"

// Fingerprint for "*.scdn.co", correct as of May 7th, 2022
#define SPOTIFY_IMAGE_SERVER_FINGERPRINT "08 9F D9 6B 46 93 45 43 29 07 BE AD 76 CF 6B 44 3B AE 95 F6"

#define SPOTIFY_TIMEOUT 2000

#define SPOTIFY_NAME_CHAR_LENGTH 100 //Increase if artists/song/album names are being cut off
#define SPOTIFY_URI_CHAR_LENGTH 40
#define SPOTIFY_URL_CHAR_LENGTH 70

#define SPOTIFY_DEVICE_ID_CHAR_LENGTH 45
#define SPOTIFY_DEVICE_NAME_CHAR_LENGTH 80
#define SPOTIFY_DEVICE_TYPE_CHAR_LENGTH 30

#define SPOTIFY_CURRENTLY_PLAYING_ENDPOINT "/v1/me/player/currently-playing"

#define SPOTIFY_PLAYER_ENDPOINT "/v1/me/player"
#define SPOTIFY_DEVICES_ENDPOINT "/v1/me/player/devices"

#define SPOTIFY_PLAY_ENDPOINT "/v1/me/player/play"
#define SPOTIFY_SEARCH_ENDPOINT "/v1/search"
#define SPOTIFY_PAUSE_ENDPOINT "/v1/me/player/pause"
#define SPOTIFY_VOLUME_ENDPOINT "/v1/me/player/volume?volume_percent=%d"
#define SPOTIFY_SHUFFLE_ENDPOINT "/v1/me/player/shuffle?state=%s"
#define SPOTIFY_REPEAT_ENDPOINT "/v1/me/player/repeat?state=%s"

#define SPOTIFY_NEXT_TRACK_ENDPOINT "/v1/me/player/next"
#define SPOTIFY_PREVIOUS_TRACK_ENDPOINT "/v1/me/player/previous"

#define SPOTIFY_SEEK_ENDPOINT "/v1/me/player/seek"

#define SPOTIFY_TOKEN_ENDPOINT "/api/token"

#define SPOTIFY_NUM_ALBUM_IMAGES 3 // Max spotify returns is 3, but the third one is probably too big for an ESP

#define SPOTIFY_MAX_NUM_ARTISTS 5

#define SPOTIFY_ACCESS_TOKEN_LENGTH 309

#ifdef SPOTIFY_JSON_PSRAM
struct SpiRamAllocator {
  void* allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }

  void deallocate(void* pointer) {
    heap_caps_free(pointer);
  }

  void* reallocate(void* ptr, size_t new_size) {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};
using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;
#endif

enum RepeatOptions
{
  repeat_track,
  repeat_context,
  repeat_off
};

struct SpotifyImage
{
  int height;
  int width;
  const char *url;
};

struct SpotifyDevice
{
  const char *id;
  const char *name;
  const char *type;
  bool isActive;
  bool isRestricted;
  bool isPrivateSession;
  int volumePercent;
};

struct PlayerDetails
{
  SpotifyDevice device;

  long progressMs;
  bool isPlaying;
  RepeatOptions repeateState;
  bool shuffleState;
};

struct SpotifyArtist
{
  const char *artistName;
  const char *artistUri;
};

struct SearchResult
{
  const char *albumName;
  const char *albumUri;
  const char *trackName;
  const char *trackUri;
  SpotifyArtist artists[SPOTIFY_MAX_NUM_ARTISTS];
  SpotifyImage albumImages[SPOTIFY_NUM_ALBUM_IMAGES];
  int numArtists;
  int numImages;
};

struct CurrentlyPlaying
{
  SpotifyArtist artists[SPOTIFY_MAX_NUM_ARTISTS];
  int numArtists;
  const char *albumName;
  const char *albumUri;
  const char *trackName;
  const char *trackUri;
  SpotifyImage albumImages[SPOTIFY_NUM_ALBUM_IMAGES];
  int numImages;
  bool isPlaying;
  long progressMs;
  long durationMs;
};

class SpotifyArduino
{
public:
  SpotifyArduino(Client &client);
  SpotifyArduino(Client &client, char *bearerToken);
  SpotifyArduino(Client &client, const char *clientId, const char *clientSecret, const char *refreshToken = "");

  // Auth Methods
  void setRefreshToken(const char *refreshToken);
  bool refreshAccessToken();
  bool checkAndRefreshAccessToken();
  const char *requestAccessTokens(const char *code, const char *redirectUrl);

  // Generic Request Methods
  int makeGetRequest(const char *command, const char *authorization, const char *accept = "application/json", const char *host = SPOTIFY_HOST);
  int makeRequestWithBody(const char *type, const char *command, const char *authorization, const char *body = "", const char *contentType = "application/json", const char *host = SPOTIFY_HOST);
  int makePostRequest(const char *command, const char *authorization, const char *body = "", const char *contentType = "application/json", const char *host = SPOTIFY_HOST);
  int makePutRequest(const char *command, const char *authorization, const char *body = "", const char *contentType = "application/json", const char *host = SPOTIFY_HOST);

  // User methods
  int getCurrentlyPlaying(std::function<void(CurrentlyPlaying)> currentlyPlayingCallback, const char *market = "");
  int getPlayerDetails(std::function<void(PlayerDetails)> playerDetailsCallback, const char *market = "");
  int getDevices(std::function<bool(SpotifyDevice device, int index, int numDevices)> devicesCallback);
  bool play(const char *deviceId = "");
  bool playAdvanced(char *body, const char *deviceId = "");
  bool pause(const char *deviceId = "");
  bool setVolume(int volume, const char *deviceId = "");
  bool toggleShuffle(bool shuffle, const char *deviceId = "");
  bool setRepeatMode(RepeatOptions repeat, const char *deviceId = "");
  bool nextTrack(const char *deviceId = "");
  bool previousTrack(const char *deviceId = "");
  bool playerControl(char *command, const char *deviceId = "", const char *body = "");
  bool playerNavigate(char *command, const char *deviceId = "");
  bool seek(int position, const char *deviceId = "");
  bool transferPlayback(const char *deviceId, bool play = false);
    
  //Search
  int searchForSong(String query, int limit, std::function<bool(SearchResult result, int index, int numResults)> searchCallback, SearchResult results[]);

  // Image methods
  bool getImage(char *imageUrl, Stream *file);
  bool getImage(char *imageUrl, uint8_t **image, int *imageLength);

  int portNumber = 443;
  int currentlyPlayingBufferSize = 3000;
  int playerDetailsBufferSize = 2000;
  int getDevicesBufferSize = 3000;
  int searchDetailsBufferSize = 3000;
  bool autoTokenRefresh = true;
  Client *client;
  void lateInit(const char *clientId, const char *clientSecret, const char *refreshToken = "");

#ifdef SPOTIFY_DEBUG
  char *stack_start;
#endif

private:
  char _bearerToken[SPOTIFY_ACCESS_TOKEN_LENGTH + 10]; //10 extra is for "bearer " at the start
  char *_refreshToken=nullptr;
  const char *_clientId=nullptr;
  const char *_clientSecret=nullptr;
  unsigned int timeTokenRefreshed;
  unsigned int tokenTimeToLiveMs;
  int commonGetImage(char *imageUrl);
  int getContentLength();
  int getHttpStatusCode();
  void skipHeaders(bool tossUnexpectedForJSON = true);
  void closeClient();
  void parseError();
  const char *requestAccessTokensBody =
      R"(grant_type=authorization_code&code=%s&redirect_uri=%s&client_id=%s&client_secret=%s)";
  const char *refreshAccessTokensBody =
      R"(grant_type=refresh_token&refresh_token=%s&client_id=%s&client_secret=%s)";
#ifdef SPOTIFY_DEBUG
  void printStack();
#endif
};

#endif
