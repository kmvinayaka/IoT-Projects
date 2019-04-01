#include <ESP8266WiFi.h>
//#include <String.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Servo.h>
#include <Wire.h>

#include "LiquidCrystal_I2C.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position


#define LED 5
#define buzzer 12

/////////////////////////////////////////

#define WLAN_SSID       "myTectra"              //WIFI NAME
#define WLAN_PASS       "mytectra@1"              //WIFI PASSWORD

////////////////////////////////////////

#define AIO_SERVER      "io.adafruit.com"             //SERVER NAME
#define AIO_SERVERPORT  1883                          //MQTT PORT

/////////////////////////////////////////////

#define IO_USERNAME  "vinayakaswamy123"
#define IO_KEY       "ef4cfdd521d542f5b580ad2138b4a239"
/////////////////////////////////////////


WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);

////////////////////////////////////
Adafruit_MQTT_Subscribe Switch = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/Switch");
Adafruit_MQTT_Publish Alert = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/Alert");


void MQTT_connect();
void setup()
{
  myservo.attach(14);  // attaches the servo on pin 9 to the servo object
  Wire.begin(2, 0);
  lcd.begin();

  lcd.print("Smart Medicine Dispenser"); // Start Printing
  Serial.begin(9600);
  delay(10);
  pinMode(LED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(buzzer,1);
    delay(500);
    Serial.print(".");
  }
   digitalWrite(buzzer,0);
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  mqtt.subscribe(&Switch);

}
void rotate()
{
  for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}
void loop()
{
  if (!mqtt.connected()) {
    MQTT_connect();
  }
  lcd.setCursor(0, 0);
  lcd.scrollDisplayLeft();//builtin command to scroll left the text
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(300))) {
    if (subscription == &Switch) {
      Serial.print(F("On-Off button: "));
      Serial.println((char *)Switch.lastread);

      if (strcmp((char *)Switch.lastread, "ON") == 0) {
        digitalWrite(LED, 1);
        digitalWrite(buzzer, 1);
        Alert.publish(0);
        lcd.setCursor(0, 1);
        lcd.print("Alert!          ");
        //lcd.println("                ");

      }
      if (strcmp((char *)Switch.lastread, "OFF") == 0) {
        digitalWrite(LED, 0);
        digitalWrite(buzzer, 0);
        
        Alert.publish(1);
        rotate();
        lcd.setCursor(0, 1);
        //lcd.print("Collect Medicine ");
        lcd.print("                 ");
      }
    }

  }
}



void MQTT_connect() {
  Serial.println("in mqttconnect");
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    digitalWrite(buzzer,1);
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  digitalWrite(buzzer,0);
  Serial.println("MQTT Connected!");
}
