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

ArduinoSpotify::ArduinoSpotify(Client &client, char *clientId, char *clientSecret, SaveRefreshToken saveToken, ReadRefreshToken readToken)
{
    this->client = &client;
    this->_clientId = clientId;
    this->_clientSecret = clientSecret;
    this->_readToken = readToken;
    this->_saveToken = saveToken;
}

ArduinoSpotify::ArduinoSpotify(Client &client, char *encodedClientIDSecret, SaveRefreshToken saveToken, ReadRefreshToken readToken)
{
    this->client = &client;
    this->_encodedClientIDSecret = encodedClientIDSecret;
    this->_readToken = readToken;
    this->_saveToken = saveToken;
}

bool ArduinoSpotify::refreshToken(){
    //_authToken = _readToken();

    // Do something

    //_saveToken(_authToken);
}

int ArduinoSpotify::makePutRequest(char *command)
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
    client->print(F("PUT "));
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

    client->print(F("Content-Length: "));
    client->println(0);

    client->println();

    //send Data here?

    if (client->println() == 0)
    {
        Serial.println(F("Failed to send request"));
        return false;
    }

    int statusCode = getHttpStatusCode();
    skipHeaders();
    return statusCode;
}

int ArduinoSpotify::makePostRequest(char *command, char* authorization, char *body, char *contentType, char *host)
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
    client->print(F("POST "));
    client->print(command);
    client->println(F(" HTTP/1.1"));

    //Headers
    client->print(F("Host: "));
    client->println(host);

    client->println(F("Accept: application/json"));
    client->print(F("Content-Type: "));
    client->println(contentType);

    client->print(F("Authorization: "));
    client->println(authorization);

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

char* ArduinoSpotify::getAuthToken(char * code, char * redirectUrl){

    char body[1000];
    sprintf(body, getAuthTokenBody, code, redirectUrl);

    char auth[200];
    sprintf(auth, "Basic %s", _encodedClientIDSecret);

    if (_debug)
    {
        Serial.println(body);
    }

    int statusCode = makePostRequest(SPOTIFY_TOKEN_ENDPOINT, auth, body, "application/x-www-form-urlencoded", SPOTIFY_ACCOUNTS_HOST);
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
            _authToken = (char *) doc["refresh_token"].as<char *>();
            int expires_in = doc["expires_in"]; // 3600
            Serial.print("Expires in: ");
            Serial.println(expires_in);
        }
    } else {
        DynamicJsonDocument doc(1000);
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            Serial.print("getAuthToken error");
            serializeJson(doc, Serial);
        }
    }
    
    // _saveToken(_authToken);
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

    int statusCode = makePostRequest(command, _bearerToken);

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

    int statusCode = makePutRequest(command);

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
            currentlyPlaying.albumName = (char *) item["album"]["name"].as<char *>(); 
            currentlyPlaying.trackName = (char *) item["name"].as<char *>(); 

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
            Serial.print("Tossing an unexpected character: ");
            Serial.println(c);
        }
    }
}

int ArduinoSpotify::getHttpStatusCode()
{
    // Check HTTP status
    if(client->find("HTTP/1.1")){
        int statusCode = client->parseInt();
        return statusCode;
    } 

    return -1;
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