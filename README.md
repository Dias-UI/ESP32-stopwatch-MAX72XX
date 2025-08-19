# ESP32 MAX72XX Stopwatch

This project is a digital stopwatch implemented on an ESP32 microcontroller that displays the elapsed time on a series of four 8x8 LED matrix displays.

## Features

- **Time Format:** Displays elapsed time from `00.00` to `99.99` seconds.
- **Display:** Utilizes four daisy-chained 8x8 LED matrix modules controlled by the MAX72XX driver.
- **Control:** A single push-button provides the following operations:
    - **1st Press:** Start
    - **2nd Press:** Pause
    - **3rd Press:** Reset
- **Custom Digits:** The number patterns are defined by the 8x8 patterns and can be customized and edited.

## Hardware Requirements

- **Microcontroller:** ESP32 Development Board
- **Display:** 4 x MAX7219 8x8 Dot Matrix LED Display Modules (daisy-chained)
- **Button:** 1 x Push Button
- **Wiring:** Jumper wires

Instead of a traditional push button, you can create a simple pressure-sensitive switch using:
- Two conductive pads (e.g., aluminum tape)
- A non-conductive spacer (e.g., layers of acrylic tape) between them
- Wiring each pad to the ESP32 (one to GND, the other to the button pin)
When pressed, the pads touch, completing the circuit just like a button press. This works well for foot-activated controls or large touch surfaces.

### Pin Configuration

| ESP32 Pin | Component      | Purpose          |
|-----------|----------------|------------------|
| GPIO 5    | MAX72XX CLK    | Clock Pin        |
| GPIO 16   | MAX72XX DATA   | Data In Pin      |
| GPIO 17   | MAX72XX CS     | Chip Select Pin  |
| GPIO 33   | Push Button    | Start/Stop/Reset |

## Software & Dependencies

This project is built using [PlatformIO](https://platformio.org/) with the Arduino framework.

- **Framework:** `arduino`
- **Libraries:**
    - `majicdesigns/MD_MAX72XX @ ^3.3.1`

## Setup & Installation

1. **Create a new PlatformIO project**  
   Open VS Code with the PlatformIO extension installed and create a new project for ESP32.

2. **Add project files**  
   Replace the default files with the provided ones:
   - Paste the contents into `main.cpp`
   - Paste the contents into `platformio.ini`

3. **Build and Upload**  
   Save the files (this will trigger dependency installation), then:
   ```bash
   pio run -t upload

## How It Works

The application cycles through three states: `STOPPED`, `RUNNING`, and `PAUSED`.

1.  **STOPPED:** The initial state. The display shows `00.00`. A button press transitions the state to `RUNNING` and records the start time.
2.  **RUNNING:** The elapsed time is continuously calculated and updated on the LED matrix display every 10 milliseconds. A button press moves the state to `PAUSED`.
3.  **PAUSED:** The stopwatch time freezes. The display holds the time at which it was paused. A button press resets the stopwatch, clears the display, and returns to the `STOPPED` state, ready for a new timing session.

The custom digit patterns are defined in `src/main.cpp` to render numbers on the 8x8 displays.
