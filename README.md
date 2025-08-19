# ESP32 MAX72XX Stopwatch

This project is a digital stopwatch implemented on an ESP32 microcontroller that displays the elapsed time on a series of four 8x8 LED matrix displays, driven by the MAX72XX driver.

The stopwatch displays time in `SS.cs` format (Seconds.centiseconds) up to `99.99`. A single button controls the start, stop (pause), and reset functions.

## Features

- **Time Format:** Displays elapsed time from `00.00` to `99.99` seconds.
- **Display:** Utilizes four daisy-chained 8x8 LED matrix modules controlled by the MAX72XX driver.
- **Control:** A single push-button provides the following operations:
    - **1st Press:** Start
    - **2nd Press:** Pause
    - **3rd Press:** Reset
- **Leading Zero Blanking:** The tens-of-seconds digit is blank until the time reaches 10.00 seconds.

## Hardware Requirements

- **Microcontroller:** ESP32 Development Board
- **Display:** 4 x MAX7219 8x8 Dot Matrix LED Display Modules (daisy-chained)
- **Button:** 1 x Push Button
- **Wiring:** Jumper wires

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

1.  **Clone the repository:**
    ```bash
    git clone <your-repo-url>
    ```

2.  **Open in PlatformIO:**
    Open the project folder in VS Code with the PlatformIO extension installed.

3.  **Build and Upload:**
    PlatformIO will automatically fetch the required dependencies. Build the project and upload it to your ESP32 board.

## How It Works

The application cycles through three states: `STOPPED`, `RUNNING`, and `PAUSED`.

1.  **STOPPED:** The initial state. The display shows `00.00`. A button press transitions the state to `RUNNING` and records the start time.
2.  **RUNNING:** The elapsed time is continuously calculated and updated on the LED matrix display every 10 milliseconds. A button press moves the state to `PAUSED`.
3.  **PAUSED:** The stopwatch time freezes. The display holds the time at which it was paused. A button press resets the stopwatch, clears the display, and returns to the `STOPPED` state, ready for a new timing session.

The custom digit patterns are defined in `src/main.cpp` to render numbers on the 8x8 displays.
