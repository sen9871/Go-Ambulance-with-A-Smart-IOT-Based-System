#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <ESP8266WiFi.h>

LiquidCrystal_I2C lcd(0x27,20,4);
SoftwareSerial Gsm(D7,D8);
TinyGPS gps;
WiFiClient client;

const int channelID = 1735582;
String apiKey  = "YQKJ0HB5W2QIQH9H";
const char* ssid = "打你屁屁";
const char* password = "ymslmc520";
const char* server = "api.thingspeak.com";
const int postingInterval = 2*1000;


int buzzer = D5;
int vs = D10;
int buttonPin = D2;
int buttonState = 0;


void setup()
{
  lcd.init();
  lcd.backlight();
  
  pinMode(buzzer, OUTPUT);
  pinMode(vs, INPUT); 
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  Gsm.begin(9600);
  Wire.begin();
  delay(10);
  WiFi.begin(ssid,password);
  
  lcd.setCursor(0,0);
  lcd.print("Car Engine");
  lcd.setCursor(0,1);
  lcd.print("Started");
  delay(1500);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Sensor Standby");
  delay(2000);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("Error connected to WiFi");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}


void loop()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Driver Tracking");

  if (client.connect(server,80))
  {
    
  if (Gsm.available())
  Serial.write(Gsm.read());

  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
 
  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (Serial.available())
    {
      char c = Serial.read();
      Serial.print(c);
      if (gps.encode(c)) 
        newData = true;  
    }
  }
  

  buttonState = digitalRead(buttonPin);
  
  long measurement = vibration();
  delay(50);
  Serial.println("Impact: ");
  Serial.println(measurement);

  String postStr = apiKey;
  postStr += "&field1=";
  postStr += measurement;
  postStr += "&field2=";
  postStr += String(buttonState);

    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: ESP8266 (nothans)/1.0");
    client.println("Connection: close");
    client.println("X-THINGSPEAKAPIKEY: " + apiKey);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(postStr.length()));
    client.println("");
    client.print(postStr);
  
  if (measurement > 1500)
  {   
    digitalWrite(buzzer, HIGH);
    Serial.println("Impact Detected");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Magnitude:");
    lcd.setCursor(0,1);
    lcd.print(measurement);
    lcd.print(" N");
    delay(2000);
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Impact");
    lcd.setCursor(2,1);
    lcd.print("Detected");
    delay(2000);
    
    SendMessage();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sending SMS");
    delay(1500);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Successful");
    delay(1500);

    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("Waiting");
    lcd.setCursor(3,1);
    lcd.print("Ambulance");
    delay(3000);
  }
  
  else
  {
    digitalWrite(buzzer, LOW);
    lcd.clear();
  }

    
    delay(1000);


  buttonState = digitalRead(buttonPin);
  
  if(buttonState == HIGH)
  {
    digitalWrite(buzzer, HIGH);
    Serial.println("Button is pressed");
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Emergency");
    lcd.setCursor(4,1);
    lcd.print("Calling");
    delay(2000);
    
    SendMessage();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sending SMS");
    delay(1500);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Successful");
    delay(1500);
    
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("Waiting");
    lcd.setCursor(3,1);
    lcd.print("Ambulance");

  }
  else
  {
    digitalWrite(buzzer, LOW);
    lcd.clear();
  }
  
}
  client.stop();

  delay(postingInterval);
}


long vibration()
{
  long measurement=pulseIn (vs, HIGH);
  return measurement;
}


void SendMessage()
{
  
  long lat, lon;
  float flat, flon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  
  
  Serial.println ("Sending Message");
  Gsm.println("AT+CMGF=1"); 
  delay(1000);
  Serial.println ("Set SMS Number");
  Gsm.println("AT+CMGS=\"+601112609750\"\r"); 
  delay(1000);
  Serial.println ("Set SMS Content");
  Gsm.println("Alert. I had a car accident.");
  Gsm.println("Please help me....");
  Gsm.println("Name: Felix Teng Hong Eng");
  Gsm.println("IC No: 701003-13-7779");
  Gsm.println("My current location is: ");

  Gsm.print("http://maps.google.com/maps?q=loc:");
  
  Gsm.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
  Gsm.print(","); 
  Gsm.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
 
  delay(500);
  Serial.println ("Finish");
  Gsm.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  Serial.println ("Message has been sent ->SMS Selesai dikirim");

}
