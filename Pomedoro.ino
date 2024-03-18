/*
  _____      __  __ ______    _                 _ 
 |  __ \    |  \/  |  ____|  | |               | |
 | |__) |__ | \  / | |__   __| | ___  _ __ ___ | |
 |  ___/ _ \| |\/| |  __| / _` |/ _ \| '__/ _ \| |
 | |  | (_) | |  | | |___| (_| | (_) | | | (_) |_|
 |_|   \___/|_|  |_|______\__,_|\___/|_|  \___/(_)
                                                                                       
@author Saul Figueroa (2024)
PoMEdoro (ポミドーロ)

<!> NOTE: It is recommended to use a prototyping shield, which
 can help make the design more compact and also easily enables
 the RESET functionality for the device.

 ~220Ω resistor: we'll call it "R1". There are some references to
 "same node as R1", this means that certain components are connected
 to one end of R1 and the opposite end of R1 goes to GND.
 
 ~LED: connect to 5V and same node as R1
 ~LCD:
 * VSS to GND
 * VDD to 5V
 * V0 to potentiometer (one end of potentiometer to 5V and the other to GND)
 * RS to digital pin 12
 * RW to GND
 * E (Enable) to digital pin 11
 * D4 pin to digital pin 5
 * D5 pin to digital pin 4
 * D6 pin to digital pin 3
 * D7 pin to digital pin 2
 * A to 5V
 * K to R1 and other end of resistor to GND

 ~Tilt switch:
 * One end to digital pin 10 an other to same node as R1

 ~Passive Buzzer:
 * Positive (+) end to digital pin 9 and the other to same node as R1
*/

// Include LCD library:
#include <LiquidCrystal.h>

int animationCounter = 0; // Keep track of animations
bool animationCelebrate = false; // Keep track of the celebrate animation, separately
bool animationSleepy = false; // Keep track of the sleep animation, separately

// Keep track of time:
int minutes = 25;
int seconds = 0;


int roundCounter = 0; // Keep track of the number of rounds

// Passive Buzzer is connected to GND and Digital Pin 8
int buzzerPin = 9;

// Tilt switch goes to ground and digital pin (pull-up)
int tiltPin = 10; // Digital Pin 10 has tilt sensor: 0/LOW when normal; 1/HIGH when tilted
bool tilted = false; // For managing "toggle functionality" with tilt sensor 

// Custom backslash for LCD display
byte backslash[8] = {
  0b00000,
  0b10000,
  0b01000,
  0b00100,
  0b00010,
  0b00001,
  0b00000,
  0b00000
};

// Custom Flag for LCD display
byte flag[8] = {
  B01000,
  B01100,
  B01110,
  B01111,
  B01000,
  B01000,
  B01000,
  B11100
};

// Custom Big for LCD display
byte smileBig[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B01110,
  B00000,
};

// Custom Smile for LCD display
byte smile[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};

// Custom Distress Mouth for LCD display (Not used in first released version)
byte distressMouth[8] = {
  B00000,
  B00000,
  B00000,
  B00100,
  B01010,
  B10001,
  B11111,
  B00000
};

// Custom Eye for LCD display
byte eye[8] = {
  B01110,
  B10011,
  B10011,
  B10011,
  B11111,
  B01110,
  B00000,
  B00000
};

// Initialize LCD library
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Print banner messages on LCD
void printBanner(String msg){
  lcd.setCursor(0, 0);
  lcd.print(msg);
}

// Sleep mode when tilted while timer is counting
void sleepTimer(){
  delay(500);
  sleepNotification(); // Play sound
 
  // Clear display
  lcd.clear();

  while(1){
  printBanner(" *yawn*");
  printSleepy();

  // Check if device is tilted (each iteration of this main loop/each second)
  if (digitalRead(tiltPin) == HIGH){
    tilted = !tilted;
    sleepNotification(); // Play sound (on exit)
    delay(500);
    sleepNotification(); 
    // Clear display
    lcd.clear();
    delay(500);
    return; // Exit sleep state
    }
  delay(1000); // 1 second delay
  }
}

