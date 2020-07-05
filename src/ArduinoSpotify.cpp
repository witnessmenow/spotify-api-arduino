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

#include "ArduinoSpotify.h"

ArduinoSpotify::ArduinoSpotify(Client &client, char *bearerToken)
{
    this->client = &client;
    sprintf(this->_bearerToken, "Bearer %s", bearerToken);
   
}

ArduinoSpotify::ArduinoSpotify(Client &client, char *clientId, char *clientSecret, char *refreshToken)
{
    this->client = &client;
    this->_clientId = clientId;
    this->_clientSecret = clientSecret;
    this->_refreshToken = refreshToken;
}

int ArduinoSpotify::makeRequestWithBody(char *type, char *command, char* authorization, char *body, char *contentType, char *host)
{
    client->flush();
    client->setTimeout(SPOTIFY_TIMEOUT);
    if (!client->connect(host, portNumber))
    {
        Serial.println(F("Connection failed"));
        return false;
    }

    // give the esp a breather
    yield();

    // Send HTTP request
    client->print(type);
    client->print(command);
    client->println(F(" HTTP/1.1"));

    //Headers
    client->print(F("Host: "));
    client->println(host);

    client->println(F("Accept: application/json"));
    client->print(F("Content-Type: "));
    client->println(contentType);

    if(authorization != NULL){
        client->print(F("Authorization: "));
        client->println(authorization);
    }

    client->println(F("Cache-Control: no-cache"));

    client->print(F("Content-Length: "));
    client->println(strlen(body));

    client->println();

    client->print(body);

    if (client->println() == 0)
    {
        Serial.println(F("Failed to send request"));
        return false;
    }

    int statusCode = getHttpStatusCode();
    skipHeaders();
    return statusCode;
}

int ArduinoSpotify::makePutRequest(char *command, char* authorization, char *body, char *contentType)
{
    return makeRequestWithBody("PUT ", command, authorization, body, contentType);
}

int ArduinoSpotify::makePostRequest(char *command, char* authorization, char *body, char *contentType, char *host)
{
    return makeRequestWithBody("POST ", command, authorization, body, contentType, host);
}

int ArduinoSpotify::makeGetRequest(char *command)
{
    client->flush();
    client->setTimeout(SPOTIFY_TIMEOUT);
    if (!client->connect(SPOTIFY_HOST, portNumber))
    {
        Serial.println(F("Connection failed"));
        return false;
    }

    // give the esp a breather
    yield();

    // Send HTTP request
    client->print(F("GET "));
    client->print(command);
    client->println(F(" HTTP/1.1"));

    //Headers
    client->print(F("Host: "));
    client->println(SPOTIFY_HOST);

    client->println(F("Accept: application/json"));
    client->println(F("Content-Type: application/json"));

    client->print(F("Authorization: "));
    client->println(_bearerToken);

    client->println(F("Cache-Control: no-cache"));

    if (client->println() == 0)
    {
        Serial.println(F("Failed to send request"));
        return false;
    }

    int statusCode = getHttpStatusCode();
    
    // Let the caller of this method parse the JSon from the client
    skipHeaders();
    return statusCode;
}

void ArduinoSpotify::setRefreshToken(char *refreshToken){
    _refreshToken = refreshToken;
}

bool ArduinoSpotify::refreshAccessToken(){
    char body[1000];
    sprintf(body, refreshAccessTokensBody, _refreshToken, _clientId, _clientSecret);

    if (_debug)
    {
        Serial.println(body);
    }

    int statusCode = makePostRequest(SPOTIFY_TOKEN_ENDPOINT, NULL, body, "application/x-www-form-urlencoded", SPOTIFY_ACCOUNTS_HOST);
    unsigned long now = millis();
    if (_debug)
    {
        Serial.print("status Code");
        Serial.println(statusCode);
    }
    bool refreshed = false;
    if(statusCode == 200){
        DynamicJsonDocument doc(1000);
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            sprintf(this->_bearerToken, "Bearer %s", doc["access_token"].as<char *>());
            int tokenTtl = doc["expires_in"]; // Usually 3600 (1 hour)
            tokenTimeToLiveMs = (tokenTtl * 1000) - 2000; // The 2000 is just to force the token expiry to check if its very close
            timeTokenRefreshed = now;
            refreshed = true;
        }
    } else {
        parseError();
    }
    
    closeClient();
    return refreshed;
}

