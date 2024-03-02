Basic wireless bluetooth on/off control for Mira Mode shower / bath. Tested on Mira Mode dual outlet digital shower / bath purchased 2023 in UK using ESP32-WROOM-32 and ESP32-WROOM-32E (FireBeetle 2 dev. board).

Mira Mode requires pairing before you can write to the characteristics. To pair: on the main shower control hold shower 'on' button for 6 seconds until light flashes, then send 'p' over serial to pair. 

Send commands from serial to test then reduce to a device with buttons etc. Originally designed as a simple remote switch but could be extended to use wifi capabilities of ESP32 and interface with Alexa, Google Home. 

Included code to receive notifications so can be developed to get feedback by sending appropriate request. I have not figured these out yet. 

Inspired by this library https://github.com/alexpilotti/python-miramode but communication structure of my Mira Mode was different.

