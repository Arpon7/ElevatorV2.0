const int SENSOR_FLOOR[4] = {2, 3, 4, 5};
const int BUTTON_FLOOR[4] = {6, 7, 8, 9};
const int BUTTON_CALL[3] = {10, 11, 12};

const int MOTOR_PWM = 44;
const int MOTOR_DIR1 = 46;
const int MOTOR_DIR2 = 48;

const int LED_FLOOR[4] = {22, 24, 26, 28};
const int LED_UP = 30;
const int LED_DOWN = 32;

const int BUZZER = 34;

const int MOTOR_SPEED = 200;
const int DEBOUNCE_DELAY = 50;
const int DOOR_OPEN_TIME = 2000;
const int FLOOR_TRAVEL_TIME = 3000;
const int MAX_QUEUE_SIZE = 10;

int currentFloor = 0;
String state = "IDLE";
String direction = "";

int requestQueue[MAX_QUEUE_SIZE];
int queueSize = 0;

unsigned long lastButtonPress[13];
bool buttonPressed[13];

int tripCount = 0;
int floorsTravel = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("=================================");
  Serial.println("4-FLOOR ELEVATOR SYSTEM");
  Serial.println("SCAN Algorithm Implementation");
  Serial.println("=================================");
  
  for (int i = 0; i < 4; i++) {
    pinMode(SENSOR_FLOOR[i], INPUT_PULLUP);
  }
  
  for (int i = 0; i < 4; i++) {
    pinMode(BUTTON_FLOOR[i], INPUT_PULLUP);
  }
  for (int i = 0; i < 3; i++) {
    pinMode(BUTTON_CALL[i], INPUT_PULLUP);
  }
  
  pinMode(MOTOR_PWM, OUTPUT);
  pinMode(MOTOR_DIR1, OUTPUT);
  pinMode(MOTOR_DIR2, OUTPUT);
  
  for (int i = 0; i < 4; i++) {
    pinMode(LED_FLOOR[i], OUTPUT);
  }
  pinMode(LED_UP, OUTPUT);
  pinMode(LED_DOWN, OUTPUT);
  
  pinMode(BUZZER, OUTPUT);
  
  stopMotor();
  allLEDsOff();
  
  for (int i = 0; i < 13; i++) {
    lastButtonPress[i] = 0;
    buttonPressed[i] = false;
  }
  
  digitalWrite(LED_FLOOR[currentFloor], HIGH);
  
  Serial.println("System initialized. Ready!");
  Serial.println("Current Floor: Ground (0)");
  Serial.println("State: IDLE");
  Serial.println();
}

void loop() {
  readButtons();
  updatePosition();
  
  if (state == "IDLE") {
    if (queueSize > 0) {
      determineDirection();
      sortQueue();
      state = direction == "UP" ? "MOVING_UP" : "MOVING_DOWN";
      Serial.println("Starting movement: " + state);
      printQueue();
      moveToNextFloor();
    }
  } 
  else if (state == "MOVING_UP" || state == "MOVING_DOWN") {
  }
  else if (state == "DOORS_OPENING" || state == "DOORS_CLOSING") {
  }
  
  updateLEDs();
  delay(10);
}

void readButtons() {
  unsigned long currentTime = millis();
  
  for (int i = 0; i < 4; i++) {
    if (digitalRead(BUTTON_FLOOR[i]) == LOW) {
      if (!buttonPressed[i] && (currentTime - lastButtonPress[i] > DEBOUNCE_DELAY)) {
        buttonPressed[i] = true;
        lastButtonPress[i] = currentTime;
        addRequest(i);
        Serial.println("Floor button pressed: " + String(i));
      }
    } else {
      buttonPressed[i] = false;
    }
  }
  
  for (int i = 0; i < 3; i++) {
    int buttonIndex = i + 4;
    if (digitalRead(BUTTON_CALL[i]) == LOW) {
      if (!buttonPressed[buttonIndex] && (currentTime - lastButtonPress[buttonIndex] > DEBOUNCE_DELAY)) {
        buttonPressed[buttonIndex] = true;
        lastButtonPress[buttonIndex] = currentTime;
        addRequest(i + 1);
        Serial.println("Call button pressed at floor: " + String(i + 1));
      }
    } else {
      buttonPressed[buttonIndex] = false;
    }
  }
}

