# Bill of Materials (BoM) — Electrostatic Materials Separator

| Subsystem | Component | Purpose / Description |
| :--- | :--- | :--- |
| **Microcontrollers** | Arduino Mega 2560 Pro | Central controller; manages TFT display, Variac voltage sampling, and actuator logic |
| **Microcontrollers** | Arduino Pro Mini | Dedicated sensor node; processes Hall sensor pulses for RPM and sends over SPI/UART |
| **User Interface** | ILI9488 TFT Screen | Graphical display showing live RPM, voltage (kV), feeder speed, and vibration % |
| **High Voltage** | Variac & Transformer | Generates adjustable electrostatic field for metallic/non-metallic separation |
| **Power Supply** | XL4015 DC-DC Buck | Steps down input supply to provide a stable 5V rail for microcontrollers |
| **Power Switching** | Industrial Contactors | High-power AC switching for main system power and motor isolation |
| **Signal Conditioning** | TIP31C & Op-Amps | Amplifies and conditions high-voltage transformer feedback for ADC monitoring |
| **Electromechanical** | Hall Effect Sensor | Measures drum revolutions per minute via mounted neodymium magnets |
| **Electromechanical** | ESC & Drum Motor | Regulates rotating drum speed using microcontroller PWM control |
| **Electromechanical** | 2x Tray Vibrators | Controls steady material flow across the feeder tray and hopper (0–100% intensity) |
