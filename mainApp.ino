#include <EEPROM.h>

const unsigned long debounceDelay = 50;

const int waterLevelSensorLow = 0;
const int waterLevelSensorMedium = 1;
const int waterLevelSensorHigh = 2;

const int btnPause = 3;
int btnPauseState;
int lastBtnPauseState = LOW;
unsigned long lastBtnPauseDebounceTime = 0;

const int btnAutoManual = 4;
int btnAutoManualState;
int lastBtnAutoManualState = LOW;
unsigned long lastBtnAutoManualDebounceTime = 0;

const int btnSemiAuto =5;
int btnSemiAutoState;
int lastSemiAutoState = LOW;
unsigned long lastBtnSemiAutoDebounceTime = 0;

const int ledWaterLevelLow = 6;
const int ledWaterLevelMedium = 7;
const int ledWaterLevelHigh = 8;

const int ledAuto = 9;
const int ledManual = 10;
const int ledPause = 13;
const int ledSemiAuto = A0;

const int waterMotor = A2;
const int buzzer = A3;

const int sumpTankLowSensor = A4;
const int ledSumpTankLow = A5;

const int ledRunning = 12;
const int ledStandby = 11;
int standbyLedBrightness = 0;
int standbyLedFadeAmount = 2;

const int ledTimer = A1;

bool isPaused = false;
bool isAuto = true;
bool isSemiAuto = false;

bool isPauseLedOn = false;
bool isRunningLedOn = false;
bool isTimerLedOn = false;

const long ledBlinkingTimeInMilliSeconds = 200;
long lastPauseLedChangedTime = 0;
long lastRunningLedChangedTime = 0;
long lastTimerLedChangedTime = 0;

//Water Level 0 = Empty, 1 = LOW, 2 = MEDIUM, 3 = HIGH
int waterLevel = -1;
int lastWaterLevel = -1; 

bool runWaterMotor = false;
bool isWaterMotorRunning = false;

bool isSumpTankLow = false;

long motorStarTime = 0;
//5 minutes
const long motorMaxRunTimeInMilliseconds = 300000;

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
  
  pinMode(waterMotor, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  pinMode(sumpTankLowSensor, INPUT);
  pinMode(ledSumpTankLow, OUTPUT);
  
  pinMode(ledRunning, OUTPUT);
  pinMode(ledStandby, OUTPUT);
  
  pinMode(ledTimer, OUTPUT);
  
  //readFromEEPROM();
  
  buzzerBeep();
  buzzerBeep();
}

void readFromEEPROM() {
  
  bool isEEPROMUpdated;
  EEPROM.get(EEPROM.length() - 1, isEEPROMUpdated);
  
  if (!isEEPROMUpdated){
    return;
  }
  
  int eeAddress = 0;
  
  EEPROM.get(eeAddress, isPaused);
  eeAddress += sizeof(bool);
    
  EEPROM.get(eeAddress, isAuto);
  eeAddress += sizeof(bool);
  
  EEPROM.get(eeAddress, isSemiAuto);
  eeAddress += sizeof(bool);
  
  EEPROM.get(eeAddress, lastWaterLevel);
  eeAddress += sizeof(int);
  
  EEPROM.get(eeAddress, runWaterMotor);
  eeAddress += sizeof(bool);
  
  EEPROM.get(eeAddress, isWaterMotorRunning);
  eeAddress += sizeof(bool);
  
  EEPROM.get(eeAddress, motorStarTime);
  eeAddress += sizeof(long);
}

void loop() {
  
  updateSumpTankWaterStatus();
  updateWaterLevel();
  updateButtonStatuses();
  handleWaterMotor();
  //updateEEPROM();
}

void updateSumpTankWaterStatus() {
  int status = digitalRead(sumpTankLowSensor);
  
  isSumpTankLow = !status;
  digitalWrite(ledSumpTankLow, isSumpTankLow);
}

void updateWaterLevel() {
  setWaterLevel();
  handleWaterLevelLed();
}

