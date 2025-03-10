# Bosconnect - Intelligent Home Automation System
## Project Overview

Bosconnect is a home automation project aimed at designing a versatile connected system for monitoring and controlling a domestic environment. Developed as part of a second-year project in BUT GEII (Electrical Engineering and Industrial Computing), it offers a complete solution combining custom hardware and an ergonomic web interface.

<br>
<div align="center">
  <figure>
    <img src="docs/carte_domotique_pcb.PNG" alt="Bosconnect Board" width="500" style="max-width: 100%;">
    <p><em>Overview of the Bosconnect home automation board with its components</em></p>
  </figure>
</div>
<br>

## Objectives

1. **Create an accessible home automation hub** allowing any user to monitor and control their home
2. **Develop an integrated solution** combining environmental sensors and device control
3. **Offer an intuitive interface** both on the physical device and remotely via the web
4. **Enable task automation** based on time or environmental conditions

## System Operation

The Bosconnect system is based on two fundamental components:

### 1. ESP32 Hardware Module

At the heart of the system is an electronic board based on an ESP32 that:
- **Collects environmental data** via several sensors (temperature, pressure, light, gas)
- **Controls electrical devices** through integrated relays and a dimmer
- **Manages access** with an RFID system for security
- **Offers a local interface** via a TFT touch screen
- **Communicates with the cloud** via Wi-Fi connection

### 2. Firebase Database

The database system complements the physical device by:
- **Storing sensor data** in real-time
- **Allowing remote control** of connected devices
- **Visualizing trends** through historical graphs
- **Managing automations** based on time or conditions
- **Securing access** through user authentication

## Main Technical Features

<br>
<div align="center">
  <figure>
    <img src="docs/carte_domotique.png" width="500" style="max-width: 100%;">
    <p><em>Schematic of the home automation board</em></p>
  </figure>
</div>
<br>

- **Sensors**: BMP280 (temperature/pressure), BH1750 (light), MQ9 (gas)
- **Actuators**: 2 double relays, 1 power dimmer, 1 relay for RFID access control
- **Local Interface**: TFT touch screen
- **Connectivity**: Wi-Fi for database connection
- **Data Platform**: Firebase (authentication, real-time database)

## Practical Application

Bosconnect can be used for various home automation applications:
- Indoor air quality monitoring
- Intelligent lighting control (on/off and dimming)
- Automation of electrical devices based on time schedules
- Access security with RFID technology
- Creation of conditional scenarios (if temperature > X then action Y)

### Security Note

For security reasons, some configuration parts have been removed from this repository, including Firebase access information and other sensitive authentication data. These details are necessary for the full operation of the system but are not shared publicly.

---

© 2024 BOSCONNECT. Developed by Bosco.