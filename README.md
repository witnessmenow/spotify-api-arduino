# arduino-spotify-api
Arduino library for integrating with the Spotify Web-API (Does not play music)

**Work in progress library - expect changes!**

### Currently supports:

- Get Refresh Token (for authenticating calls)
- Getting your currently playing track
- Player Controls: (Example needs to be updated)
    - Next
    - Previous
    - Seek

### What needs to be added:

- Update getAuthToken example to remove FS stuff
- Update playerControls example to work
- Update readme with info on how to setup
- Play/Pause
- Volume Control
- Current play time and song length to currently playing.


### Scopes

put a `%20` between the ones you need.

| Feature        | Required Scope          
| ------------- |-------------| 
| Current Playing Song Info      | user-read-playback-state |
| Player Controls      | user-modify-playback-state      |
