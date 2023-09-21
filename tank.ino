//  4 Channel Receiver | 4 Kanal Alıcı
//  PWM output on pins D2, D3, D4, D5 (Çıkış pinleri)
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
//#include <Servo.h>
#define LEFT_TRACK_PWM_PIN 6
#define RIGHT_TRACK_PWM_PIN 5

#define R_FLAG_VPRED_PIN 22
#define R_FLAG_VZAD_PIN 23
#define L_FLAG_VPRED_PIN 24
#define L_FLAG_VZAD_PIN 25

LiquidCrystal_I2C lcd(0x27, 16, 4); // I2C address 0x27, 16 column and 2 rows

int right_track_incoming = 0;
int right_track_final = 0;

int left_track_incoming = 0;
int left_track_final = 0;

int ch_width_1 = 0;
int ch_width_2 = 0;
int ch_width_4 = 0;

int RT_FRONT = 0;
int RT_FRONT_FINAL = 0;
int RT_BACK = 0;
int RT_BACK_FINAL = 0;
int R_FLAG_VPRED = 0;
int R_FLAG_VZAD = 0;

int LT_FRONT = 0;
int LT_FRONT_FINAL = 0;
int LT_BACK = 0;
int LT_BACK_FINAL = 0;
int L_FLAG_VPRED = 0;
int L_FLAG_VZAD = 0;

struct Signal  {  
  byte rightTracks;
  byte leftTracks;
};

Signal data;
const uint64_t pipeIn = 0xE9E8F0F0E1LL;
RF24 radio(7, 8); 
void ResetData() {
  data.rightTracks = 127;  // Center
  data.leftTracks = 127;   // Center
}

void setup() {
  lcd.init(); // initialize the lcd
  lcd.backlight();

  pinMode(LEFT_TRACK_PWM_PIN, OUTPUT);
  pinMode(RIGHT_TRACK_PWM_PIN, OUTPUT);

  pinMode(R_FLAG_VPRED_PIN, OUTPUT);
  pinMode(R_FLAG_VZAD_PIN, OUTPUT);
  pinMode(L_FLAG_VPRED_PIN, OUTPUT);
  pinMode(L_FLAG_VZAD_PIN, OUTPUT);
  //Set the pins for each PWM signal 

  //Configure the NRF24 module
  ResetData();
  radio.begin();
  radio.openReadingPipe(1,pipeIn);
  radio.startListening(); //start the radio comunication for receiver | Alıcı olarak sinyal iletişimi başlatılıyor
}

 unsigned long lastRecvTime = 0;
 void recvData() {
    while ( radio.available() ) {
    radio.read(&data, sizeof(Signal));
    lastRecvTime = millis();  
  }
}

int rightFrontTrack(int right_track_final) {
  RT_FRONT = right_track_final - 255; // 500 - 259 = cca 0 // START
  RT_FRONT_FINAL = 0;
  if (RT_FRONT > 100 ) {  //  stabilizacia malych hodnot
    RT_FRONT_FINAL = RT_FRONT;
  } else {
    RT_FRONT_FINAL = 0;
  }
  return RT_FRONT_FINAL;
}

int rightBackTrack(int right_track_final) {
  RT_BACK = 255 - right_track_final;
  if (RT_BACK > 100) {
    RT_BACK_FINAL = RT_BACK;
  } else {
    RT_BACK_FINAL = 0;
  }
  return RT_BACK_FINAL;
}

int leftFrontTrack(int left_track_final) {
  LT_FRONT = left_track_final - 255; // 500 - 259 = cca 0 // START
  LT_FRONT_FINAL = 0;
  if (LT_FRONT > 100 ) {  //  stabilizacia malych hodnot
    LT_FRONT_FINAL = LT_FRONT;
  } else {
    LT_FRONT_FINAL = 0;
  }
  return LT_FRONT_FINAL;
}

int leftBackTrack(int left_track_final) {
  LT_BACK = 255 - left_track_final;
  LT_BACK_FINAL = 0;
  if (LT_BACK > 100) {
    LT_BACK_FINAL = LT_BACK;
  } else {
    LT_BACK_FINAL = 0;
  }
  return LT_BACK_FINAL;
}

