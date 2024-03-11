/*
  Serial Event example

  When new serial data arrives, this sketch adds it to a String.
  When a newline is received, the loop prints the string and clears it.

  A good test for this is to try it with a GPS receiver that sends out
  NMEA 0183 sentences.

  NOTE: The serialEvent() feature is not available on the Leonardo, Micro, or
  other ATmega32U4 based boards.

  created 9 May 2011
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/SerialEvent
*/
// Default pin configuration

// #include <MemoryUsage.h>

int pul_pin = 13;
int dir_pin = 12;
int ena_pin = 11;
int far_pin = 10;
int ner_pin = 9;

int i = 0;
int freq = 200;
unsigned long previous_micros = 0;  //will store last time LED was blinked
unsigned long current_micros = 0;
unsigned long on_micros = 0;
int period = 5000;         // period at which to blink in us
unsigned long pulse_cnt = 0;
unsigned long pulse_tot = 0;
int pulse_state = LOW;
bool ena_flag = false;
bool dir_flag = true; // Far

String inputString[3] = {"", "", ""};      // a String to hold incoming data
String Word;
bool stringComplete = false;  // whether the string is complete

void setup() {
  // initialize pin:
  pinMode(pul_pin, OUTPUT);
  pinMode(dir_pin, OUTPUT);
  pinMode(ena_pin, OUTPUT);
  pinMode(far_pin, INPUT);
  pinMode(ner_pin, INPUT);
  // initialize serial:
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  // inputString.reserve(200);
  digitalWrite(ena_pin, HIGH);
  digitalWrite(dir_pin, HIGH);
  digitalWrite(pul_pin, LOW);

  Serial.println("Initialization Complete");
}

void loop() {
  // print the string when a newline arrives:
  if (stringComplete) {
    bool if_correct = true;
    // Serial.println(inputString[0]);
    // Serial.println(inputString[1]);
    // Serial.println(inputString[2]);
    if (strcmp(inputString[0].c_str(), "SET")==0){
      if (strcmp(inputString[1].c_str(), "FREQ" )== 0){
        freq = atoi(inputString[2].c_str());
        if (freq > 600){
          Serial.println("*** Exceed maximum frequency(600)! ***");
          freq = 1000000 / period;
        }
        else{
        period = 1000000 / freq;
        Serial.println(String("Set Frequency: ")+String(freq)+String(" Hz"));
        }
      }
      else if (strcmp(inputString[1].c_str(), "DIRC" )== 0){
        if (strcmp(inputString[2].c_str(), "NER" )== 0) {digitalWrite(dir_pin, LOW); dir_flag = false; Serial.println("Set Direction: Near");}
        else if (strcmp(inputString[2].c_str(), "FAR" )== 0) {digitalWrite(dir_pin, HIGH); dir_flag = true; Serial.println("Set Direction: Far");}
        else if_correct = false;
      }
      else if_correct = false;
    }
    else if (strcmp(inputString[0].c_str(), "ON")==0){
      if ((digitalRead(far_pin) == LOW && !dir_flag) || (digitalRead(ner_pin) == LOW && dir_flag) || (digitalRead(far_pin) == HIGH && digitalRead(ner_pin) == HIGH)){
        if (strcmp(inputString[1].c_str(), "") == 0) {
          digitalWrite(ena_pin, LOW);
          pulse_tot = 999999999;
          previous_micros = 0;
          Serial.println("Motor ON");
        }
        else {
          digitalWrite(ena_pin, LOW);
          pulse_tot = atoi(inputString[1].c_str());
          Serial.println(String("Motor ON for ") + String(pulse_tot) + String(" steps"));
        }
        pulse_cnt = 0;
        previous_micros = 0;
        on_micros = micros();
        ena_flag = true;
      }
      delay(300);
    }
    else if (strcmp(inputString[0].c_str(), "OFF")==0){
      digitalWrite(ena_pin, HIGH);
      digitalWrite(pul_pin, LOW);

      Serial.println("Motor OFF");
      ena_flag = false;
    }
    else if (strcmp(inputString[0].c_str(), "ASK")==0){
      if (strcmp(inputString[1].c_str(), "FREQ") == 0) Serial.println(String(freq)+String(" Hz"));
      else if (strcmp(inputString[1].c_str(), "DIRC") == 0) Serial.println(dir_flag? String("Far") : String("Near"));
      else if_correct = false;
    }

    else if_correct = false;

    if (!if_correct) Serial.println("Incorrect Command");

    // clear the string:
    inputString[0] = "";
    inputString[1] = "";
    inputString[2] = "";
    stringComplete = false;
    i = 0;
  }
  
  // Stop the motor
  if (ena_flag && (pulse_cnt >= pulse_tot || digitalRead(ner_pin) == LOW && !dir_flag || digitalRead(far_pin) == LOW && dir_flag)){
    digitalWrite(ena_pin, HIGH);
    digitalWrite(pul_pin, LOW);
    ena_flag = false;
    if (digitalRead(ner_pin) == LOW) Serial.println("*** Reach Near End! ***");
    if (digitalRead(far_pin) == LOW) Serial.println("*** Reach Far End! ***");

    Serial.println("Motor Stopped");
  }

  // waveform generator
  if (ena_flag){
    current_micros = micros();
    if (current_micros - previous_micros >= period/2) { // check if half of the period passed
      previous_micros = current_micros;   // save the last time you blinked the LED
      if (current_micros - on_micros > 50000){ // Wait 50 ms after ENA is turned LOW
        if (pulse_state == LOW) { // if the LED is off turn it on and vice-versa
        pulse_state = HIGH;
        } else {
        pulse_state = LOW;
        pulse_cnt++;
      }
      }
      digitalWrite(pul_pin, pulse_state);//set LED with ledState to blink again
    }
  }

}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    if (inChar == ' '){
      i++;
      continue;
    }
    if (inChar == '\n') {
      stringComplete = true;
      continue;
    }
    inputString[i] += inChar; 
    // inputString = Serial.readString();
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:

  }
}