void displayRest(){
  // Clear display
  lcd.clear();

  // Set rest duration (5 min. or 30 min for long rest)
  if (roundCounter < 4){ // There'll be a longer rest after 4 pomodoros
     // Set banner message
    minutes = 5;
    seconds = 0;
  }else{
    minutes = 30;
    seconds = 0;
  }

  while ((minutes > 0) || (seconds > 0)){
    // Manage banner messages
    if (roundCounter < 4){
      printBanner("You can rest!");
    }else{
      printBanner("Amazing! Relax!");
    }
    // Check if device is tilted (each iteration of this main loop/each second)
    if (digitalRead(tiltPin) == HIGH){
      tilted = !tilted;
      toggleNotification(); // Play Tilt Notification
    }

    // Handle timer's pause
    if (tilted){
      sleepTimer();
  }

    // Display animation
    printCelebrate();

    if(seconds < 0){
      minutes--;
      seconds = 59;
    }
    
    lcd.setCursor(11, 1);
    lcd.print(minutes);
    lcd.print(':');
    lcd.print(seconds--);
    lcd.print("   "); // Clean some extra characters that appeared on display
    lcd.print("  ");

    delay(1000); // 1 second
  }

  if (roundCounter >= 4) roundCounter = 0; // Restart number of rounds
  timerNotification(); // Play sound (on exit)
}

void displayTimer(){
  // Update values: 
  if(seconds < 0){
    minutes--;
    seconds = 59;
  }

  if ((minutes <= 0 ) && (seconds <= 0)){
    timerNotification(); // Play sound
    displayRest();
    roundCounter++;
    minutes = 25; // Reset 25 min. timer
    seconds = 0;
  }

  lcd.setCursor(11, 1);
  lcd.print(minutes);
  lcd.print(':');
  lcd.print(seconds--);
  lcd.print("  ");
}

void timerNotification(){
  // B4, C5, D5

  tone(buzzerPin, 493.88); // B4
  delay(250);
  tone(buzzerPin, 523.25); // C5
  delay(250);
  tone(buzzerPin, 587.33); // D5
  delay(250);

  noTone(buzzerPin); // Silence/End
}

void sleepNotification(){
  tone(buzzerPin, 523.25); // C5
  delay(250);

  noTone(buzzerPin); // Silence/End
}

void setupNotification(){
  // G4, A4, B4, C5, D5, C5
  
  tone(buzzerPin, 392); // G4
  delay(250);
  tone(buzzerPin, 440); // A4
  delay(250);
  tone(buzzerPin, 493.88); // B4
  delay(250);
  tone(buzzerPin, 523.25); // C5
  delay(500);
  tone(buzzerPin, 587.33); // D5
  delay(250);
  tone(buzzerPin, 523.25); // C5
  delay(750);

  noTone(buzzerPin); // Silence/End
}

void toggleNotification(){
  // G4, B4, D5

  tone(buzzerPin, 392); // G4
  delay(250);
  tone(buzzerPin, 493.88); // B4
  delay(250);
  tone(buzzerPin, 587.33); // D5
  delay(250);

  noTone(buzzerPin); // Silence/End
}

void printFlag(){
  // Set the cursor to column 0, line 14 (second row); all the way to the left in first line
  lcd.setCursor(14, 0);

  // Display flag
  lcd.write(byte(4));
}

void printBigSmily(){
  // Set the cursor to column 0, line 1 (second row); all the way to the left in first line
  lcd.setCursor(0, 1);

  // Display animation
  lcd.write(byte(5));
  lcd.print("(");
  lcd.write(byte(1));
  lcd.print(" ");
  lcd.write(byte(3)); 
  lcd.print(" ");
  lcd.write(byte(1));
  lcd.print(")/ <Hi!>");
}

