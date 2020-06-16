# arduino-spotify-api
Arduino library for integrating with the Spotify Web-API (Does not play music)

**Work in progress library - expect changes!**

### Currently supports:

- Getting your currently playing track
- Player Controls:
    - Next
    - Previous
    - Seek

### What needs to be added:

- Correct support of OAuth
    - Implement saving refresh token to persistant memory
    - Implement Refresh token path
    - Investigate putting clientID and secret in body ("An alternative way to send the client id and secret is as request parameters (client_id and client_secret) in the POST body, instead of sending them base64-encoded in the header")
- Play/Pause
- Volume Control
- Current play time and song length to currently playing.


### Scopes

put a `%20` between the ones you need.

| Feature        | Required Scope          
| ------------- |-------------| 
| Current Playing Song Info      | user-read-playback-state |
| Player Controls      | user-modify-playback-state      |