void setWaterLevel() {
  
  int low = digitalRead(waterLevelSensorLow);
  int medium = digitalRead(waterLevelSensorMedium);
  int high = digitalRead(waterLevelSensorHigh);
  
  int calculatedWaterLevel;
  
  if (low == LOW && medium == LOW && high == LOW) {
  	calculatedWaterLevel = 3;
  } else if (low == LOW && medium == LOW && high == HIGH) {
  	calculatedWaterLevel = 2;
  } else if (low == LOW && medium == HIGH && high == HIGH) {
  	calculatedWaterLevel = 1;
  } else if (low == HIGH && medium == HIGH && high == HIGH) {
  	calculatedWaterLevel = 0;
  } else {
  	calculatedWaterLevel = -1;
  }
  
  if(waterLevel != calculatedWaterLevel) {
    
    if(lastWaterLevel == -1) {
      lastWaterLevel = calculatedWaterLevel;
    } else {
      lastWaterLevel = waterLevel;
    }
    
    waterLevel = calculatedWaterLevel;
  }
}

void handleWaterLevelLed() {
    
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
}

void updateButtonStatuses() {
  setAutoStatus();
  setPauseStatus();
  setSemiAutoStatus();
  
  handleAutoManualLeds();
  handlePauseLed();
  handleSemiAutoLed();
}

void setAutoStatus() {
  
  int reading = digitalRead(btnAutoManual);
  
  if (reading != lastBtnAutoManualState) {
    lastBtnAutoManualDebounceTime = millis();
  }
  
  if ((millis() - lastBtnAutoManualDebounceTime) > debounceDelay) {
    if (reading != btnAutoManualState) {
      btnAutoManualState = reading;

      if (btnAutoManualState == HIGH) {
        isAuto = !isAuto;
        buzzerBeep();
      }
    }
  }
  lastBtnAutoManualState = reading;
}

void handleAutoManualLeds() {
  
  bool isAutoLedOn = isAuto && !isPaused;
  digitalWrite(ledAuto, isAutoLedOn);
  
  bool isManualLedOn = !isAuto && !isPaused;
  digitalWrite(ledManual, isManualLedOn);
}

void setPauseStatus() {
  
  int reading = digitalRead(btnPause);
  
  if (reading != lastBtnPauseState) {
    lastBtnPauseDebounceTime = millis();
  }
  
  if ((millis() - lastBtnPauseDebounceTime) > debounceDelay) {
    if (reading != btnPauseState) {
      btnPauseState = reading;

      if (btnPauseState == HIGH) {
        isPaused = !isPaused;
      }
    }
  }
  lastBtnPauseState = reading;
}

void handlePauseLed() {
    
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

  if(isPauseLedOn) {
    buzzerBeep();
  }
}

void setSemiAutoStatus() {
  
  int reading = digitalRead(btnSemiAuto);
  
  if (reading != lastSemiAutoState) {
    lastBtnSemiAutoDebounceTime = millis();
  }
  
  if ((millis() - lastBtnSemiAutoDebounceTime) > debounceDelay) {
    if (reading != btnSemiAutoState) {
      btnSemiAutoState = reading;

      if (btnSemiAutoState == HIGH) {
        isSemiAuto = !isSemiAuto && (waterLevel < 3 && waterLevel > 0);
        buzzerBeep();
      }
    }
  }
  lastSemiAutoState = reading;
  
  if (waterLevel == 3) {
    isSemiAuto = false;
  }
}

void handleSemiAutoLed() {
  digitalWrite(ledSemiAuto, isSemiAuto && isAuto && !isPaused);
}

void handleWaterMotor() {
  setWaterMotorRunStatus();
  
  if(isWaterMotorRunning != runWaterMotor) {
    
    if(runWaterMotor) {
      buzzerBeep();
      buzzerBeep();
      buzzerBeep();
      
      motorStarTime = millis();
    }
    
    digitalWrite(waterMotor, runWaterMotor);
    isWaterMotorRunning = runWaterMotor;
    
    if(!runWaterMotor) {
      buzzerBeep();
      buzzerBeep();
      buzzerBeep();
      
      motorStarTime = 0;
    }
  }
  
  handleRunningLed();
  handleSandbyLed();
  handleTimerLed();
}

