const int waterLevelSensorLow = 0;
const int waterLevelSensorMedium = 1;
const int waterLevelSensorHigh = 2;

const int btnPause = 3;
const int btnAutoManual = 4;
const int btnSemiAuto =5;

const int ledWaterLevelLow = 6;
const int ledWaterLevelMedium = 7;
const int ledWaterLevelHigh = 8;

const int ledAuto = 9;
const int ledManual = 10;
const int ledPause = 13;
const int ledSemiAuto = A0;

bool isPaused = false;
bool isAuto = true;
bool isSemiAuto = false;

bool isPauseLedOn = false;

long ledBlinkingTimeInMilliSeconds = 200;
long lastPauseLedChangedTime = 0;

//Water Level 0 = Empty, 1 = LOW, 2 = MEDIUM, 3 = HIGH
int waterLevel = 0;

void setup() {
  
  Serial.begin(9600);
  
  pinMode(btnPause, INPUT);
  pinMode(btnAutoManual, INPUT);
  pinMode(btnSemiAuto, INPUT);
  
  pinMode(ledPause, OUTPUT);
  pinMode(ledAuto, OUTPUT);
  pinMode(ledManual, OUTPUT);
  pinMode(ledSemiAuto, OUTPUT);
  pinMode(ledWaterLevelLow, OUTPUT);
  pinMode(ledWaterLevelMedium, OUTPUT);
  pinMode(ledWaterLevelHigh, OUTPUT);
  
  pinMode(waterLevelSensorHigh, INPUT);
  pinMode(waterLevelSensorMedium, INPUT);
  pinMode(waterLevelSensorLow, INPUT);
}

void loop() {
  
  setWaterLevel();
  handleWaterLevelLed();
  
  setAutoStatus();
  handleAutoManualLeds();
  
  setPauseStatus();
  handlePauseLed();
  
  setSemiAutoStatus();
  handleSemiAutoLed();
}

void setWaterLevel() {
  
  int low = digitalRead(waterLevelSensorLow);
  int medium = digitalRead(waterLevelSensorMedium);
  int high = digitalRead(waterLevelSensorHigh);
  
  Serial.print("Low Sensor Value: ");
  Serial.println(low);
  
  Serial.print("Medium Sensor Value: ");
  Serial.println(medium);
  
  Serial.print("High Sensor Value: ");
  Serial.println(high);
  
  if (low == LOW && medium == LOW && high == LOW) {
  	waterLevel = 3;
  } else if (low == LOW && medium == LOW && high == HIGH) {
  	waterLevel = 2;
  } else if (low == LOW && medium == HIGH && high == HIGH) {
  	waterLevel = 1;
  } else if (low == HIGH && medium == HIGH && high == HIGH) {
  	waterLevel = 0;
  } else {
  	waterLevel = -1;
  }
}

void handleWaterLevelLed() {
  
  Serial.println("Water Level: ");
  Serial.println(waterLevel);
    
  if (waterLevel == 1) {
    
    digitalWrite(ledWaterLevelHigh, LOW);
    digitalWrite(ledWaterLevelMedium, LOW);
    digitalWrite(ledWaterLevelLow, HIGH);
    
  } else if (waterLevel == 2) {
    
    digitalWrite(ledWaterLevelHigh, LOW);
    digitalWrite(ledWaterLevelMedium, HIGH);
    digitalWrite(ledWaterLevelLow, HIGH);
    
  } else if (waterLevel == 3) {
    
    digitalWrite(ledWaterLevelHigh, HIGH);
    digitalWrite(ledWaterLevelMedium, HIGH);
    digitalWrite(ledWaterLevelLow, HIGH);
  
  } else {
  
    digitalWrite(ledWaterLevelHigh, LOW);
    digitalWrite(ledWaterLevelMedium, LOW);
    digitalWrite(ledWaterLevelLow, LOW);
  }
  
  Serial.println("Low Led: ");
  Serial.println(ledWaterLevelLow);
  
  Serial.print("Medium Led: ");
  Serial.println(ledWaterLevelMedium);
  
  Serial.print("High Led: ");
  Serial.println(ledWaterLevelHigh);
}

void setAutoStatus() {
 
  int autoButtonStatus = digitalRead(btnAutoManual);
  
  if (autoButtonStatus == HIGH) {
    
    isAuto = !isAuto;
    handleAutoManualLeds();
    delay(100);
  }
}

void handleAutoManualLeds() {
  
  bool isAutoLedOn = isAuto && !isPaused;
  digitalWrite(ledAuto, isAutoLedOn);
  
  bool isManualLedOn = !isAuto && !isPaused;
  digitalWrite(ledManual, isManualLedOn);
}

void setPauseStatus() {
  
  int pauseButtonStatus = digitalRead(btnPause);
  
  if (pauseButtonStatus == HIGH) {
    
    isPaused = !isPaused;
    handleAutoManualLeds();
    handlePauseLed();
    delay(100);
  }
}

void handlePauseLed() {
  
  long millist = millis();
  
  if(isPaused) {
    
    long timeElapsedAfterLastChange = millis() - lastPauseLedChangedTime;
    
    if(timeElapsedAfterLastChange > ledBlinkingTimeInMilliSeconds) {
      
      isPauseLedOn = !isPauseLedOn;
      digitalWrite(ledPause, isPauseLedOn);
      lastPauseLedChangedTime = millis();
    }
  } else {
  
    digitalWrite(ledPause, LOW);
    isPauseLedOn = false;
  } 
}

void setSemiAutoStatus() {
  
  int semiAutoButtonStatus = digitalRead(btnSemiAuto);
  
  if (semiAutoButtonStatus == HIGH) {
    
    isSemiAuto = !isSemiAuto;
    handleSemiAutoLed();
    delay(100);
  }
}

void handleSemiAutoLed() {
  
  bool isSemiAutoLedOn = isSemiAuto && waterLevel == 2;
  digitalWrite(ledSemiAuto, isSemiAutoLedOn);
}