void updatePosition() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(SENSOR_FLOOR[i]) == LOW) {
      if (currentFloor != i) {
        currentFloor = i;
        Serial.println("Position updated: Floor " + String(i));
      }
      break;
    }
  }
}

void addRequest(int floor) {
  if (floor == currentFloor && state == "IDLE") {
    Serial.println("Already at floor " + String(floor) + ", opening doors");
    openDoors();
    return;
  }
  
  for (int i = 0; i < queueSize; i++) {
    if (requestQueue[i] == floor) {
      Serial.println("Floor " + String(floor) + " already in queue");
      return;
    }
  }
  
  if (queueSize < MAX_QUEUE_SIZE) {
    requestQueue[queueSize] = floor;
    queueSize++;
    Serial.println("Added floor " + String(floor) + " to queue");
    printQueue();
  } else {
    Serial.println("Queue full! Cannot add floor " + String(floor));
  }
}

void removeRequest(int floor) {
  for (int i = 0; i < queueSize; i++) {
    if (requestQueue[i] == floor) {
      for (int j = i; j < queueSize - 1; j++) {
        requestQueue[j] = requestQueue[j + 1];
      }
      queueSize--;
      Serial.println("Removed floor " + String(floor) + " from queue");
      return;
    }
  }
}

void determineDirection() {
  if (queueSize == 0) {
    direction = "";
    return;
  }
  
  if (direction == "") {
    if (requestQueue[0] > currentFloor) {
      direction = "UP";
    } else if (requestQueue[0] < currentFloor) {
      direction = "DOWN";
    }
    Serial.println("Direction set to: " + direction);
  }
}

void sortQueue() {
  if (queueSize <= 1) return;
  
  Serial.println("Sorting queue using SCAN algorithm...");
  Serial.print("Direction: " + direction + ", Current Floor: ");
  Serial.println(currentFloor);
  
  if (direction == "UP") {
    int above[MAX_QUEUE_SIZE];
    int below[MAX_QUEUE_SIZE];
    int aboveCount = 0;
    int belowCount = 0;
    
    for (int i = 0; i < queueSize; i++) {
      if (requestQueue[i] >= currentFloor) {
        above[aboveCount++] = requestQueue[i];
      } else {
        below[belowCount++] = requestQueue[i];
      }
    }
    
    bubbleSort(above, aboveCount, true);
    bubbleSort(below, belowCount, false);
    
    int index = 0;
    for (int i = 0; i < aboveCount; i++) {
      requestQueue[index++] = above[i];
    }
    for (int i = 0; i < belowCount; i++) {
      requestQueue[index++] = below[i];
    }
  } 
  else if (direction == "DOWN") {
    int below[MAX_QUEUE_SIZE];
    int above[MAX_QUEUE_SIZE];
    int belowCount = 0;
    int aboveCount = 0;
    
    for (int i = 0; i < queueSize; i++) {
      if (requestQueue[i] <= currentFloor) {
        below[belowCount++] = requestQueue[i];
      } else {
        above[aboveCount++] = requestQueue[i];
      }
    }
    
    bubbleSort(below, belowCount, false);
    bubbleSort(above, aboveCount, true);
    
    int index = 0;
    for (int i = 0; i < belowCount; i++) {
      requestQueue[index++] = below[i];
    }
    for (int i = 0; i < aboveCount; i++) {
      requestQueue[index++] = above[i];
    }
  }
  
  Serial.println("Queue sorted:");
  printQueue();
}

void bubbleSort(int arr[], int size, bool ascending) {
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (ascending) {
        if (arr[j] > arr[j + 1]) {
          int temp = arr[j];
          arr[j] = arr[j + 1];
          arr[j + 1] = temp;
        }
      } else {
        if (arr[j] < arr[j + 1]) {
          int temp = arr[j];
          arr[j] = arr[j + 1];
          arr[j + 1] = temp;
        }
      }
    }
  }
}

