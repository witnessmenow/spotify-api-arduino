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
// Fingerprint correct as of May 4th 2020
#define SPOTIFY_FINGERPRINT "AB BC 7C 9B 7A D8 5D 98 8B B2 72 A4 4C 13 47 9A 00 2F 70 B5"
#define SPOTIFY_TIMEOUT 2000

#define SPOTIFY_CURRENTLY_PLAYING_ENDPOINT "/v1/me/player/currently-playing"

struct CurrentlyPlaying
{
    char *firstArtistName;
    char *albumName;
    char *trackName;
    bool isPlaying;

    bool error;
};

class ArduinoSpotify
{
  public:
    ArduinoSpotify(Client &client, char *bearerToken);
    bool makeGetRequest(char *command);
    CurrentlyPlaying getCurrentlyPlaying(char *market = "");
    int portNumber = 443;
    int tagArraySize = 10;
    int currentlyPlayingBufferSize = 10000;
    bool _debug = false;
    Client *client;

  private:
    char *_bearerToken;
    void closeClient();
};

#endif