bool ArduinoSpotify::checkAndRefreshAccessToken(){
    unsigned long timeSinceLastRefresh = millis() - timeTokenRefreshed;
    if(timeSinceLastRefresh >= tokenTimeToLiveMs){
        Serial.println("Refresh of the Access token is due, doing that now.");
        return refreshAccessToken();
    }

    // Token is still valid
    return true;
}

char* ArduinoSpotify::requestAccessTokens(char * code, char * redirectUrl){

    char body[1000];
    sprintf(body, requestAccessTokensBody, code, redirectUrl, _clientId, _clientSecret);

    if (_debug)
    {
        Serial.println(body);
    }

    int statusCode = makePostRequest(SPOTIFY_TOKEN_ENDPOINT, NULL, body, "application/x-www-form-urlencoded", SPOTIFY_ACCOUNTS_HOST);
    unsigned long now = millis();
    if (_debug)
    {
        Serial.print("status Code");
        Serial.println(statusCode);
    }
    if(statusCode == 200){
        DynamicJsonDocument doc(1000);
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            sprintf(this->_bearerToken, "Bearer %s", doc["access_token"].as<char *>());
            _refreshToken = (char *) doc["refresh_token"].as<char *>();
            int tokenTtl = doc["expires_in"]; // Usually 3600 (1 hour)
            tokenTimeToLiveMs = (tokenTtl * 1000) - 2000; // The 2000 is just to force the token expiry to check if its very close
            timeTokenRefreshed = now;
        }
    } else {
        parseError();
    }
    
    closeClient();
    return _refreshToken;
}

