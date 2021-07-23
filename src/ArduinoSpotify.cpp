/*
ArduinoSpotify - An Arduino library to wrap the Spotify API

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

#include "ArduinoSpotify.h"

ArduinoSpotify::ArduinoSpotify()
{
}

ArduinoSpotify::ArduinoSpotify(Client &client, char *bearerToken)
{
    this->client = &client;
    sprintf(this->_bearerToken, "Bearer %s", bearerToken);
    initStructs();
}

ArduinoSpotify::ArduinoSpotify(Client &client, const char *clientId, const char *clientSecret, const char *refreshToken)
{
    this->client = &client;
    this->_clientId = clientId;
    this->_clientSecret = clientSecret;
    this->_refreshToken = refreshToken;
    initStructs();
}

int ArduinoSpotify::makeRequestWithBody(const char *type, const char *command, const char *authorization, const char *body, const char *contentType, const char *host)
{
    client->flush();
    #ifdef SPOTIFY_DEBUG
        Serial.println(host);
    #endif
    client->setTimeout(SPOTIFY_TIMEOUT);
    if (!client->connect(host, portNumber))
    {
        Serial.println(F("Connection failed"));
        return -1;
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

    if (authorization != NULL)
    {
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
        return -2;
    }

    int statusCode = getHttpStatusCode();
    return statusCode;
}

int ArduinoSpotify::makePutRequest(const char *command, const char *authorization, const char *body, const char *contentType, const char *host)
{
    return makeRequestWithBody("PUT ", command, authorization, body, contentType);
}

int ArduinoSpotify::makePostRequest(const char *command, const char *authorization, const char *body, const char *contentType, const char *host)
{
    return makeRequestWithBody("POST ", command, authorization, body, contentType, host);
}

int ArduinoSpotify::makeGetRequest(const char *command, const char *authorization, const char *accept, const char *host)
{
    client->flush();
    client->setTimeout(SPOTIFY_TIMEOUT);
    if (!client->connect(host, portNumber))
    {
        Serial.println(F("Connection failed"));
        return -1;
    }

    // give the esp a breather
    yield();

    // Send HTTP request
    client->print(F("GET "));
    client->print(command);
    client->println(F(" HTTP/1.1"));

    //Headers
    client->print(F("Host: "));
    client->println(host);

    if (accept != NULL)
    {
        client->print(F("Accept: "));
        client->println(accept);
    }

    if (authorization != NULL)
    {
        client->print(F("Authorization: "));
        client->println(authorization);
    }

    client->println(F("Cache-Control: no-cache"));

    if (client->println() == 0)
    {
        Serial.println(F("Failed to send request"));
        return -2;
    }

    int statusCode = getHttpStatusCode();

    return statusCode;
}

void ArduinoSpotify::setRefreshToken(const char *refreshToken)
{
    _refreshToken = refreshToken;
}

bool ArduinoSpotify::refreshAccessToken()
{
    char body[300];
    sprintf(body, refreshAccessTokensBody, _refreshToken, _clientId, _clientSecret);

#ifdef SPOTIFY_DEBUG
    Serial.println(body);
    printStack();
#endif

    int statusCode = makePostRequest(SPOTIFY_TOKEN_ENDPOINT, NULL, body, "application/x-www-form-urlencoded", SPOTIFY_ACCOUNTS_HOST);
    if (statusCode > 0)
    {
        skipHeaders();
    }
    unsigned long now = millis();

#ifdef SPOTIFY_DEBUG
    Serial.print("status Code");
    Serial.println(statusCode);
#endif

    bool refreshed = false;
    if (statusCode == 200)
    {
        DynamicJsonDocument doc(1000);
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            sprintf(this->_bearerToken, "Bearer %s", doc["access_token"].as<char *>());
            int tokenTtl = doc["expires_in"];             // Usually 3600 (1 hour)
            tokenTimeToLiveMs = (tokenTtl * 1000) - 2000; // The 2000 is just to force the token expiry to check if its very close
            timeTokenRefreshed = now;
            refreshed = true;
        }
    }
    else
    {
        parseError();
    }

    closeClient();
    return refreshed;
}

bool ArduinoSpotify::checkAndRefreshAccessToken()
{
    unsigned long timeSinceLastRefresh = millis() - timeTokenRefreshed;
    if (timeSinceLastRefresh >= tokenTimeToLiveMs)
    {
        Serial.println("Refresh of the Access token is due, doing that now.");
        return refreshAccessToken();
    }

    // Token is still valid
    return true;
}

const char *ArduinoSpotify::requestAccessTokens(const char *code, const char *redirectUrl)
{

    char body[500];
    sprintf(body, requestAccessTokensBody, code, redirectUrl, _clientId, _clientSecret);

#ifdef SPOTIFY_DEBUG
    Serial.println(body);
#endif

    int statusCode = makePostRequest(SPOTIFY_TOKEN_ENDPOINT, NULL, body, "application/x-www-form-urlencoded", SPOTIFY_ACCOUNTS_HOST);
    if (statusCode > 0)
    {
        skipHeaders();
    }
    unsigned long now = millis();

#ifdef SPOTIFY_DEBUG
    Serial.print("status Code");
    Serial.println(statusCode);
#endif

    if (statusCode == 200)
    {
        DynamicJsonDocument doc(1000);
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            sprintf(this->_bearerToken, "Bearer %s", doc["access_token"].as<char *>());
            _refreshToken = doc["refresh_token"].as<char *>();
            int tokenTtl = doc["expires_in"];             // Usually 3600 (1 hour)
            tokenTimeToLiveMs = (tokenTtl * 1000) - 2000; // The 2000 is just to force the token expiry to check if its very close
            timeTokenRefreshed = now;
        }
    }
    else
    {
        parseError();
    }

    closeClient();
    return _refreshToken;
}

bool ArduinoSpotify::play(const char *deviceId)
{
    char command[100] = SPOTIFY_PLAY_ENDPOINT;
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::playAdvanced(char *body, const char *deviceId)
{
    char command[100] = SPOTIFY_PLAY_ENDPOINT;
    return playerControl(command, deviceId, body);
}

bool ArduinoSpotify::pause(const char *deviceId)
{
    char command[100] = SPOTIFY_PAUSE_ENDPOINT;
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::setVolume(int volume, const char *deviceId)
{
    char command[125];
    sprintf(command, SPOTIFY_VOLUME_ENDPOINT, volume);
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::toggleShuffle(bool shuffle, const char *deviceId)
{
    char command[125];
    char shuffleState[10];
    if (shuffle)
    {
        strcpy(shuffleState, "true");
    }
    else
    {
        strcpy(shuffleState, "false");
    }
    sprintf(command, SPOTIFY_SHUFFLE_ENDPOINT, shuffleState);
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::setRepeatMode(RepeatOptions repeat, const char *deviceId)
{
    char command[125];
    char repeatState[10];
    switch (repeat)
    {
    case repeat_track:
        strcpy(repeatState, "track");
        break;
    case repeat_context:
        strcpy(repeatState, "context");
        break;
    case repeat_off:
        strcpy(repeatState, "off");
        break;
    }

    sprintf(command, SPOTIFY_REPEAT_ENDPOINT, repeatState);
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::playerControl(char *command, const char *deviceId, const char *body)
{
    if (deviceId[0] != 0)
    {
        char *questionMarkPointer;
        questionMarkPointer = strchr(command, '?');
        char deviceIdBuff[50];
        if (questionMarkPointer == NULL)
        {
            sprintf(deviceIdBuff, "?device_id=%s", deviceId);
        }
        else
        {
            // params already started
            sprintf(deviceIdBuff, "&device_id=%s", deviceId);
        }
        strcat(command, deviceIdBuff);
    }

#ifdef SPOTIFY_DEBUG
    Serial.println(command);
    Serial.println(body);
#endif

    if (autoTokenRefresh)
    {
        checkAndRefreshAccessToken();
    }
    int statusCode = makePutRequest(command, _bearerToken, body);

    closeClient();
    //Will return 204 if all went well.
    return statusCode == 204;
}

bool ArduinoSpotify::playerNavigate(char *command, const char *deviceId)
{
    if (deviceId[0] != 0)
    {
        char deviceIdBuff[50];
        sprintf(deviceIdBuff, "?device_id=%s", deviceId);
        strcat(command, deviceIdBuff);
    }

#ifdef SPOTIFY_DEBUG
    Serial.println(command);
#endif

    if (autoTokenRefresh)
    {
        checkAndRefreshAccessToken();
    }
    int statusCode = makePostRequest(command, _bearerToken);

    closeClient();
    //Will return 204 if all went well.
    return statusCode == 204;
}

bool ArduinoSpotify::nextTrack(const char *deviceId)
{
    char command[100] = SPOTIFY_NEXT_TRACK_ENDPOINT;
    return playerNavigate(command, deviceId);
}

bool ArduinoSpotify::previousTrack(const char *deviceId)
{
    char command[100] = SPOTIFY_PREVIOUS_TRACK_ENDPOINT;
    return playerNavigate(command, deviceId);
}
bool ArduinoSpotify::seek(int position, const char *deviceId)
{
    char command[100] = SPOTIFY_SEEK_ENDPOINT;
    char tempBuff[100];
    sprintf(tempBuff, "?position_ms=%d", position);
    strcat(command, tempBuff);
    if (deviceId[0] != 0)
    {
        sprintf(tempBuff, "?device_id=%s", deviceId);
        strcat(command, tempBuff);
    }

#ifdef SPOTIFY_DEBUG
    Serial.println(command);
    printStack();
#endif

    if (autoTokenRefresh)
    {
        checkAndRefreshAccessToken();
    }
    int statusCode = makePutRequest(command, _bearerToken);
    closeClient();
    //Will return 204 if all went well.
    return statusCode == 204;
}

bool ArduinoSpotify::transferPlayback(const char *deviceId, bool play)
{
    char body[100];
    sprintf(body, "{\"device_ids\":[\"%s\"],\"play\":\"%s\"}", deviceId, (play?"true":"false"));

#ifdef SPOTIFY_DEBUG
    Serial.println(SPOTIFY_PLAYER_ENDPOINT);
    Serial.println(body);
    printStack();
#endif

    if (autoTokenRefresh)
    {
        checkAndRefreshAccessToken();
    }
    int statusCode = makePutRequest(SPOTIFY_PLAYER_ENDPOINT, _bearerToken, body);
    closeClient();
    //Will return 204 if all went well.
    return statusCode == 204;
}

CurrentlyPlaying ArduinoSpotify::getCurrentlyPlaying(const char *market)
{
    char command[50] = SPOTIFY_CURRENTLY_PLAYING_ENDPOINT;
    if (market[0] != 0)
    {
        char marketBuff[15];
        sprintf(marketBuff, "?market=%s", market);
        strcat(command, marketBuff);
    }

#ifdef SPOTIFY_DEBUG
    Serial.println(command);
    printStack();
#endif

    // Get from https://arduinojson.org/v6/assistant/
    const size_t bufferSize = currentlyPlayingBufferSize;
    //CurrentlyPlaying currentlyPlaying;
    // This flag will get cleared if all goes well
    currentlyPlaying.error = true;
    if (autoTokenRefresh)
    {
        checkAndRefreshAccessToken();
    }
    int statusCode = makeGetRequest(command, _bearerToken);
#ifdef SPOTIFY_DEBUG
    Serial.print("Status Code: ");
    Serial.println(statusCode);
    printStack();
#endif
    if (statusCode > 0)
    {
        skipHeaders();
        currentlyPlaying.statusCode = statusCode;
    }

    if (statusCode == 200)
    {
        //Apply Json Filter: https://arduinojson.org/v6/example/filter/
        DynamicJsonDocument filter(288);
        //StaticJsonDocument<288> filter;
        filter["is_playing"] = true;
        filter["progress_ms"] = true;

        JsonObject filter_item = filter.createNestedObject("item");
        filter_item["duration_ms"] = true;
        filter_item["name"] = true;
        filter_item["uri"] = true;

        JsonObject filter_item_artists_0 = filter_item["artists"].createNestedObject();
        filter_item_artists_0["name"] = true;
        filter_item_artists_0["uri"] = true;

        JsonObject filter_item_album = filter_item.createNestedObject("album");
        filter_item_album["name"] = true;
        filter_item_album["uri"] = true;

        JsonObject filter_item_album_images_0 = filter_item_album["images"].createNestedObject();
        filter_item_album_images_0["height"] = true;
        filter_item_album_images_0["width"] = true;
        filter_item_album_images_0["url"] = true;

        // Allocate DynamicJsonDocument
        DynamicJsonDocument doc(bufferSize);

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, *client, DeserializationOption::Filter(filter));
        if (!error)
        {
#ifdef SPOTIFY_DEBUG
            serializeJsonPretty(doc, Serial);
#endif
            JsonObject item = doc["item"];
            JsonObject firstArtist = item["artists"][0];

            strncpy(currentlyPlaying.firstArtistName, firstArtist["name"].as<char *>(), SPOTIFY_NAME_CHAR_LENGTH);
            currentlyPlaying.firstArtistName[SPOTIFY_NAME_CHAR_LENGTH-1] = '\0'; //In case the song was longer than the size of buffer
            strncpy(currentlyPlaying.firstArtistUri, firstArtist["uri"].as<char *>(), SPOTIFY_URI_CHAR_LENGTH);
            currentlyPlaying.firstArtistUri[SPOTIFY_URI_CHAR_LENGTH-1] = '\0';
            //currentlyPlaying.firstArtistName = (char *)firstArtist["name"].as<char *>();
            //currentlyPlaying.firstArtistUri = (char *)firstArtist["uri"].as<char *>();

            strncpy(currentlyPlaying.albumName, item["album"]["name"].as<char *>(), SPOTIFY_NAME_CHAR_LENGTH);
            currentlyPlaying.albumName[SPOTIFY_NAME_CHAR_LENGTH-1] = '\0';
            strncpy(currentlyPlaying.albumUri, item["album"]["uri"].as<char *>(), SPOTIFY_URI_CHAR_LENGTH);
            currentlyPlaying.albumUri[SPOTIFY_URI_CHAR_LENGTH-1] = '\0';
            //currentlyPlaying.albumName = (char *)item["album"]["name"].as<char *>();
            //currentlyPlaying.albumUri = (char *)item["album"]["uri"].as<char *>();

            JsonArray images = item["album"]["images"];

            // Images are returned in order of width, so last should be smallest.
            int numImages = images.size();
            int startingIndex = 0;
            if (numImages > SPOTIFY_NUM_ALBUM_IMAGES)
            {
                startingIndex = numImages - SPOTIFY_NUM_ALBUM_IMAGES;
                currentlyPlaying.numImages = SPOTIFY_NUM_ALBUM_IMAGES;
            }
            else
            {
                currentlyPlaying.numImages = numImages;
            }
#ifdef SPOTIFY_DEBUG
            Serial.print(F("Num Images: "));
            Serial.println(currentlyPlaying.numImages);
            Serial.println(numImages);
#endif

            for (int i = 0; i < currentlyPlaying.numImages; i++)
            {
                int adjustedIndex = startingIndex + i;
                currentlyPlaying.albumImages[i].height = images[adjustedIndex]["height"].as<int>();
                currentlyPlaying.albumImages[i].width = images[adjustedIndex]["width"].as<int>();
                strncpy(currentlyPlaying.albumImages[i].url, images[adjustedIndex]["url"].as<char *>(), SPOTIFY_URL_CHAR_LENGTH);
                currentlyPlaying.albumImages[i].url[SPOTIFY_URL_CHAR_LENGTH-1] = '\0';
                //currentlyPlaying.albumImages[i].url = (char *)images[adjustedIndex]["url"].as<char *>();
            }

            strncpy(currentlyPlaying.trackName, item["name"].as<char *>(), SPOTIFY_NAME_CHAR_LENGTH);
            currentlyPlaying.trackName[SPOTIFY_NAME_CHAR_LENGTH-1] = '\0';
            strncpy(currentlyPlaying.trackUri, item["uri"].as<char *>(), SPOTIFY_URI_CHAR_LENGTH);
            currentlyPlaying.trackUri[SPOTIFY_URI_CHAR_LENGTH-1] = '\0';
            //currentlyPlaying.trackName = (char *)item["name"].as<char *>();
            //currentlyPlaying.trackUri = (char *)item["uri"].as<char *>();

            currentlyPlaying.isPlaying = doc["is_playing"].as<bool>();

            currentlyPlaying.progressMs = doc["progress_ms"].as<long>();
            currentlyPlaying.duraitonMs = item["duration_ms"].as<long>();

            currentlyPlaying.error = false;
        }
        else
        {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
        }
    }
    if (statusCode == 204)
    {
        currentlyPlaying.error = false;
    }
    closeClient();
    return currentlyPlaying;
}

PlayerDetails ArduinoSpotify::getPlayerDetails(const char *market)
{
    char command[100] = SPOTIFY_PLAYER_ENDPOINT;
    if (market[0] != 0)
    {
        char marketBuff[30];
        sprintf(marketBuff, "?market=%s", market);
        strcat(command, marketBuff);
    }

#ifdef SPOTIFY_DEBUG
    Serial.println(command);
    printStack();
#endif

    // Get from https://arduinojson.org/v6/assistant/
    const size_t bufferSize = playerDetailsBufferSize;
    // This flag will get cleared if all goes well
    playerDetails.error = true;
    if (autoTokenRefresh)
    {
        checkAndRefreshAccessToken();
    }

    int statusCode = makeGetRequest(command, _bearerToken);
#ifdef SPOTIFY_DEBUG
    Serial.print("Status Code: ");
    Serial.println(statusCode);
#endif
    if (statusCode > 0)
    {
        skipHeaders();
        playerDetails.statusCode = statusCode;
    }

    if (statusCode == 200)
    {

        //StaticJsonDocument<192> filter;
        DynamicJsonDocument filter(192);
        JsonObject filter_device = filter.createNestedObject("device");
        filter_device["id"] = true;
        filter_device["name"] = true;
        filter_device["type"] = true;
        filter_device["is_active"] = true;
        filter_device["is_private_session"] = true;
        filter_device["is_restricted"] = true;
        filter_device["volume_percent"] = true;
        filter["progress_ms"] = true;
        filter["is_playing"] = true;
        filter["shuffle_state"] = true;
        filter["repeat_state"] = true;

        // Allocate DynamicJsonDocument
        DynamicJsonDocument doc(bufferSize);

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, *client, DeserializationOption::Filter(filter));
        if (!error)
        {
            JsonObject device = doc["device"];
            // Copy into buffer and make the last character a null just incase we went over.
            strncpy(playerDetails.device.id, (char *)device["id"].as<char *>(), SPOTIFY_DEVICE_ID_CHAR_LENGTH);
            playerDetails.device.id[SPOTIFY_DEVICE_ID_CHAR_LENGTH-1] = '\0';

            strncpy(playerDetails.device.name, (char *)device["name"].as<char *>(), SPOTIFY_DEVICE_NAME_CHAR_LENGTH);
            playerDetails.device.name[SPOTIFY_DEVICE_NAME_CHAR_LENGTH-1] = '\0';

            strncpy(playerDetails.device.type, (char *)device["type"].as<char *>(), SPOTIFY_DEVICE_TYPE_CHAR_LENGTH);
            playerDetails.device.type[SPOTIFY_DEVICE_TYPE_CHAR_LENGTH-1] = '\0';

            playerDetails.device.isActive = device["is_active"].as<bool>();
            playerDetails.device.isPrivateSession = device["is_private_session"].as<bool>();
            playerDetails.device.isRestricted = device["is_restricted"].as<bool>();
            playerDetails.device.volumePercent = device["volume_percent"].as<int>();

            playerDetails.progressMs = doc["progress_ms"].as<long>();
            playerDetails.isPlaying = doc["is_playing"].as<bool>();

            playerDetails.shuffleState = doc["shuffle_state"].as<bool>();

            const char *repeat_state = doc["repeat_state"]; 

            if (strncmp(repeat_state, "track", 5) == 0)
            {
                playerDetails.repeateState = repeat_track;
            }
            else if (strncmp(repeat_state, "context", 7) == 0)
            {
                playerDetails.repeateState = repeat_context;
            }
            else
            {
                playerDetails.repeateState = repeat_off;
            }

            playerDetails.error = false;
        }
        else
        {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
        }
    }
    if (statusCode == 204)
    {
        playerDetails.error = false;
    }
    closeClient();
    return playerDetails;
}

int ArduinoSpotify::getDevices(SpotifyDevice devices[], uint8_t maxDevices)
{

#ifdef SPOTIFY_DEBUG
    Serial.println(SPOTIFY_DEVICES_ENDPOINT);
    printStack();
#endif

    // Get from https://arduinojson.org/v6/assistant/
    const size_t bufferSize = getDevicesBufferSize;
    // This flag will get cleared if all goes well
    int returnCode = -1;
    if (autoTokenRefresh)
    {
        checkAndRefreshAccessToken();
    }

    int statusCode = makeGetRequest(SPOTIFY_DEVICES_ENDPOINT, _bearerToken);
#ifdef SPOTIFY_DEBUG
    Serial.print("Status Code: ");
    Serial.println(statusCode);
#endif
    if (statusCode > 0)
    {
        skipHeaders();
        playerDetails.statusCode = statusCode;
    }

    if (statusCode == 200)
    {

        // Allocate DynamicJsonDocument
        DynamicJsonDocument doc(bufferSize);

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            uint8_t devicesToReturn = 0;
            uint8_t totalDevices = doc["devices"].size();
            uint8_t startingIndex = 0;
            if (totalDevices > maxDevices)
            {
                startingIndex = totalDevices - maxDevices;
                devicesToReturn = maxDevices;
            }
            else
            {
                devicesToReturn = totalDevices;
            }

            for (int i = 0; i < devicesToReturn; i++)
            {
                int adjustedIndex = startingIndex + i;
                JsonObject device = doc["devices"][adjustedIndex];
                // Copy into buffer and make the last character a null just incase we went over.
                strncpy(devices[i].id, (char *)device["id"].as<char *>(), SPOTIFY_DEVICE_ID_CHAR_LENGTH);
                devices[i].id[SPOTIFY_DEVICE_ID_CHAR_LENGTH-1] = '\0';

                strncpy(devices[i].name, (char *)device["name"].as<char *>(), SPOTIFY_DEVICE_NAME_CHAR_LENGTH);
                devices[i].name[SPOTIFY_DEVICE_NAME_CHAR_LENGTH-1] = '\0';

                strncpy(devices[i].type, (char *)device["type"].as<char *>(), SPOTIFY_DEVICE_TYPE_CHAR_LENGTH);
                devices[i].type[SPOTIFY_DEVICE_TYPE_CHAR_LENGTH-1] = '\0';

                devices[i].isActive = device["is_active"].as<bool>();
                devices[i].isPrivateSession = device["is_private_session"].as<bool>();
                devices[i].isRestricted = device["is_restricted"].as<bool>();
                devices[i].volumePercent = device["volume_percent"].as<int>();

            }

            returnCode = devicesToReturn;
        }
        else
        {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
        }
    }

    closeClient();
    return returnCode;
}

int ArduinoSpotify::commonGetImage(char *imageUrl)
{
#ifdef SPOTIFY_DEBUG
    Serial.print(F("Parsing image URL: "));
    Serial.println(imageUrl);
#endif

    uint8_t lengthOfString = strlen(imageUrl);

    // We are going to just assume https, that's all I've
    // seen and I can't imagine a company will go back
    // to http

    if (strncmp(imageUrl, "https://", 8) != 0)
    {
        Serial.print(F("Url not in expected format: "));
        Serial.println(imageUrl);
        Serial.println("(expected it to start with \"https://\")");
        return false;
    }

    uint8_t protocolLength = 8;

    char *pathStart = strchr(imageUrl + protocolLength, '/');
    uint8_t pathIndex = pathStart - imageUrl;
    uint8_t pathLength = lengthOfString - pathIndex;
    char path[pathLength + 1];
    strncpy(path, pathStart, pathLength);
    path[pathLength] = '\0';

    uint8_t hostLength = pathIndex - protocolLength;
    char host[hostLength + 1];
    strncpy(host, imageUrl + protocolLength, hostLength);
    host[hostLength] = '\0';

#ifdef SPOTIFY_DEBUG

    Serial.print(F("host: "));
    Serial.println(host);

    Serial.print(F("len:host:"));
    Serial.println(hostLength);

    Serial.print(F("path: "));
    Serial.println(path);

    Serial.print(F("len:path: "));
    Serial.println(strlen(path));
#endif

    int statusCode = makeGetRequest(path, NULL, "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8", host);
#ifdef SPOTIFY_DEBUG
    Serial.print(F("statusCode: "));
    Serial.println(statusCode);
#endif
    if (statusCode == 200)
    {
        return getContentLength();
    }

    // Failed
    return -1;
}

bool ArduinoSpotify::getImage(char *imageUrl, Stream *file)
{
    int totalLength = commonGetImage(imageUrl);

#ifdef SPOTIFY_DEBUG
    Serial.print(F("file length: "));
    Serial.println(totalLength);
#endif
    if (totalLength > 0)
    {
        skipHeaders(false);
        int remaining = totalLength;
        // This section of code is inspired but the "Web_Jpg"
        // example of TJpg_Decoder
        // https://github.com/Bodmer/TJpg_Decoder
        // -----------
        uint8_t buff[128] = {0};
        while (client->connected() && (remaining > 0 || remaining == -1))
        {
            // Get available data size
            size_t size = client->available();

            if (size)
            {
                // Read up to 128 bytes
                int c = client->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                // Write it to file
                file->write(buff, c);

                // Calculate remaining bytes
                if (remaining > 0)
                {
                    remaining -= c;
                }
            }
            yield();
        }
// ---------
#ifdef SPOTIFY_DEBUG
        Serial.println(F("Finished getting image"));
#endif
    }

    closeClient();

    return (totalLength > 0); //Probably could be improved!
}

bool ArduinoSpotify::getImage(char *imageUrl, uint8_t **image, int *imageLength)
{
    int totalLength = commonGetImage(imageUrl);

#ifdef SPOTIFY_DEBUG
    Serial.print(F("file length: "));
    Serial.println(totalLength);
#endif
    if (totalLength > 0)
    {
        skipHeaders(false);
        uint8_t *imgPtr = (uint8_t *) malloc(totalLength);
        *image = imgPtr;
        *imageLength = totalLength;
        int remaining = totalLength;
        int amountRead = 0;

#ifdef SPOTIFY_DEBUG
    Serial.println(F("Fetching Image"));
#endif

        // This section of code is inspired but the "Web_Jpg"
        // example of TJpg_Decoder
        // https://github.com/Bodmer/TJpg_Decoder
        // -----------
        uint8_t buff[128] = {0};
        while (client->connected() && (remaining > 0 || remaining == -1))
        {
            // Get available data size
            size_t size = client->available();

            if (size)
            {
                // Read up to 128 bytes
                int c = client->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                // Write it to file
                memcpy( (uint8_t*)imgPtr + amountRead , (uint8_t*)buff , c );

                // Calculate remaining bytes
                if (remaining > 0)
                {
                    amountRead += c;
                    remaining -= c;
                }
            }
            yield();
        }
// ---------
#ifdef SPOTIFY_DEBUG
        Serial.println(F("Finished getting image"));
#endif
    }

    closeClient();

    return (totalLength > 0); //Probably could be improved!
}

int ArduinoSpotify::getContentLength()
{

    if (client->find("Content-Length:"))
    {
        int contentLength = client->parseInt();
#ifdef SPOTIFY_DEBUG
        Serial.print(F("Content-Length: "));
        Serial.println(contentLength);
#endif
        return contentLength;
    }

    return -1;
}

void ArduinoSpotify::skipHeaders(bool tossUnexpectedForJSON)
{
    // Skip HTTP headers
    if (!client->find("\r\n\r\n"))
    {
        Serial.println(F("Invalid response"));
        return;
    }

    if (tossUnexpectedForJSON)
    {
        // Was getting stray characters between the headers and the body
        // This should toss them away
        while (client->available() && client->peek() != '{')
        {
            char c = 0;
            client->readBytes(&c, 1);
#ifdef SPOTIFY_DEBUG
            Serial.print(F("Tossing an unexpected character: "));
            Serial.println(c);
#endif
        }
    }
}

int ArduinoSpotify::getHttpStatusCode()
{
    // Check HTTP status
    if (client->find("HTTP/1.1"))
    {
        int statusCode = client->parseInt();
#ifdef SPOTIFY_DEBUG
        Serial.print(F("Status Code: "));
        Serial.println(statusCode);
#endif
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
    }
    else
    {
        Serial.print(F("Could not parse error"));
    }
}

void ArduinoSpotify::lateInit(Client &client, const char *clientId, const char *clientSecret, const char *refreshToken)
{
    this->client = &client;
    this->_clientId = clientId;
    this->_clientSecret = clientSecret;
    this->_refreshToken = refreshToken;
    initStructs();
}

void ArduinoSpotify::initStructs()
{
    currentlyPlaying.firstArtistName = (char *)malloc(SPOTIFY_NAME_CHAR_LENGTH);
    currentlyPlaying.firstArtistUri = (char *)malloc(SPOTIFY_URI_CHAR_LENGTH);
    currentlyPlaying.albumName = (char *)malloc(SPOTIFY_NAME_CHAR_LENGTH);
    currentlyPlaying.albumUri = (char *)malloc(SPOTIFY_URI_CHAR_LENGTH);
    currentlyPlaying.trackName = (char *)malloc(SPOTIFY_NAME_CHAR_LENGTH);
    currentlyPlaying.trackUri = (char *)malloc(SPOTIFY_URI_CHAR_LENGTH);
    for(int i = 0; i < SPOTIFY_NUM_ALBUM_IMAGES; i++){
        currentlyPlaying.albumImages[i].url = (char *)malloc(SPOTIFY_URL_CHAR_LENGTH);
    }

    playerDetails.device.id = (char *)malloc(SPOTIFY_DEVICE_ID_CHAR_LENGTH);
    playerDetails.device.name = (char *)malloc(SPOTIFY_DEVICE_NAME_CHAR_LENGTH);
    playerDetails.device.type = (char *)malloc(SPOTIFY_DEVICE_TYPE_CHAR_LENGTH);

}

// Not sure why this would ever be needed, but sure why not.
void ArduinoSpotify::destroyStructs()
{
    free(currentlyPlaying.firstArtistName);
    free(currentlyPlaying.firstArtistUri);
    free(currentlyPlaying.albumName);
    free(currentlyPlaying.albumUri);
    free(currentlyPlaying.trackName);
    free(currentlyPlaying.trackUri);
    for(int i = 0; i < SPOTIFY_NUM_ALBUM_IMAGES; i++){
        free(currentlyPlaying.albumImages[i].url);
    }

    free(playerDetails.device.id);
    free(playerDetails.device.name);
    free(playerDetails.device.type);
}

SpotifyDevice* ArduinoSpotify::generateDevicesArray(uint8_t size)
{
    SpotifyDevice* devices = (SpotifyDevice*)malloc(sizeof(SpotifyDevice) * size);

    for(int i = 0; i < size; i++){
        devices[i].id = (char *)malloc(SPOTIFY_DEVICE_ID_CHAR_LENGTH);
        devices[i].name = (char *)malloc(SPOTIFY_DEVICE_NAME_CHAR_LENGTH);
        devices[i].type = (char *)malloc(SPOTIFY_DEVICE_TYPE_CHAR_LENGTH);
    }

    return devices;
}

void ArduinoSpotify::destroyDevicesArray(SpotifyDevice* devices, uint8_t size)
{

    for(int i = 0; i < size; i++){
        free(devices[i].id);
        free(devices[i].name);
        free(devices[i].type);
    }

    free(devices);
}

void ArduinoSpotify::closeClient()
{
    if (client->connected())
    {
#ifdef SPOTIFY_DEBUG
        Serial.println(F("Closing client"));
#endif
        client->stop();
    }
}

#ifdef SPOTIFY_DEBUG
void ArduinoSpotify::printStack()
{
    char stack;
    Serial.print (F("stack size "));
    Serial.println (stack_start - &stack);
}
#endif
