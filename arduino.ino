/*
  Proyek untuk: Smart Storage System for Enhanced Protection
  Kode untuk: Arduino UNO
  Ringkasan fungsi kode:
  - Send & receive data to/from ESP32
  - Main controller of the storage system
  Komponen yang digunakan:
  - Keypad 3x4
  - LCD I2C
  - MFRC522 module (RFID)
  - Solenoid lock 12V
  - Battery Holder for 12V
  - Relay 5V
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>
#define RST_PIN 9
#define SS_PIN 10
#define countof(a) (sizeof(a) / sizeof(a[0]))

const int RELAY_PIN = A3;
int state=0;
String accessCode="1234"; //GANTI PIN DISINI
int codeLength = accessCode.length();
String dataFromESP;


//NFC
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Membuat MFRC522 instance

//LCD
LiquidCrystal_I2C  lcd(0x27,16,2);

//KEYPAD
const byte numRows= 4;  //Jumlah baris keypad
const byte numCols= 3;  //Jumlah kolom keypad
unsigned long lastKeyPressTime = 0;  // variabel untuk menyimpan waktu terakhir kali tombol keypad ditekan
const unsigned long TIMEOUT_PERIOD = 5000;  // Batas waktu timeout (5 detik)
String codeInput; //Variabel untuk menyimpan PIN yang diinput user
char keymap[numRows][numCols]=
{
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

//Kode untuk menunjukkan koneksi keypad ke Arduino
byte rowPins[numRows] = {2,3,4,5};    
byte colPins[numCols] = {6,7,8};

//Membuat Keypad Class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

void setup() 
{
  pinMode(RELAY_PIN, OUTPUT);
  Serial.begin(9600);  // Memulai komunikasi serial dengan ESP32
  while (!Serial);     
  SPI.begin();         // Insiisalisasi SPI Bus
  mfrc522.PCD_Init();  // Inisialisasi MFRC522
  // Inisialisasi LCD I2C
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.clear();
  lcd.print( "LOCKED" );
  lcd.setCursor(0,1);
  lcd.print("Use pin or tag");
  }



int compareCODE(String a)  //PIN yang dimasukkan user akan dibandingkan dengan PIN sebenarnya di Fungsi ini
{  //with the accessCode;
  if(a.equals(accessCode))
    return 1;
  else if(a.equals("interrupt")) 
    return 2;
  else 
    return 0;
}

void unlockedPrompt() //Instruksi ke LCD ketika solenoid terbuka
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print( "UNLOCKED" );
  lcd.setCursor(0,1);
  lcd.print("Press # to lock");
}

void lockedPrompt() //Instruksi ke LCD ketika solenoid tertutup
{
  lcd.setCursor(0,0);
  lcd.clear();
  lcd.print( "LOCKED" );
  lcd.setCursor(0,1);
  lcd.print("Use pin or tag");
}

void loop() 
{
  switch(state)
  {// state merepresentasikan kondisi lock, 0 = terkunci, 1 = terbuka
    case 0: {
      // Kode untuk pengecekan tag RFID
      mfrc522.PCD_Init();
      if ( mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() )
      {
        String content = "";
        byte letter;
        for (byte i = 0; i < mfrc522.uid.size; i++)
        {
          content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
          content.concat(String(mfrc522.uid.uidByte[i], HEX));
        }
        content.toUpperCase();
        if(content.substring(1) == "21 E7 E6 26")  //UBAH RFID DISINI
          { 
            state=1;
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print( "VALID NFC CODE" );
            Serial.print('c'); //Mengirim tanda bahwa berhasil terbuka ke ESP32
            delay(1000);
            unlockedPrompt();
            digitalWrite(RELAY_PIN, HIGH);
            return;
          }
        else
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print( "INVALID NFC CODE" );
          delay(1000);
          Serial.print('i'); //Mengirim tanda bahwa tidak berhasil terbuka ke ESP32
          lockedPrompt();
          return;
        }
     // Kode ketika diterima data dari ESP32, artinya user sedang mencoba membuka storage system melalui Webserver
    }else if (Serial.available()) {
    dataFromESP = Serial.readString();
    //Cek kode untuk ESP32 supaya terbayang string "1", "2", dan "3" didapat darimana.
    if(dataFromESP == "1") {
      state=1;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print( "VALID CODE" );
      delay(1000);
      unlockedPrompt();
      digitalWrite(RELAY_PIN, HIGH);
      return;
    } else if(dataFromESP == "2"){
      state=0;
      lockedPrompt();
      digitalWrite(RELAY_PIN, LOW);
      return;
    } else if(dataFromESP == "3"){ //3 ARTINYA GAGAL MASUK
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print( "INVALID PIN" );
      delay(1000);
      lockedPrompt();
      return;
    }
    }
      // Kode untuk keypad
      int A = 0;
      char key=myKeypad.getKey();
      if(key)
      {
        lastKeyPressTime = millis();
        codeInput += key;
        if (codeInput.length() == 1)
        {
          lcd.clear();
          lcd.setCursor(0,0);
        }
        lcd.print('X');
        if(codeInput.length() == codeLength)
        {
          A = compareCODE(codeInput);
        }
        else
        {
          return;
        }
        if(A==0)
        {  //A adalah variabel yang menyimpan kondisi (0=code tidak valid, 1=code valid)
          lcd.clear();
          lcd.print("INVALID CODE");
          delay(1000);
          Serial.print('i');
          lockedPrompt();
          codeInput = "";
          return;
        }
        if(A==1)
        {
          lcd.setCursor(0,0);
          lcd.clear();
          lcd.print( "VALID CODE " );
          delay(1000);
          Serial.print('c');
          state = 1;
          unlockedPrompt();
          digitalWrite(RELAY_PIN, HIGH);
          codeInput = "";
          return;
        }
        if(A == 2); 
        {
          state=0;
          codeInput = "";
          return;
        }
      }
      unsigned long currentTime = millis();
      if (currentTime - lastKeyPressTime > TIMEOUT_PERIOD && codeInput.length() != 0) 
      {
        // Waktu timeout tercapai sehingga input pin direset
        codeInput = "";
        lockedPrompt();
      }
      break;
    }

    case 1:
    {// Lock akan tertutup kembali bila tanda pagar ditekan
      char c=myKeypad.getKey();
      if(c == '#')
      {
        state=0;
        Serial.print('r');
        lockedPrompt();
        digitalWrite(RELAY_PIN, LOW);
        return;
     // Lock juga akan tertutup semisalkan ada perintah reset diterima dari Webserver
      }else if (Serial.available()) {
    dataFromESP = Serial.readString();

    // Sending data from Arduino to ESP32
    if(dataFromESP == "2"){
      state=0;
      lockedPrompt();
      digitalWrite(RELAY_PIN, LOW);
      return;
    } 
      break;
  }
  }
}
}