void setWaterMotorRunStatus() {
  
  bool isWaterLevelLow = waterLevel == 0;
  bool isWaterLevelHigh = waterLevel == 3;
  bool isWaterLevelLowToHigh = waterLevel < 3 && waterLevel > 0;
  bool isTankFillingUp = isWaterLevelLowToHigh && waterLevel > lastWaterLevel;
  bool runMotorAutoMode = isAuto && (isWaterLevelLow || isTankFillingUp);
  bool runMotorSemiAutoMode = isSemiAuto && (isWaterLevelLowToHigh || isTankFillingUp);
  bool isTimerUp = isAuto && (millis() - motorStarTime) >= motorMaxRunTimeInMilliseconds;
  
  
  runWaterMotor = (runMotorAutoMode || runMotorSemiAutoMode || !(isAuto || isWaterLevelHigh)) && !isPaused && !isSumpTankLow && !isTimerUp;
}

void handleRunningLed() {
    
  if(isWaterMotorRunning) {
    
    long timeElapsedAfterLastChange = millis() - lastRunningLedChangedTime;
    
    if(timeElapsedAfterLastChange > ledBlinkingTimeInMilliSeconds) {
      
      isRunningLedOn = !isRunningLedOn;
      digitalWrite(ledRunning, isRunningLedOn);
      lastRunningLedChangedTime = millis();
    }
  } else {
  
    digitalWrite(ledRunning, LOW);
    isRunningLedOn = false;
  } 

  if(isRunningLedOn) {
    buzzerBeep();
  }
}

void handleSandbyLed() {
  
  if(!isWaterMotorRunning && (isAuto || isSemiAuto) && !isPaused) {      
  	
    analogWrite(ledStandby, standbyLedBrightness);
  	standbyLedBrightness = standbyLedBrightness + standbyLedFadeAmount;

  	if (standbyLedBrightness <= 0 || standbyLedBrightness >= 255) {
    	standbyLedFadeAmount = -standbyLedFadeAmount;
  	}
    
  	delay(30);
    
  } else {
    digitalWrite(ledStandby, LOW);
  }
}

void handleTimerLed() {
  
  if(isWaterMotorRunning && motorStarTime > 0 && isAuto){
    
    long timeElapsedAfterLastChange = millis() - lastTimerLedChangedTime;
    
    if(timeElapsedAfterLastChange > ledBlinkingTimeInMilliSeconds) {
      
      isTimerLedOn = !isTimerLedOn;
      digitalWrite(ledTimer, isTimerLedOn);
      lastTimerLedChangedTime = millis();
    }
  } else {
     digitalWrite(ledTimer, LOW);
     isTimerLedOn = false;
  }
}

void buzzerBeep() {
  const int buzzerTime = 250;
  
  digitalWrite(buzzer, HIGH);
  delay(buzzerTime);
  digitalWrite(buzzer, LOW);
  delay(buzzerTime);
}

void updateEEPROM() {
  
  int eeAddress = 0;
  
  EEPROM.update(eeAddress, isPaused);
  eeAddress += sizeof(bool);
    
  EEPROM.update(eeAddress, isAuto);
  eeAddress += sizeof(bool);
  
  EEPROM.update(eeAddress, isSemiAuto);
  eeAddress += sizeof(bool);
  
  EEPROM.update(eeAddress, lastWaterLevel);
  eeAddress += sizeof(int);
  
  EEPROM.update(eeAddress, runWaterMotor);
  eeAddress += sizeof(bool);
  
  EEPROM.update(eeAddress, isWaterMotorRunning);
  eeAddress += sizeof(bool);
  
  EEPROM.update(eeAddress, motorStarTime);
  eeAddress += sizeof(long);
  
  EEPROM.update(EEPROM.length() - 1, true);
}
