#include <LiquidCrystal.h>


const int pirPin = 2;        
const int trigPin = 3;       
const int echoPin = 4;    
const int firePin = 5;       
const int redLedPin = 6;     
const int greenLedPin = 7;    
const int yellowLedPin = 8;  
const int buzzerPin = 9;      
const int gasPin = A0;      
const int tempPin = A1;      


LiquidCrystal lcd(12, 11, 10, A2, A3, A4);


const int GAS_LEAK_THRESHOLD =200 ;    
const float HIGH_TEMP_THRESHOLD = 48.0; 

const int REACHABLE_DISTANCE_CM = 500; 


unsigned long previousMillis = 0;
bool greenLedState = LOW;        

void setup() {
  
  Serial.begin(9600);
 
  lcd.begin(16, 2);

 
  pinMode(pirPin, INPUT);
  pinMode(firePin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  
  lcd.print("Safety System");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(1000);
}

void loop() {
  // --- Stage 1: READ ALL SENSOR DATA ---
  bool motionDetected = digitalRead(pirPin);
  bool fireOverride = digitalRead(firePin);
  int gasValue = analogRead(gasPin);
  int tempValue = analogRead(tempPin);
  float temperatureC = (tempValue * (5.0 / 1023.0) * 100.0) / 10.0;
  
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  // --- Stage 2: PRINT TO VIRTUAL TERMINAL ---
  Serial.println("---------- SENSOR STATUS UPDATE ----------");
  Serial.print("PIR Result: "); Serial.println(motionDetected ? "PERSON DETECTED" : "THERE IS NO-ONE");
  Serial.print("Fire: "); Serial.println(fireOverride ? "ON (EMERGENCY!)" : "OFF");
  Serial.print("Gas Sensor: "); Serial.println(gasValue);
  Serial.print("Temperature: "); Serial.print(temperatureC, 1); Serial.println(" C");
  Serial.print("Distance: "); Serial.print(distance); Serial.println(" cm");
  Serial.println("--------------------------------------\n");

  // --- Stage 3: RESET OUTPUTS ---
  digitalWrite(redLedPin, LOW);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(buzzerPin, LOW);
  lcd.clear();

  // --- Stage 4: LOGIC CORE (PRIORITY-BASED) ---
  if (fireOverride) {
    digitalWrite(redLedPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
    lcd.setCursor(0, 0); lcd.print("!!FIRE EMERGENCY");
    lcd.setCursor(0, 1); lcd.print("!!  EVACUATE  !!");
    digitalWrite(greenLedPin, LOW);
  }
  else if (temperatureC > HIGH_TEMP_THRESHOLD) {
    digitalWrite(redLedPin, HIGH);
    digitalWrite(buzzerPin, HIGH); delay(150); digitalWrite(buzzerPin, LOW);
    lcd.setCursor(0, 0); lcd.print("DANGER: HIGH TEMP");
    lcd.setCursor(0, 1); lcd.print("Temp: "); lcd.print(temperatureC, 1); lcd.print("C");
    digitalWrite(greenLedPin, LOW);
  }
  else if (gasValue > GAS_LEAK_THRESHOLD) {
    if (!motionDetected && distance > REACHABLE_DISTANCE_CM && distance > 0) {
      digitalWrite(redLedPin, HIGH);
      digitalWrite(buzzerPin, HIGH);
      lcd.setCursor(0, 0); lcd.print("!! HEY...!!!");
      lcd.setCursor(0, 1); lcd.print("TOXIC GAS, CHECK");
    } else {
      digitalWrite(yellowLedPin, HIGH);
      digitalWrite(buzzerPin, HIGH); delay(200); digitalWrite(buzzerPin, LOW);
      lcd.setCursor(0, 0); lcd.print("GAS LEAK WARNING");
      lcd.setCursor(0, 1); lcd.print("Go away from Toxic gas");
    }
    digitalWrite(greenLedPin, LOW);
  }
  else if (motionDetected) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 500) { // CORRECT: '{' is here
      previousMillis = currentMillis;
      greenLedState = !greenLedState;
      digitalWrite(greenLedPin, greenLedState);
    } // CORRECT: '}' is here
    lcd.setCursor(0, 0); lcd.print("STATUS: ALL SAFE"); // CORRECT: Moved inside this block
    lcd.setCursor(0, 1); lcd.print("Someone is there");
  }
  else {
    digitalWrite(greenLedPin, LOW);
    lcd.setCursor(0, 0); lcd.print("SYSTEM STANDBY");
    lcd.setCursor(0, 1); lcd.print("Area is Clear");
  }

  delay(500);
}