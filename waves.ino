/*
 * This file is part of the BadCard project, originally developed by Iván (VoidNoi).
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

#define display M5Cardputer.Display
#define keyboard M5Cardputer.Keyboard

#define availableOptions 2

int letterHeight = 16.6;
int letterWidth = 12;

int cursorPosX, cursorPosY, screenPosX, screenPosY = 0;

int menuOption = 0;

//const int bgColor = 0xFC80;

IRrecv irReceiver(1);
decode_results results;

void setup() {
  // put your setup code here, to run once:
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

  Serial.begin(115200);

  irReceiver.enableIRIn();

  display.setRotation(1);
  display.setTextColor(BLACK);

  bootLogo();

}

void loop() {
 // M5Cardputer.update();
  
}

void bootLogo(){
  display.fillScreen(ORANGE);

  display.setTextSize(2.2);
  String APPversion = "Waves v0.0.1";
  display.setCursor(display.width()/2-(APPversion.length()/2)*letterWidth, display.height()/2 - 15);
  display.println(APPversion);

  display.setTextSize(2);
  display.setCursor(display.width()/2 - 95, display.height()/2 + 40);
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
  display.println("2 - IR (receive)");
  display.setCursor(20, 80);
  display.println("3 - About");
  display.setCursor(20, 100);
  display.println("4 - Exit");

  while(true) {
    M5Cardputer.update();
    if (keyboard.isChange() && menuOption == 0) {
      delay(100);
      if(keyboard.isKeyPressed('1')){
        menuOption = 1;
        RFfn();
        break;
      }
      if(keyboard.isKeyPressed('2')){
        menuOption = 2;
        irReceive();
        break;
      }
      if(keyboard.isKeyPressed('3')){
        menuOption = 3;
        about();
        break;
      }
    }
  }
  
  

}

void RFfn(){
  display.setTextSize(2);
  display.fillScreen(ORANGE);
  display.setCursor(20, 20);
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
  display.setCursor(20, 20);
  display.println("About");
  display.setCursor(20, 40);
  display.println("Waves v0.0.1");
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

void irReceive() {
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
      display.fillRect(20, 40, 120, 30, ORANGE); // Limpa a área onde o valor é exibido
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