bool ArduinoSpotify::play(char *deviceId){
    char command[100] = SPOTIFY_PLAY_ENDPOINT;
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::playAdvanced(char *body, char *deviceId){
    char command[100] = SPOTIFY_PLAY_ENDPOINT;
    return playerControl(command, deviceId, body);
}

bool ArduinoSpotify::pause(char *deviceId){
    char command[100] = SPOTIFY_PAUSE_ENDPOINT;
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::setVolume(int volume, char *deviceId){
    char command[125];
    sprintf(command, SPOTIFY_VOLUME_ENDPOINT, volume);
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::toggleShuffle(bool shuffle, char *deviceId){
    char command[125];
    char shuffleState[10];
    if(shuffle){
        strcpy(shuffleState, "true");
    } else {
        strcpy(shuffleState, "false");
    }
    sprintf(command, SPOTIFY_SHUFFLE_ENDPOINT, shuffleState);
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::setRepeatMode(RepeatOptions repeat, char *deviceId){
    char command[125];
    char repeatState[10];
    switch(repeat)
    {
        case repeat_track  : strcpy(repeatState, "track");   break;
        case repeat_context: strcpy(repeatState, "context"); break;
        case repeat_off : strcpy(repeatState, "off");  break;
    }

    sprintf(command, SPOTIFY_REPEAT_ENDPOINT, repeatState);
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::playerControl(char *command,char *deviceId, char *body){
    if (deviceId != ""){
        char * questionMarkPointer;
        questionMarkPointer = strchr(command,'?');
        if(questionMarkPointer == NULL){
            strcat(command, "?deviceId=%s");
        } else {
            // params already started
            strcat(command, "&deviceId=%s");
        }
        sprintf(command, command, deviceId);
    }

    if (_debug)
    {
        Serial.println(command);
        Serial.println(body);
    }

    if(autoTokenRefresh){
        checkAndRefreshAccessToken();
    }
    int statusCode = makePutRequest(command, _bearerToken, body);
    
    closeClient();
    //Will return 204 if all went well.
    return statusCode == 204;
}

bool ArduinoSpotify::playerNavigate(char *command,char *deviceId){
    if (deviceId != ""){
        strcat(command, "?deviceId=%s");
        sprintf(command, command, deviceId);
    }

    if (_debug)
    {
        Serial.println(command);
    }

    if(autoTokenRefresh){
        checkAndRefreshAccessToken();
    }
    int statusCode = makePostRequest(command, _bearerToken);
    
    closeClient();
    //Will return 204 if all went well.
    return statusCode == 204;
}

bool ArduinoSpotify::nextTrack(char *deviceId){
    char command[100] = SPOTIFY_NEXT_TRACK_ENDPOINT;
    return playerNavigate(command, deviceId);
}

bool ArduinoSpotify::previousTrack(char *deviceId){
    char command[100] = SPOTIFY_PREVIOUS_TRACK_ENDPOINT;
    return playerNavigate(command, deviceId);
}
bool ArduinoSpotify::seek(int position, char *deviceId){
    char command[100] = SPOTIFY_SEEK_ENDPOINT;
    strcat(command, "?position_ms=%d");
    sprintf(command, command, position);
    if (deviceId != ""){
        strcat(command, "?deviceId=%s");
        sprintf(command, command, deviceId);
    }

    if (_debug)
    {
        Serial.println(command);
    }

    if(autoTokenRefresh){
        checkAndRefreshAccessToken();
    }
    int statusCode = makePutRequest(command, _bearerToken);
    closeClient();
    //Will return 204 if all went well.
    return statusCode == 204;
}

CurrentlyPlaying ArduinoSpotify::getCurrentlyPlaying(char *market)
{
    char command[100] = SPOTIFY_CURRENTLY_PLAYING_ENDPOINT;
    if (market != ""){
        strcat(command, "?market=%s");
        sprintf(command, command, market);
    }
    if (_debug)
    {
        Serial.println(command);
    }

    // Get from https://arduinojson.org/v6/assistant/
    const size_t bufferSize = currentlyPlayingBufferSize;
    CurrentlyPlaying currentlyPlaying;
    // This flag will get cleared if all goes well
    currentlyPlaying.error = true;
    if(autoTokenRefresh){
        checkAndRefreshAccessToken();
    }
    if (makeGetRequest(command) == 200)
    {
        // Allocate DynamicJsonDocument
        DynamicJsonDocument doc(bufferSize);

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            JsonObject item = doc["item"];
            JsonObject firstArtist = item["album"]["artists"][0];

            currentlyPlaying.firstArtistName = (char *) firstArtist["name"].as<char *>();
            currentlyPlaying.firstArtistUri = (char *) firstArtist["uri"].as<char *>(); 

            currentlyPlaying.albumName = (char *) item["album"]["name"].as<char *>(); 
            currentlyPlaying.albumUri = (char *) item["album"]["uri"].as<char *>(); 

            currentlyPlaying.trackName = (char *) item["name"].as<char *>(); 
            currentlyPlaying.trackUri = (char *) item["uri"].as<char *>(); 

            currentlyPlaying.isPlaying = doc["is_playing"].as<bool>();

            currentlyPlaying.error = false;
        }
        else
        {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
        }
    }
    closeClient();
    return currentlyPlaying;
}

void ArduinoSpotify::skipHeaders()
{
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client->find(endOfHeaders))
    {
        Serial.println(F("Invalid response"));
        return;
    }

    // Was getting stray characters between the headers and the body
    // This should toss them away
    while (client->available() && client->peek() != '{')
    {
        char c = 0;
        client->readBytes(&c, 1);
        if (_debug)
        {
            Serial.print(F("Tossing an unexpected character: "));
            Serial.println(c);
        }
    }
}

int ArduinoSpotify::getHttpStatusCode()
{
    // Check HTTP status
    if(client->find("HTTP/1.1")){
        int statusCode = client->parseInt();
        if (_debug)
        {
            Serial.print(F("Status Code: "));
            Serial.println(statusCode);
        }
        return statusCode;
    } 

    return -1;
}

void ArduinoSpotify::parseError()
{
    DynamicJsonDocument doc(1000);
    DeserializationError error = deserializeJson(doc, *client);
    if (!error)
    {
        Serial.print(F("getAuthToken error"));
        serializeJson(doc, Serial);
    } else {
        Serial.print(F("Could not parse error"));
    }
}

void ArduinoSpotify::closeClient()
{
    if (client->connected())
    {
        if (_debug)
        {
            Serial.println(F("Closing client"));
        }
        client->stop();
    }
}