void moveToNextFloor() {
  if (queueSize == 0) {
    state = "IDLE";
    direction = "";
    stopMotor();
    Serial.println("Queue empty. Returning to IDLE.");
    return;
  }
  
  int targetFloor = requestQueue[0];
  
  Serial.print("Moving to floor ");
  Serial.print(targetFloor);
  Serial.print(" from floor ");
  Serial.println(currentFloor);
  
  if (currentFloor == targetFloor) {
    stopMotor();
    removeRequest(targetFloor);
    openDoors();
    delay(DOOR_OPEN_TIME);
    closeDoors();
    
    tripCount++;
    
    if (queueSize > 0) {
      bool hasRequestsInDirection = false;
      if (direction == "UP") {
        for (int i = 0; i < queueSize; i++) {
          if (requestQueue[i] > currentFloor) {
            hasRequestsInDirection = true;
            break;
          }
        }
      } else if (direction == "DOWN") {
        for (int i = 0; i < queueSize; i++) {
          if (requestQueue[i] < currentFloor) {
            hasRequestsInDirection = true;
            break;
          }
        }
      }
      
      if (!hasRequestsInDirection) {
        direction = (direction == "UP") ? "DOWN" : "UP";
        Serial.println("Direction changed to: " + direction);
      }
      
      sortQueue();
      moveToNextFloor();
    } else {
      state = "IDLE";
      direction = "";
      Serial.println("All requests completed. State: IDLE");
      printStats();
    }
    return;
  }
  
  if (targetFloor > currentFloor) {
    moveUp();
    state = "MOVING_UP";
  } else {
    moveDown();
    state = "MOVING_DOWN";
  }
  
  int floorsToMove = abs(targetFloor - currentFloor);
  floorsTravel += floorsToMove;
  
  Serial.print("State: " + state + " | Traveling ");
  Serial.print(floorsToMove);
  Serial.println(" floor(s)...");
  
  delay(FLOOR_TRAVEL_TIME * floorsToMove);
  
  currentFloor = targetFloor;
  Serial.println("Arrived at floor " + String(currentFloor));
  
  stopMotor();
  removeRequest(targetFloor);
  openDoors();
  delay(DOOR_OPEN_TIME);
  closeDoors();
  
  tripCount++;
  
  delay(500);
  moveToNextFloor();
}

void moveUp() {
  digitalWrite(MOTOR_DIR1, HIGH);
  digitalWrite(MOTOR_DIR2, LOW);
  analogWrite(MOTOR_PWM, MOTOR_SPEED);
  direction = "UP";
}

void moveDown() {
  digitalWrite(MOTOR_DIR1, LOW);
  digitalWrite(MOTOR_DIR2, HIGH);
  analogWrite(MOTOR_PWM, MOTOR_SPEED);
  direction = "DOWN";
}

void stopMotor() {
  digitalWrite(MOTOR_DIR1, LOW);
  digitalWrite(MOTOR_DIR2, LOW);
  analogWrite(MOTOR_PWM, 0);
}

void openDoors() {
  state = "DOORS_OPENING";
  Serial.println("Opening doors at floor " + String(currentFloor));
  
  tone(BUZZER, 1000, 200);
  delay(200);
  tone(BUZZER, 1200, 200);
  
  state = "DOORS_OPEN";
}

void closeDoors() {
  state = "DOORS_CLOSING";
  Serial.println("Closing doors at floor " + String(currentFloor));
  
  tone(BUZZER, 800, 300);
  
  delay(500);
}

void updateLEDs() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_FLOOR[i], (i == currentFloor) ? HIGH : LOW);
  }
  
  digitalWrite(LED_UP, (state == "MOVING_UP") ? HIGH : LOW);
  digitalWrite(LED_DOWN, (state == "MOVING_DOWN") ? HIGH : LOW);
}

void allLEDsOff() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_FLOOR[i], LOW);
  }
  digitalWrite(LED_UP, LOW);
  digitalWrite(LED_DOWN, LOW);
}

void printQueue() {
  Serial.print("Current Queue [");
  Serial.print(queueSize);
  Serial.print("]: ");
  if (queueSize == 0) {
    Serial.println("Empty");
  } else {
    for (int i = 0; i < queueSize; i++) {
      Serial.print(requestQueue[i]);
      if (i < queueSize - 1) Serial.print(", ");
    }
    Serial.println();
  }
}

void printStats() {
  Serial.println("\n========== STATISTICS ==========");
  Serial.print("Total Trips: ");
  Serial.println(tripCount);
  Serial.print("Total Floors Traveled: ");
  Serial.println(floorsTravel);
  if (tripCount > 0) {
    Serial.print("Average Floors per Trip: ");
    Serial.println((float)floorsTravel / tripCount, 2);
  }
  Serial.println("================================\n");
}