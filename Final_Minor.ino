// ESP 32 RFID and Keypad Door Lock and Voice-Controlled Home Automation
#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <HTTPClient.h>
#include "RMaker.h"
#include "WiFiProv.h"

const char *service_name = "ESP_32_PROV";
const char *pop = "1234567";

// define the Device Names
char deviceName_1[] = "Lamp";
char deviceName_2[] = "DoorLock";

// define the GPIO connected with Relays and switches
static uint8_t RelayPin1 = 4;  //D4
static uint8_t RelayPin2 = 2;  //D2


static uint8_t wifiLed    = 34;   //D34
static uint8_t gpio_reset = 0;

/* Variable for reading pin status*/
// Relay State
bool toggleState_1 = LOW; //Define integer to remember the toggle state for relay 1
bool toggleState_2 = LOW; //Define integer to remember the toggle state for relay 2

// Switch State
bool SwitchState_1 = LOW;
bool SwitchState_2 = LOW;

//The framework provides some standard device types like switch, lightbulb, fan, temperature sensor.
static Switch my_Lamp(deviceName_1, &RelayPin1);
static Switch my_DoorLock(deviceName_2, &RelayPin2);

// Keypad Configurations
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {13, 12, 14, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 25, 33, 32}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// OLED Configurations
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pinouts
#define RST_PIN         36        // Configurable, see typical pin layout above
#define SS_PIN          5         // Configurable, see typical pin layout above
#define BUZZER          15

int Secret_Code = 9628; // Code used to unlock the door via keypad

// Necessary Variables
String ID_e;
int val = 0;
bool done_flag = 0;
char* response = " ";
String res = "";
char* succ_code = "200 OK";
bool rfid_flag = 1;

MFRC522 mfrc522(SS_PIN, RST_PIN); // Setting up RFID Pins

const char* NAME; // Variable to save the name of the person

void sysProvEvent(arduino_event_t *sys_event)
{
    switch (sys_event->event_id) {      
        case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
        Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
        printQR(service_name, pop, "ble");
#else
        Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
        printQR(service_name, pop, "softap");
#endif        
        break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.printf("\nConnected to Wi-Fi!\n");
        digitalWrite(wifiLed, true);
        break;
    }
}

void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx)
{
    const char *device_name = device->getDeviceName();
    const char *param_name = param->getParamName();

    if(strcmp(device_name, deviceName_1) == 0) 
    {
      
      Serial.printf("Lightbulb = %s\n", val.val.b? "true" : "false");
      
      if(strcmp(param_name, "Power") == 0) 
      {
        Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
        toggleState_1 = val.val.b;
        (toggleState_1 == false) ? digitalWrite(RelayPin1, HIGH) : digitalWrite(RelayPin1, LOW);
        param->updateAndReport(val);
      }
      
    } else if(strcmp(device_name, deviceName_2) == 0) 
    {
      Serial.printf("Switch value = %s\n", val.val.b? "true" : "false");

      if(strcmp(param_name, "Power") == 0) 
      {
        Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
        toggleState_2 = val.val.b;
        (toggleState_2 == false) ? digitalWrite(RelayPin2, LOW) : digitalWrite(RelayPin2, HIGH);
        param->updateAndReport(val);
      }
  
    }
}

void setup()
{
  uint32_t chipId = 0;
  Serial.begin(115200);
  // Set the Relays GPIOs as output mode
    pinMode(RelayPin1, OUTPUT);
    pinMode(RelayPin2, OUTPUT); 
    pinMode(wifiLed, OUTPUT);
    
    // Configure the input GPIOs
    pinMode(gpio_reset, INPUT);
    
    // Write to the GPIOs the default state on booting
    digitalWrite(RelayPin1, !toggleState_1);
    digitalWrite(RelayPin2, !toggleState_2);
    digitalWrite(wifiLed, LOW);

    Node my_node;    
    my_node = RMaker.initNode("ESP32_Relay_2");

    //Standard switch device
    my_Lamp.addCb(write_callback);
    my_DoorLock.addCb(write_callback);

    //Add switch device to the node   
    my_node.addDevice(my_Lamp);
    my_node.addDevice(my_DoorLock);

    //This is optional 
    RMaker.enableOTA(OTA_USING_PARAMS);
    //If you want to enable scheduling, set time zone for your region using setTimeZone(). 
    //The list of available values are provided here https://rainmaker.espressif.com/docs/time-service.html
    // RMaker.setTimeZone("Asia/Shanghai");
    // Alternatively, enable the Timezone service and let the phone apps set the appropriate timezone
    RMaker.enableTZService();
    RMaker.enableSchedule();

    //Service Name
    for(int i=0; i<17; i=i+8) {
      chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    Serial.printf("\nChip ID:  %d Service Name: %s\n", chipId, service_name);

    Serial.printf("\nStarting ESP-RainMaker\n");
    RMaker.start();

    WiFi.onEvent(sysProvEvent);
#if CONFIG_IDF_TARGET_ESP32
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif

    my_Lamp.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);
    my_DoorLock.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  // Checking OLED Display connections
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("RFID Door Lock");
  display.display();

  SPI.begin();
  mfrc522.PCD_Init();

  display.clearDisplay();
  Serial.println("Scan tag");
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15, 30);
  display.print("Scan tag");
  display.display();
}

