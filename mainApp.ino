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

bool isPaused = false;
bool isAuto = true;
bool isSemiAuto = false;

bool isPauseLedOn = false;

long ledBlinkingTimeInMilliSeconds = 200;
long lastPauseLedChangedTime = 0;

//Water Level 0 = Empty, 1 = LOW, 2 = MEDIUM, 3 = HIGH
int waterLevel = -1;
int lastWaterLevel = -1; 

bool runWaterMotor = false;

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
}

void loop() {
  
  updateWaterLevel();
  updateButtonStatuses();
  handleWaterMotor();
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
  
  int reading = digitalRead(btnSemiAuto);
  
  if (reading != lastSemiAutoState) {
    lastBtnSemiAutoDebounceTime = millis();
  }
  
  if ((millis() - lastBtnSemiAutoDebounceTime) > debounceDelay) {
    if (reading != btnSemiAutoState) {
      btnSemiAutoState = reading;

      if (btnSemiAutoState == HIGH) {
        isSemiAuto = !isSemiAuto && (waterLevel < 3 && waterLevel > 0);
      }
    }
  }
  lastSemiAutoState = reading;
}

void handleSemiAutoLed() {
  digitalWrite(ledSemiAuto, isSemiAuto && isAuto && !isPaused);
}

void handleWaterMotor() {
  setWaterMotorRunStatus();
  
  digitalWrite(waterMotor, runWaterMotor);
}

void setWaterMotorRunStatus() {
  
  bool isWaterLevelLow = waterLevel == 0;
  bool isWaterLevelLowToHigh = waterLevel < 3 && waterLevel > 0;
  bool isTankFillingUp = isWaterLevelLowToHigh && waterLevel > lastWaterLevel;
  
  bool runMotorAutoMode = isAuto && (isWaterLevelLow || isTankFillingUp);
  
  runWaterMotor = (runMotorAutoMode || isSemiAuto || !isAuto) && !isPaused;
}
