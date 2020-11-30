// http://www.co2meters.com/Documentation/Manuals/Manual-GSS-Sensors.pdf

//    Command             Description               Example         Notes

//    G\r\n               fresh air calibration     G\r\n           Assumes 450ppm in fresh air !
//    F ##### #####\r\n   exact calibration         F 410 400\r\n   Note the space after F and the first value
//    K #\r\n             set mode                  K 1\r\n         Mode 0 = Command Mode,  Mode 1 = Streaming Mode,  Mode 2 = Polling Mode. --- Note the space after K
//    Z\r\n               get Co2                   Z\r\n
//    T\r\n               get temperature           T\r\n
//    H\r\n               get humidity              H\r\n

const int bufferLength = 30;
char buffer[bufferLength];
byte bufferCount = 0;
byte receiveCount = 0;
float celsius;
float humidity;
int digitalCo2;
const int commandDelay = 50;
bool pollingEnabled = true;
// timer vars
unsigned long currentMillis;
unsigned long oneSecond_PreviousMillis;
const long oneSecondInterval = 10000;

void readData();

void setup() {
  Serial1.begin(9600);
  Serial.begin(115200);
  delay(300);

  Serial.println(F("=========================="));
  Serial.println(F("Starting system."));
  Serial1.println("K 2"); // polling mode
  delay(commandDelay);
  readData();

  oneSecond_PreviousMillis = millis() - oneSecondInterval;
}

void readData(){
  bool foundValues = false;
  for (byte i = 0; i < bufferLength; i++) { // clear the entire buffer array
    buffer[i] = 0;
  }
        
  while (!foundValues) {
    char input = Serial1.read();
    if (input > 0) {
      if (input != '\n' && bufferCount < bufferLength) { // get responce for COZIR
        buffer[bufferCount] = input;
        bufferCount++;
        //Serial.print(char(input));
      }
      else if (input == '\n') { // sort Serial data once we have recived the \n
        foundValues = true;
        //Serial.print("found n: ");Serial.print(bufferCount);
        buffer[bufferCount] = 0;

        for (byte i = 0; i < bufferCount; i++) { // write what was in the buffer
          //Serial.print(char(buffer[i]));
        }
        //Serial.println(" :end");
        foundValues = true;
      }
      else if (bufferCount >= bufferLength) {
        Serial.println("overflow");
        Serial.println(bufferCount);
        foundValues = true; 
      }
      else{
        Serial.println("unknown error");
        Serial.println(char(input));
        foundValues = true;
      }
    }
    else{
      Serial.print("input = 0");
    }
    //Serial.print("foundValue: "); Serial.println(foundValues);
  }
  //Serial.println("resetting bufferCount");
  bufferCount = 0;
  delay(commandDelay);
  return;
}

float getIntFromReturn(char code){
  int value = atoi(&buffer[3]);
  Serial.println(value);
  

  return -1;
}

float getCo2(){
  Serial1.println("Z"); // request co2
  delay(commandDelay);
  readData();
  if (buffer[1] != 'Z'){
    Serial.print("Z:"); // request co2
    Serial.println(buffer);
    pollingEnabled = false;
  }
  float co2 = atoi(&buffer[3]);
  return co2;
}

float getTemp(){
  Serial1.println("T"); // request temp
  delay(commandDelay);
  readData();
  if (buffer[1] != 'T'){
    Serial.print("T:"); // request temp
    Serial.println(buffer);
    pollingEnabled = false;
  }

  float temp = atoi(&buffer[5]) / 10.0;
  temp = (temp * 9 / 5) + 32;

  return temp;
}


float getHumidity(){
  Serial1.println("H"); // request humidity
  delay(commandDelay);
  readData();
  if (buffer[1] != 'H'){
    Serial.print("H"); // request humidity
    Serial.println(buffer);
    pollingEnabled = false;
  }
  
  float humidity = atoi(&buffer[3]) / 10.0;
  return humidity;
}


void loop() {
  currentMillis = millis();
  if (currentMillis - oneSecond_PreviousMillis >= oneSecondInterval && pollingEnabled == true) { // one second timer
    float co2 = getCo2();
    float temp = getTemp();
    float humidity = getHumidity();
    Serial.print("CO2 "); Serial.print(co2);
    Serial.print(" Temp "); Serial.print(temp);
    Serial.print(" Humidity "); Serial.println(humidity);
    
    oneSecond_PreviousMillis = currentMillis;
  }
}
