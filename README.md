# arduino-spotify-api
Arduino library for integrating with the Spotify Web-API (Does not play music)

**Work in progress library - expect changes!**

### Currently supports:

- Get Refresh Token (for authenticating calls)
- Getting your currently playing track
- Player Controls:
    - Next
    - Previous
    - Seek
    - Play (basic version, basically resumes a paused track)
    - Pause
    - Set Volume (doesn't seem to work on my phone, works on desktop though)
    - Set Repeat Modes
    - Toggle Shuffle

### What needs to be added:

- Add a solution for expired tokens (hopefully done, needs to be tested)
- Current play time and song length to currently playing.
- Save more info on player devices and state.


### Scopes

put a `%20` between the ones you need.

| Feature        | Required Scope          
| ------------- |-------------| 
| Current Playing Song Info      | user-read-playback-state |
| Player Controls      | user-modify-playback-state      |
