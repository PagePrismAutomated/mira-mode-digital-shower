basic wireless bluetooth on/off control for Mira Mode shower/bath
tested on Mira Mode dual outlet digital shower/bath purchased 2023 in UK
tested on old ESP32-WROOM-32 and ESP32-WROOM-32E (FireBeetle 2 dev. board)
Mira Mode requires pairing before you can write to characteristics
hold shower on button for 6 seconds until light flashes, then send 'p' over serial to pair
send commands from serial to test then reduce to a device with buttons etc. 
inspired by this library https://github.com/alexpilotti/python-miramode but comms different
included code to receive notifications so can be developed to get feedback etc.
