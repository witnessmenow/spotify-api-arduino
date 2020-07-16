/*
Copyright (c) 2020 Brian Lough. All right reserved.

ArduinoSpotify - An Arduino library to wrap the Spotify API

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

#ifndef ArduinoSpotify_h
#define ArduinoSpotify_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Client.h>

#define SPOTIFY_HOST "api.spotify.com"
#define SPOTIFY_ACCOUNTS_HOST "accounts.spotify.com"
// Fingerprint correct as of May 4th 2020
#define SPOTIFY_FINGERPRINT "AB BC 7C 9B 7A D8 5D 98 8B B2 72 A4 4C 13 47 9A 00 2F 70 B5"
#define SPOTIFY_TIMEOUT 2000


#define SPOTIFY_CURRENTLY_PLAYING_ENDPOINT "/v1/me/player/currently-playing"

#define SPOTIFY_PLAY_ENDPOINT "/v1/me/player/play"
#define SPOTIFY_PAUSE_ENDPOINT "/v1/me/player/pause"
#define SPOTIFY_VOLUME_ENDPOINT "/v1/me/player/volume?volume_percent=%d"
#define SPOTIFY_SHUFFLE_ENDPOINT "/v1/me/player/shuffle?state=%s"
#define SPOTIFY_REPEAT_ENDPOINT "/v1/me/player/repeat?state=%s"

#define SPOTIFY_NEXT_TRACK_ENDPOINT "/v1/me/player/next"
#define SPOTIFY_PREVIOUS_TRACK_ENDPOINT "/v1/me/player/previous"

#define SPOTIFY_SEEK_ENDPOINT "/v1/me/player/seek"
//const char currentlyPlayingEndpoint[] = "/v1/me/player/currently-playing";
//const char playerNextTrackEndpoint[] = "/v1/me/player/next";

#define SPOTIFY_TOKEN_ENDPOINT "/api/token"

enum RepeatOptions { repeat_track, repeat_context, repeat_off };

struct SpotifyImage
{
  int height;
  int width;
  char *url;
};

struct CurrentlyPlaying
{
    char *firstArtistName;
    char *firstArtistUri;
    char *albumName;
    char *albumUri;
    char *trackName;
    char *trackUri;
    SpotifyImage smallestImage;
    bool isPlaying;

    bool error;
};

class ArduinoSpotify
{
  public:
    ArduinoSpotify(Client &client, char *bearerToken);
    ArduinoSpotify(Client &client, char *clientId, char *clientSecret, char *refreshToken = "");
    
    // Auth Methods
    void setRefreshToken(char *refreshToken);
    bool refreshAccessToken();
    bool checkAndRefreshAccessToken();
    char* requestAccessTokens(char * code, char * redirectUrl);
    
    // Generic Request Methods
    int makeGetRequest(char *command);
    int makeRequestWithBody(char *type, char *command, char* authorization, char *body = "", char *contentType = "application/json", char * host = SPOTIFY_HOST);
    int makePostRequest(char *command, char* authorization, char *body = "", char *contentType = "application/json", char * host = SPOTIFY_HOST);
    int makePutRequest(char *command, char* authorization, char *body = "", char *contentType = "application/json");

    // User methods
    CurrentlyPlaying getCurrentlyPlaying(char *market = "");
    bool play(char *deviceId = "");
    bool playAdvanced(char *body, char *deviceId = "");
    bool pause(char *deviceId = "");
    bool setVolume(int volume, char *deviceId = "");
    bool toggleShuffle(bool shuffle, char *deviceId = "");
    bool setRepeatMode(RepeatOptions repeat, char *deviceId = "");
    bool nextTrack(char *deviceId = "");
    bool previousTrack(char *deviceId = "");
    bool playerControl(char *command, char *deviceId, char *body = "");
    bool playerNavigate(char *command, char *deviceId);
    bool seek(int position, char *deviceId = "");
    
    
    int portNumber = 443;
    int tagArraySize = 10;
    int currentlyPlayingBufferSize = 10000;
    bool autoTokenRefresh = true;
    bool _debug = false;
    Client *client;

  private:
    char _bearerToken[200];
    char *_refreshToken;
    char *_clientId;
    char *_clientSecret;
    unsigned int timeTokenRefreshed;
    unsigned int tokenTimeToLiveMs;
    int getHttpStatusCode();
    void skipHeaders();
    void closeClient();
    void parseError();
    const char *requestAccessTokensBody = 
    R"(grant_type=authorization_code&code=%s&redirect_uri=%s&client_id=%s&client_secret=%s)"
    ;
    const char *refreshAccessTokensBody = 
    R"(grant_type=refresh_token&refresh_token=%s&client_id=%s&client_secret=%s)"
    ;
};

#endif