void writeLcdRightTrack( int vpred, int vzad) {
    lcd.setCursor(0, 0);         
    lcd.print("P="); 
    lcd.setCursor(2, 0);   
    lcd.print(right_track_final);  

    lcd.setCursor(6, 0);         
    lcd.print("P_d="); 
    lcd.setCursor(10, 0);   
    lcd.print(vpred);  

    lcd.setCursor(15, 0);         
    lcd.print("VP="); 
    lcd.setCursor(18 ,0);   
    lcd.print(R_FLAG_VPRED);  

    lcd.setCursor(6, 1);         
    lcd.print("P_z="); 
    lcd.setCursor(10, 1);   
    lcd.print(vzad); 

    lcd.setCursor(15, 1);         
    lcd.print("VZ="); 
    lcd.setCursor(18 ,1);   
    lcd.print(R_FLAG_VZAD);  

    delay(15);
}

void writeLcdLeftTrack( int vpred, int vzad) {
    lcd.setCursor(0, 2);         
    lcd.print("L="); 
    lcd.setCursor(2, 2);   
    lcd.print(left_track_final);  

    lcd.setCursor(6, 2);         
    lcd.print("L_d="); 
    lcd.setCursor(10, 2);   
    lcd.print(vpred);  

    lcd.setCursor(15, 2);         
    lcd.print("VP="); 
    lcd.setCursor(18 ,2);   
    lcd.print(L_FLAG_VPRED);  

    lcd.setCursor(6, 3);         
    lcd.print("L_z="); 
    lcd.setCursor(10, 3);   
    lcd.print(vzad); 

    lcd.setCursor(15, 3);         
    lcd.print("VZ="); 
    lcd.setCursor(18 ,3);   
    lcd.print(L_FLAG_VZAD);  

    delay(15);
}



void loop() {
  recvData();
  unsigned long now = millis();
  if ( now - lastRecvTime > 1000 ) {
    ResetData(); // Signal lost..
  }

  right_track_incoming = map(data.rightTracks, 0, 255, 1000, 2000); 
  right_track_final = map(right_track_incoming, 1000, 2000, 0, 510);

  left_track_incoming = map(data.leftTracks, 0, 255, 1000, 2000); 
  left_track_final = map(left_track_incoming, 1000, 2000, 0, 510);


  int Rvpred = rightFrontTrack(right_track_final);
  int Rvzad = rightBackTrack(right_track_final);

  int Lvpred = leftFrontTrack(left_track_final);
  int Lvzad = leftBackTrack(left_track_final);

  if (Rvpred > 100 && Rvzad < 100) {
    R_FLAG_VPRED = 1;
    R_FLAG_VZAD = 0;
  } else if (Rvpred < 100 && Rvzad > 100) {
    R_FLAG_VPRED = 0;
    R_FLAG_VZAD = 1;
  }   

  if (Lvpred > 100 && Lvzad < 100) {
    L_FLAG_VPRED = 1;
    L_FLAG_VZAD = 0;
  } else if (Lvpred < 100 && Lvzad > 100) {
    L_FLAG_VPRED = 0;
    L_FLAG_VZAD = 1;
  }   

  writeLcdRightTrack(Rvpred, Rvzad);
  writeLcdLeftTrack(Lvpred, Lvzad);

  if (R_FLAG_VPRED == 1 && R_FLAG_VZAD == 0) {
    digitalWrite(R_FLAG_VPRED_PIN, HIGH);
    digitalWrite(R_FLAG_VZAD_PIN, LOW);
    analogWrite(RIGHT_TRACK_PWM_PIN, Rvpred); 
  } 

  if (R_FLAG_VPRED == 0 && R_FLAG_VZAD == 1) {
    digitalWrite(R_FLAG_VPRED_PIN, LOW);
    digitalWrite(R_FLAG_VZAD_PIN, HIGH);
    analogWrite(RIGHT_TRACK_PWM_PIN, Rvzad); 
  }

  if (L_FLAG_VPRED == 1 && L_FLAG_VZAD == 0) {
    digitalWrite(L_FLAG_VPRED_PIN, HIGH);
    digitalWrite(L_FLAG_VZAD_PIN, LOW);
    analogWrite(LEFT_TRACK_PWM_PIN, Lvpred); 
  }

  if (L_FLAG_VPRED == 0 && L_FLAG_VZAD == 1) {
    digitalWrite(L_FLAG_VPRED_PIN, LOW);
    digitalWrite(L_FLAG_VZAD_PIN, HIGH);
    analogWrite(LEFT_TRACK_PWM_PIN, Lvzad); 
  }




 
}
