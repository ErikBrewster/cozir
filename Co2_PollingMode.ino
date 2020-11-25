// https://www.airtest.com/support/datasheet/COZIRSerialInterface.pdf

//    Command             Description               Example         Notes

//    G\r\n               fresh air calibration     G\r\n           Assumes 450ppm in fresh air !
//    F ##### #####\r\n   exact calibration         F 410 400\r\n   Note the space after F and the first value
//    K #\r\n             set mode                  K 1\r\n         Mode 0 = Command Mode,  Mode 1 = Streaming Mode,  Mode 2 = Polling Mode. --- Note the space after K
//    Z\r\n               get Co2                   Z\r\n
//    T\r\n               get temperature           T\r\n
//    H\r\n               get humidity              H\r\n

char buffer[8];
byte bufferCount = 0;
byte receiveCount = 0;
float celsius;
float humidity;
int digitalCo2;
// timer vars
unsigned long currentMillis;
unsigned long oneSecond_PreviousMillis;
const long oneSecondInterval = 1000;


void setup() {
  Serial1.begin(9600); // here i am using an Arduino Mega2560 on Serial 1, if your using a Uno you can use software serial instead.
  // Or just use the Main Serial below which i am using for data output, but if you do just use the main serial alone be care what you print when debugging else you may
  // Send a cmd to the COZIR and calibrate it in the wrong situation i.e gas mode or whatever so please be careful.
  Serial.begin(115200);
  Serial.println(F("Starting system."));
}

void loop() {
  currentMillis = millis();
  if (currentMillis - oneSecond_PreviousMillis >= oneSecondInterval) { // one second timer
    cozSerialData();
    oneSecond_PreviousMillis = currentMillis;
  }
}

void cozSerialData(void) {
  Serial1.println(F("Z")); // request co2
  delay(50);
  Serial1.println(F("T")); // request celsius
  delay(50);
  Serial1.println(F("H")); // request R/H
  bool foundValues = false;
  while (!foundValues) {
    char input = Serial1.read();
    if (input > 0) {
      if (input != '\n' && input != 32 && bufferCount < 8) { // get responce for COZIR
        buffer[bufferCount] = input;
        bufferCount++;
      }
      else if (input == '\n') { // sort Serial data once we have recived the \n
        bufferCount = 0;
        //Serial.println(buffer);
        for (byte i = 0; i < 7; i++) { // shuffle array back one position to get rid of the capital letter
          buffer[i] = buffer[i + 1];
        }
        receiveCount++;
        if (receiveCount == 1) {
          digitalCo2 = atoi(buffer);
          Serial.print(F("\nDigital Co2 ppm: ")); Serial.println(digitalCo2);
        }
        else if (receiveCount == 2) {
          celsius = atof(buffer);
          if (celsius < 1000) {
            celsius = 0.1 * celsius;
          }
          else {
            celsius = 0.1 * (celsius - 1000);
          }
          Serial.print(F("Celsius: ")); Serial.println(celsius);
        }
        else if (receiveCount == 3) {
          humidity = atof(buffer);
          humidity = humidity * 0.1;
          Serial.print(F("Humidity: ")); Serial.println(humidity);
          receiveCount = 0;
          foundValues = true;
        }
        for (byte i = 0; i < 8; i++) { // clear the entire buffer array
          buffer[i] = 0;
        }
      }
    }
  }
}