void printSleepy(){
  // Toggle value to change animation
  animationSleepy = !animationSleepy;

  // Set the cursor to column 0, line 1 (second row); all the way to the left in first line
  lcd.setCursor(0, 1);

  if(animationSleepy){
    lcd.print("(- _ -) zZz     ");
  }else{
    lcd.print("(- o -) ZzZ     ");
  }
}

void printCelebrate(){
  // Toggle value to change animation
  animationCelebrate = !animationCelebrate; 

  if(animationCelebrate){
    lcd.setCursor(0, 1);

    lcd.write(byte(5)); 
    lcd.print("(> ");
    lcd.write(byte(3)); 
    lcd.print(" <)/");
    lcd.print("  ");
  }else{
    lcd.setCursor(0, 1);

    lcd.print("/(> ");
    lcd.write(byte(0)); 
    lcd.print(" <)");
    lcd.write(byte(5)); 
    lcd.print("  ");
  }
}


void printBigSmily2(){
  // Set the cursor to column 0, line 1 (second row); all the way to the left in first line
  lcd.setCursor(0, 1);

  // Display animation
  lcd.write(byte(5));
  lcd.print("(");
  lcd.write(byte(1));
  lcd.print(" ");
  lcd.write(byte(3)); 
  lcd.print(" ");
  lcd.write(byte(1));
  lcd.print(")/");
}

void printSmily(){
  lcd.setCursor(0, 1);

  lcd.print(" (");
  lcd.write(byte(1));
  lcd.print(" ");
  lcd.write(byte(0)); 
  lcd.print(" ");
  lcd.write(byte(1));
  lcd.print(")   ");
}

void printBlinky(){
  lcd.setCursor(0, 1);

  lcd.print(" (- ");
  lcd.write(byte(0)); 
  lcd.print(" -)  ");
}

void printDistress(){
  lcd.setCursor(0, 1);

  lcd.print("(O ");
  lcd.write(byte(2)); 
  lcd.print(" O)<!>");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  delay(500);

  // Create custom smile
  lcd.createChar(0, smile);
  // Create custom eye
  lcd.createChar(1, eye);
  // Create custom mouth
  lcd.createChar(2, distressMouth);
  // Create custom big mouth
  lcd.createChar(3, smileBig);
  // Create custom big mouth
  lcd.createChar(4, flag); 
  // Create custom backslash
  lcd.createChar(5, backslash);

  // Set up the LCD's number of columns and rows
  lcd.begin(16, 2);

  // Display LCD message
  printBanner("|PoMEdoro|");
  printBigSmily();
  
  // Setup tilt sensor
  pinMode(tiltPin, INPUT_PULLUP);

  // Passive Buzzer Setup
  pinMode(buzzerPin, OUTPUT);
  setupNotification();

  // Initial "toggle state" notification
  printFlag();
  lcd.print(roundCounter);

  // Wait for user to start the timer by tilting the device
  while(!tilted){
    tilted = digitalRead(tiltPin);
    delay(250);
  }
  
  // Update round counter
  roundCounter++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  // Display timer
  displayTimer();

  // Print transition animation
  printBigSmily2();

  // Display LCD message
  printBanner(" Time 2 Work! ");

  // Print flag
  printFlag();

  // Update round flag
  lcd.setCursor(15, 0);
  lcd.print(roundCounter);

  // Check if device is tilted (each iteration of this main loop/each second)
  if (digitalRead(tiltPin) == HIGH){
    tilted = !tilted;
    toggleNotification(); // Play Tilt Notification
  }

  // Handle timer's pause
  if (tilted){
    sleepTimer();
  }

  if(animationCounter <= 3)
    printSmily();
  else
    printBlinky();

  // animationCounter++;

  if(animationCounter++ >= 4)
    animationCounter = 0;

  delay(1 * 1000); // 1 second
  // lcd.print(animationCounter); 
}
