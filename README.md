# BW16-SmartDeauther
Deauth networks using BW16 (RTL8720DN). Deauth+Dissoc+CTS <br>
More smart deauth attck then normal

# Hardware Requirements
- Ai-Thinker BW16 RTL8720DN Development Board

# Setup
1. Download Arduino IDE from [here](https://www.arduino.cc/en/software) according to your Operating System.
2. Install it.
3. Go to `File` → `Preferences` → `Additional Boards Manager URLs`.
4. Paste the following link :
   
   ```
   https://github.com/ambiot/ambd_arduino/raw/master/Arduino_package/package_realtek_amebad_index.json
   ```
5. Click on `OK`.
6. Go to `Tools` → `Board` → `Board Manager`.
7. Search `Realtek Ameba Boards (32-Bits ARM Cortex-M33@200MHz)` by `Realtek`. <br>
!!! CODE WORK WITH 3.1.7 pack, other versions may not be work !!!
9. Install it.
10. Restart the Arduino IDE.
11. Done!

# Install
1. Download or Clone the Repository.
2. Open the folder and open `SmartDeauth.ino` in Arduino IDE.
3. Select board from the `Tools` → `Board` → `AmebaD ARM (32-bits) Boards`.
   - It is `Ai-Thinker BW16 (RTL8720DN)`.
4. Select the port of that board.
5. Go to `Tools` → `Board` → `Auto Flash Mode` and select `Enable`.
6. Upload the code.
   - Open Serial Monitor (115200) and write "scan", than write "attck [num]" ot "mattck [num]"

# Note
Tested only on provider none smart routers <br>
Code for educational purposes, good luck