void loop()
{
  if(digitalRead(gpio_reset) == LOW) 
  { //Push button pressed
      Serial.printf("Reset Button Pressed!\n");
      // Key debounce handling
      delay(100);
      int startTime = millis();
      while(digitalRead(gpio_reset) == LOW) delay(50);
      int endTime = millis();

      if ((endTime - startTime) > 10000) 
      {
        // If key pressed for more than 10secs, reset all
        Serial.printf("Reset to factory.\n");
        RMakerFactoryReset(2);
      } 
      else if ((endTime - startTime) > 3000) 
      {
        Serial.printf("Reset Wi-Fi.\n");
        // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
        RMakerWiFiReset(2);
      }
    }
  delay(100);

  if (WiFi.status() != WL_CONNECTED)
  {
    //Serial.println("WiFi Not Connected");
    digitalWrite(wifiLed, false);
  }
  else
  {
    //Serial.println("WiFi Connected");
    digitalWrite(wifiLed, true);
  }

  char key = keypad.getKey(); // Store the key pressed on the Keypad
  if (key) {
    Serial.println(key);
    beep(100);
  }

  if (key == 'A') // If 'A' is pressed, go to pincode mode
  {
    Serial.println("A");
    Serial.println("Pin Code");
    display.clearDisplay();
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0, 0);            // Start at top-left corner
    display.println(("Pin Code"));
    display.setCursor(0, 30);
    display.println(("Mode"));
    display.display();
    beep(100);
    beep(100);
    delay(500);

    Serial.println("Enter the \nFour Digit \nCode");

    rfid_flag = 0;
  }

  if (key == 'C') // If 'C' is pressed, manually reset the board 
  {
   ESP.restart();
  }

  // RFID Mode
  if (rfid_flag == 1)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(15, 30);
    display.print("Scan Tag");
    display.display();

    Serial.println("Waiting for the tag1");

    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
      return;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
    {
      return;
    }

    Serial.print("UID tag :");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      //Serial.println(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.println(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    content.toUpperCase();
    Serial.println("Waiting for the tag2");

    if (content.substring(1) == "A3 97 97 06") //change here the UID of the card that you want to give access
    {
      Serial.print("Authorized Access");
      Serial.println(content.substring(1));
      NAME = "Himanshu";
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(1, 1);
      display.println("Welcome ");
      display.println(NAME);
      display.setCursor(1, 35);
      display.print("Door Open");
      display.display();
      Door_Open();
      digitalWrite(BUZZER, LOW);
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(1, 1);
      display.print("Door");
      display.setCursor(1, 35);
      display.print("Closed");
      display.display();
      delay(500);

    }

    else if (content.substring(1) == "AD FA AD 89") //change here the UID of the card that you want to give access
    {
      Serial.print("Authorized Access");
      Serial.println(content.substring(1));
      NAME = "Indranil";
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(1, 1);
      display.println("Welcome ");
      display.println(NAME);
      display.setCursor(1, 35);
      display.print("Door Open");
      display.display();
      Door_Open();
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(1, 1);
      display.print("Door");
      display.setCursor(1, 35);
      display.print("Closed");
      display.display();
      delay(500);


    }
    else
    {
      Serial.println("Not Registered");
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 15);
      display.print("Access");
      display.setCursor(0,30 );
      display.print("Denied");
      display.display();
      beep(200);
      beep(200);
      beep(200);
      Serial.println("You can't enter room.");
    }
    content.substring(1) = "";
  }

  // Pincode Mode
  else
  {

    if (Keypad_Input() == Secret_Code) // Checking the code entered by user
    {
      Serial.println("Correct Code");
      display.clearDisplay();
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(15, 20);            // Start at top-left corner
      display.println("Correct");
      display.setCursor(32, 40);            // Start at top-left corner
      display.println("Code");
      display.display();
      Door_Open();

      rfid_flag = 1;
    }
    else
    {
      Serial.println("Wrong Code");
      display.clearDisplay();
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(0, 20);            // Start at top-left corner
      display.println(("Wrong Code"));
      display.display();
      beep(200);
      beep(200);
      delay(1000);
      display.clearDisplay();
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(35, 20);            // Start at top-left corner
      display.println(("Code"));
      display.display();
    }
    ID_e = "";
  }

}

// Just a normal beep sound
void beep(int duration)
{
  digitalWrite(BUZZER, HIGH);
  delay(duration);
  digitalWrite(BUZZER, LOW);
  delay(25);
}

// Reading and Storing the code entered by user
int Keypad_Input(void)
{
  int i = 0;
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.println(("Enter Code"));
  display.display();
  i = 0;
  ID_e = "";
  val = 0;
  while (1)
  {
    char key = keypad.getKey();

    if (key && i < 4)
    {
      ID_e = ID_e + key;
      val = ID_e.toInt();
      Serial.println(val);
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(i * 35, 50);            // Start at top-left corner
      display.println(("*"));
      display.display();
      beep(100);
      i++;
    }
    if (key == '#')
    {
      done_flag = 1;
      Serial.println("DONE");
      i = 0;
      ID_e = "";
      return (val);
    }
    if (key == 'B')
    {
      rfid_flag = 1;
      ID_e = "";
      return (0);
    }
  }
}

// Opens the Door, and close it after 5 sec
void Door_Open()
{
  digitalWrite(BUZZER, HIGH);
  digitalWrite(RelayPin2, HIGH);
  Serial.println("DOOR OPENED");
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(5000);
  digitalWrite(RelayPin2, LOW);
  Serial.println("DOOR CLOSED");
}