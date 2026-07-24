# Women's Safety Alert System

An Arduino-based emergency alert system for personal safety during travel or commutes. Pressing a panic button fetches the current GPS location and sends it via SMS to pre-saved emergency contacts, while a buzzer sounds to draw immediate attention. A "STOP" SMS reply silences the alert and resets the system.

Built as the capstone project for the Maven Silicon Embedded System Design certification.

## How it works

1. On startup, the system initializes the LCD, GPS module, and GSM module, and displays "System Ready."
2. The main loop continuously reads GPS coordinates and monitors the panic button.
3. On panic button press: the buzzer turns on, the current GPS coordinates are read, and an emergency SMS with those coordinates is sent to the saved contact number(s). The LCD updates to show alert status.
4. The alert stays active until a "STOP" SMS is received from the contact, which silences the buzzer and resets the LCD.

## Hardware

| Component | Role |
|---|---|
| Arduino Uno | Central controller |
| GPS Module (NEO-6M) | Location tracking |
| GSM Module (SIM800L) | Sends emergency SMS |
| I2C LCD (16x2) | Displays system/alert status |
| Push Button | Panic button trigger |
| Buzzer | Audible alert |

**Wiring:**
- GPS module → Arduino digital pins (TX/RX) via SoftwareSerial (pins 10, 11)
- GSM module → Arduino hardware Serial port
- LCD → I2C (SDA/SCL)
- Buzzer → Digital Pin 13
- Panic Button → Digital Pin 7 (with internal pull-up)

## Software architecture

The code is organized into functional blocks:
- **Core control:** `setup()`, `loop()`
- **Emergency response:** `tracking()`, `send_sms()`
- **Peripheral interface:** `get_gps()`, `gpsEvent()`
- **Communication & display:** `gsm_init()`, `serialEvent()`, `lcd_status()`

GSM communication is handled through AT commands (module detection, SMS text mode configuration, network registration check) sent over the hardware serial port. GPS data is parsed directly from raw NMEA `$GPGGA` sentences to extract latitude and longitude.

See [`src/women_safety_alert_system.ino`](src/women_safety_alert_system.ino) for the full code.

## Testing results

- GPS accuracy: ~5-10 meter margin, with lock time of 15-40 seconds outdoors.
- Panic button to SMS delivery: 10-20 seconds under normal network conditions.
- STOP command deactivated the buzzer and reset the system within 5-10 seconds.
- System operated continuously on backup battery during power-disconnection tests.

## Future enhancements

- Companion mobile app for live tracking and instant notifications
- Cloud logging of alert history and GPS locations
- Camera module for automatic evidence capture during emergencies
- Wearable device trigger for more discreet activation
- Voice-activated SOS using keyword recognition

## Tech stack

Arduino Uno, Embedded C/C++, GPS (NMEA parsing), GSM (AT commands), I2C
