#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
LiquidCrystal_I2C lcd(0x27,16,2);

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

char key_code[4];
char password[4]={'3','8','7','6'};

int lock = A0;
const int buzzp = A1;
const int buzzn = A2;

unsigned int k=0;
byte rowPins[ROWS] = {2,3,5,4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6,7,8}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
int i=0;
int card1[4]{173,250,173,137};
int card2[4]{163,151,151,6};

constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
byte nuidPICC[2];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(lock, OUTPUT);
  pinMode(buzzp, OUTPUT);
  pinMode(buzzn, OUTPUT);
  pinMode(A3, OUTPUT);
  digitalWrite(lock, HIGH); 
  
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
 
  lcd.init();                      
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("  RFID & KEYPAD ");
  lcd.setCursor(0,1);
  lcd.print("  Lock Project  "); 
  lcd.clear();   
  for (byte i = 0; i < 6; i++) {key.keyByte[i] = 0xFF;}
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));  
  lcd.setCursor(0, 0);
  lcd.print("Scan Card or");
  lcd.setCursor(0, 1);
  lcd.print("Enter 4-Bit PIN");
}

void loop() {
   
  char key = keypad.getKey();
  
    if(key != NO_KEY){
    //Serial.println(key);
    //Serial.println("pressed:");    
    //Serial.print(i);    
     key_code[i++]=key;
     k=i;
    delay(200);
    digitalWrite(lock, HIGH);
     }
     
 if(k==4){
   //if(key_code[0]=='3'&&key_code[1]=='8'&&key_code[2]=='7'&&key_code[3]=='6'){
   if(!strncmp(password,key_code,4)){
   lcd.clear() ;lcd.setCursor(0, 0);lcd.print("Access Granted");
   digitalWrite(buzzp, HIGH);digitalWrite(buzzn, LOW);
   delay(100);
   digitalWrite(buzzp, LOW);digitalWrite(buzzn, LOW);
   digitalWrite(lock, LOW);digitalWrite(A3, HIGH);
   delay(5000);
   digitalWrite(lock, HIGH);    digitalWrite(A3, LOW);
    i=k=0;  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Card or");
  lcd.setCursor(0, 1);
  lcd.print("Enter 4-Bit PIN");
   }

   else {
   lcd.clear(); lcd.setCursor(0, 0);lcd.print("Access denied");
    digitalWrite(buzzp, HIGH);digitalWrite(buzzn, LOW);
    delay(1000);
    digitalWrite(buzzp, LOW);digitalWrite(buzzn, LOW);
    delay(500);
  //digitalWrite(lock, LOW);
  delay(1000);
  i=k=0;
 // digitalWrite(lock, HIGH); 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Card or");
  lcd.setCursor(0, 1);
  lcd.print("Enter 4-Bit PIN");
    }
 }
  
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

   // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
    Serial.println(nuidPICC[i]);
    }

  if(card1[0] == nuidPICC[0] &&card1[1] == nuidPICC[1] && card1[2] == nuidPICC[2] && card1[3] == nuidPICC[3])
 {
 // digitalWrite(buzzp, HIGH);digitalWrite(buzzn, LOW);delay(100);digitalWrite(buzzp, LOW);digitalWrite(buzzn, LOW);
  lcd.clear(); lcd.setCursor(0, 0);lcd.print("Access Granted");
  digitalWrite(buzzp, HIGH);digitalWrite(buzzn, LOW);
  delay(100);
  digitalWrite(buzzp, LOW);digitalWrite(buzzn, LOW);
  delay(500);
  digitalWrite(lock, LOW);digitalWrite(A3, HIGH);
  delay(5000);
  digitalWrite(lock, HIGH);digitalWrite(A3, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Card or");
  lcd.setCursor(0, 1);
  lcd.print("Enter 4-Bit PIN");   
 }

 else if(card2[0] == nuidPICC[0] &&card2[1] == nuidPICC[1] && card2[2] == nuidPICC[2] && card2[3] == nuidPICC[3])
 {
  //digitalWrite(buzzp, HIGH);digitalWrite(buzzn, LOW);delay(100);digitalWrite(buzzp, LOW);digitalWrite(buzzn, LOW);
  lcd.clear(); lcd.setCursor(0, 0);lcd.print("Access Granted");
  digitalWrite(buzzp, HIGH);digitalWrite(buzzn, LOW);
  delay(100);
  digitalWrite(buzzp, LOW);digitalWrite(buzzn, LOW);
  delay(500);
  digitalWrite(lock, LOW);digitalWrite(A3, HIGH);
  delay(5000);
  digitalWrite(lock, HIGH);digitalWrite(A3, LOW); 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Card or");
  lcd.setCursor(0, 1);
  lcd.print("Enter 4-Bit PIN");   
    
 }
  else
  {
    lcd.clear(); lcd.setCursor(0, 0);lcd.print("Access denied");
    digitalWrite(buzzp, HIGH);digitalWrite(buzzn, LOW);
    delay(1000);
    digitalWrite(buzzp, LOW);digitalWrite(buzzn, LOW);
    delay(500);
  //digitalWrite(lock, LOW);
  delay(1000);
 // digitalWrite(lock, HIGH); 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Card or");
  lcd.setCursor(0, 1);
  lcd.print("Enter 4-Bit PIN");
    }
}