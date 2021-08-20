# spotify-api-arduino

![Travis CI status](https://api.travis-ci.org/witnessmenow/arduino-spotify-api.svg?branch=master)
![License](https://img.shields.io/github/license/witnessmenow/spotify-api-arduino)
![Release stable](https://badgen.net/github/release/witnessmenow/spotify-api-arduino/stable)  
Arduino library for integrating with a subset of the [Spotify Web-API](https://developer.spotify.com/documentation/web-api/reference/) (Does not play music)

**Work in progress library - expect changes!**

## Supported Boards:

### ESP32

Working well

### ESP8266

Working well

### Other boards - Arduino Wifi Nina (Nano IOT etc)

Should in theory work, but I have no tested it on them and will not be able to provide support for them.

## Help support what I do!

I have put a lot of effort into creating Arduino libraries that I hope people can make use of. [If you enjoy my work, please consider becoming a Github sponsor!](https://github.com/sponsors/witnessmenow/)

## Library Features:

The Library supports the following features:

- Get Authentication Tokens
- Getting your currently playing track
- Player Controls:
  - Next
  - Previous
  - Seek
  - Play (basic version, basically resumes a paused track)
  - Play Advanced (play given song, album, artist)
  - Pause
  - Set Volume (doesn't seem to work on my phone, works on desktop though)
  - Set Repeat Modes
  - Toggle Shuffle
- Get Devices

### What needs to be added:

- Better instructions for how to set up your refresh token.
- Example where refresh token and full operation are handled in same sketch.

## Setup Instructions

### Spotify Account

- Sign into the [Spotify Developer page](https://developer.spotify.com/dashboard/login)
- Create a new application. (name it whatever you want)
- You will need to use the "client ID" and "client secret" from this page in your sketches
- You will also need to add a callback URI for authentication process by clicking "Edit Settings", what URI to add will be mentioned in further instructions

### Getting Your Refresh Token

Spotify's Authentication flow requires a webserver to complete, but it's only needed once to get your refresh token. Your refresh token can then be used in all future sketches to authenticate.

Because the webserver is only needed once, I decided to seperate the logic for getting the Refresh token to it's own example.

Follow the instructions in the [getRefreshToken example](examples/getRefreshToken/getRefreshToken.ino) to get your token.

Note: Once you have a refresh token, you can use it on either platform in your sketches, it is not tied to any particular device.

### Running

Take one of the included examples and update it with your WiFi creds, Client ID, Client Secret and the refresh token you just generated.

### Scopes

By default the getRefreshToken examples will include the required scopes, but if you want change them the following info might be useful.

put a `%20` between the ones you need.

| Feature                   | Required Scope             |
| ------------------------- | -------------------------- |
| Current Playing Song Info | user-read-playback-state   |
| Player Controls           | user-modify-playback-state |

## Installation

Download zip from Github and install to the Arduino IDE using that.

#### Dependancies

- V6 of Arduino JSON - can be installed through the Arduino Library manager.

## Compile flag configuration

There are some flags that you can set in the `SpotifyArduino.h` that can help with debugging

```

#define SPOTIFY_DEBUG 1
// Enables extra debug messages on the serial.
// Will be disabled by default when library is released.
// NOTE: Do not use this option on live-streams, it will reveal your private tokens!

#define SPOTIFY_SERIAL_OUTPUT 1
// Comment out if you want to disable any serial output from this library
// (also comment out DEBUG and PRINT_JSON_PARSE)

//#define SPOTIFY_PRINT_JSON_PARSE 1
// Prints the JSON received to serial (only use for debugging as it will be slow)
// Requires the installation of ArduinoStreamUtils (https://github.com/bblanchon/ArduinoStreamUtils)

```
