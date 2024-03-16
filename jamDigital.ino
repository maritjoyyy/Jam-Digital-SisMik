#define button1 3
#define button2 8
#define button3 9
#define button4 10
#define buzzer 7

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// setup stopwatch
double i = 0;
double a = millis(); // waktu awal
double c ;
int m = 0;
int ms = 0;
int h = 0;
unsigned long elapsed;

LiquidCrystal_PCF8574 lcd(0x27);

// variabel waktu sebenarnya
int currentHrs = 10;
int currentMin = 39;
int currentSec = 0;

// variabel alarm
int alarmHrs = 10;
int alarmMin = 40;

// state default
int state = 0;
int nState = 3; // jumlah state total
int stateAlarm = 0; //nyalain alarm atau nggak

int currentStateAlarmButton, currentStateButton1, currentStateButton2, currentStateButton3;
int lastStateAlarmButton = HIGH, lastStateButton1 = HIGH, lastStateButton2 = HIGH, lastStateButton3 = HIGH;

void setup() {
  
  // setup LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setBacklight(200);

  // setup pin
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);

  // setup interrupt
  cli(); // Disable interrupts
  TCCR1A = 0; // Clear Timer1 control registers
  TCCR1B = 0;
  TCNT1 = 0; // Initialize counter value to 0
  OCR1A = 15624; // Set the compare match value (1 Hz)
  TCCR1B |= (1 << WGM12); // Configure timer for CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // Set prescaler to 1024
  TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare match interrupt
  sei(); // Enable interrupts

  // Setup interrupt button
  attachInterrupt(digitalPinToInterrupt(button1), gantiState, FALLING);
  Serial.begin(9600);
}

// Fungsi Interrupt untuk Menghitung Jam
ISR(TIMER1_COMPA_vect) {
  cli(); // Nonaktifkan interrupt sementara

  // Inkrementasi detik
  currentSec++;
  if (currentSec >= 60) {
    currentSec = 0;
    currentMin++;
    if (currentMin >= 60) {
      currentMin = 0;
      currentHrs++;
      if (currentHrs >= 24) {
        currentHrs = 0;
      }
    }
  }

  sei(); // Aktifkan kembali interrupt
}

// Fungsi Interrupt untuk Mengganti State
void gantiState() {
  // Mendeteksi falling edge
  if (digitalRead(button1) == LOW) {
    delay(50); // Penundaan untuk mengatasi bouncing
    if (digitalRead(button1) == LOW) {
      state++;
      if (state >= 5) {
        state = 0;
      }
      Serial.print("Interrupt: ");
      Serial.println(state);
    }
  }
}

// Fungsi Looping Utama
void loop() {
  // Jika stateAlarm aktif dan waktu sesuai dengan alarm, aktifkan buzzer
  if (stateAlarm == 1 && currentHrs == alarmHrs && currentMin == alarmMin) {
    digitalWrite(buzzer, HIGH);
  }
  else if ((stateAlarm == 0) || (currentHrs != alarmHrs) || (currentMin != alarmMin)) {
    digitalWrite(buzzer, LOW);
  }

  // Tampilkan jam
  if (state == 0) {
    tampilkanJam();
  }

  // Setting jam
  else if (state == 1) {
    settingJam();
  }

  // Setting alarm
  else if (state == 2) {
    settingAlarm();
  }

  // Stopwatch
  else if (state == 3) {
    stopwatch();
  }

  // Atur nyala atau mati alarm
  else if (state == 4) {
    nyalaAlarm();
  }
}

void tampilkanJam() {
  //lcd.clear();
  lcd.setCursor(0 ,1);
  lcd.print("Jam Digital");

  lcd.setCursor(8, 0);
  lcd.print("   ");

  if (currentHrs<10){
    lcd.setCursor(0,0);
    lcd.print(0);
    lcd.setCursor(1,0);
    lcd.print(currentHrs);
  }
  else {
    lcd.setCursor(0,0);
    lcd.print(currentHrs);
  }

  lcd.setCursor(2,0);
  lcd.print(":");

  if (currentMin<10){
    lcd.setCursor(3,0);
    lcd.print(0);
    lcd.setCursor(4,0);
    lcd.print(currentMin);
  }
  else {
    lcd.setCursor(3,0);
    lcd.print(currentMin);
  }
  
  lcd.setCursor(5,0);
  lcd.print(":");

  if (currentSec<10){
    lcd.setCursor(6,0);
    lcd.print(0);
    lcd.setCursor(7,0);
    lcd.print(currentSec);
  }
  else {
    lcd.setCursor(6,0);
    lcd.print(currentSec);
  }
}

void settingJam() {
  //lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Setting Jam");

  // Kalau button2 dipencet, current jam berubah
  currentStateButton2 = digitalRead(button2);
  if (currentStateButton2 == LOW && lastStateButton2 == HIGH) {
    currentHrs = (currentHrs + 1) % 24;
    lcd.setCursor(0, 0); // Set posisi kursor LCD untuk menampilkan jam
    if (currentHrs < 10) {
      lcd.print("0"); // Tambahkan angka 0 di depan jika jam kurang dari 10
    }
    lcd.print(currentHrs); // Tampilkan jam
  }
  lastStateButton2 = currentStateButton2;

  // Kalau button3 dipencet, current menit berubah
  currentStateButton3 = digitalRead(button3);
  if (currentStateButton3 == LOW && lastStateButton3 == HIGH) {
    currentMin = (currentMin + 1) % 60;
    lcd.setCursor(3, 0); // Set posisi kursor LCD untuk menampilkan menit
    if (currentMin < 10) {
      lcd.print("0"); // Tambahkan angka 0 di depan jika menit kurang dari 10
    }
    lcd.print(currentMin); // Tampilkan menit
  }
  lastStateButton3 = currentStateButton3;
}

void settingAlarm() {
  //lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Setting Alarm");

  // Print pengaturan alarm terakhir
  // Jam alarm
  lcd.setCursor(0, 0); // Set posisi kursor LCD untuk menampilkan jam
  if (alarmHrs < 10) {
    lcd.print("0"); // Tambahkan angka 0 di depan jika jam kurang dari 10
  }
  lcd.print(alarmHrs); // Tampilkan jam
  // Menit alarm
  lcd.setCursor(3, 0); // Set posisi kursor LCD untuk menampilkan menit
  if (alarmMin < 10) {
    lcd.print("0"); // Tambahkan angka 0 di depan jika menit kurang dari 10
  }
  lcd.print(alarmMin); // Tampilkan menit

  // Baca input button2, kalau button2 dipencet, jam alrm nya yang berubah
  currentStateButton2 = digitalRead(button2);
  if (currentStateButton2 == LOW && lastStateButton2 == HIGH) {
    alarmHrs = (alarmHrs + 1) % 24;
    lcd.setCursor(0, 0); // Set posisi kursor LCD untuk menampilkan jam
    if (alarmHrs < 10) {
      lcd.print("0"); // Tambahkan angka 0 di depan jika jam kurang dari 10
    }
    lcd.print(alarmHrs); // Tampilkan jam
  }
  lastStateButton2 = currentStateButton2;

  // Baca input button3, kalau button3 dipencet, menit alrm nya yang berubah
  currentStateButton3 = digitalRead(button3);
  if (currentStateButton3 == LOW && lastStateButton3 == HIGH) {
    alarmMin = (alarmMin + 1) % 60;
    lcd.setCursor(3, 0); // Set posisi kursor LCD untuk menampilkan menit
    if (alarmMin < 10) {
      lcd.print("0"); // Tambahkan angka 0 di depan jika menit kurang dari 10
    }
    lcd.print(alarmMin); // Tampilkan menit
  }
  lastStateButton3 = currentStateButton3;
}

void stopwatch() {
  lcd.clear();
    lcd.print("press start");
    delay(100);

    currentStateButton2 = digitalRead(button2);
    if(currentStateButton2 == LOW && lastStateButton2 == HIGH) // menunggu push button start ditekan untuk mulai
    {
        // pengukuran waktu dimulai
        lcd.clear();
        a = millis();

        // loop terus memperbarui layar dengan waktu yang berlalu
        while(digitalRead(button3) == HIGH){
          c = millis();

          elapsed = (c - a); // Waktu yang telah berlalu dalam detik
          h = elapsed / 3600000; // Hitung jam
          m = (elapsed % 3600000) / 60000; // Hitung menit
          i = (elapsed % 60000) / 1000; // Hitung detik
          ms = (elapsed % 1000); // Hitung milidetik

          if (h<10){
            lcd.setCursor(0,0);
            lcd.print("0");
            lcd.setCursor(1,0);
            lcd.print(h);
          }
          else{
            lcd.setCursor(0,0);
            lcd.print(h);
          }
          lcd.setCursor(2,0);
          lcd.print(":");

          if (m<10){
            lcd.setCursor(3,0);
            lcd.print("0");
            lcd.setCursor(4,0);
            lcd.print(m);
          }
          else{
            lcd.setCursor(3,0);
            lcd.print(m);
          }
          lcd.setCursor(5,0);
          lcd.print(":");

          if (i<10){
            lcd.setCursor(6,0);
            lcd.print("0");
            lcd.setCursor(7,0);
            lcd.print(i);
          }
          else{
            lcd.setCursor(6,0);
            lcd.print(i);
          }
          lcd.setCursor(8,0);
          lcd.print(".");

          if (ms<10){
            lcd.setCursor(9,0);
            lcd.print("0");
            lcd.setCursor(10,0);
            lcd.print(ms);
          }
          else{
            lcd.setCursor(9,0);
            lcd.print(ms);
          }
        }
      

      // push button stop ditekan, loop berhenti dan menunggu untuk dimulai kembali
      if(digitalRead(button3) == LOW)
      {
        while(digitalRead(button2) == HIGH)
        {
          if (h<10){
            lcd.setCursor(0,0);
            lcd.print("0");
            lcd.setCursor(1,0);
            lcd.print(h);
          }
          else{
            lcd.setCursor(0,0);
            lcd.print(h);
          }
          lcd.setCursor(2,0);
          lcd.print(":");

          if (m<10){
            lcd.setCursor(3,0);
            lcd.print("0");
            lcd.setCursor(4,0);
            lcd.print(m);
          }
          else{
            lcd.setCursor(3,0);
            lcd.print(m);
          }
          lcd.setCursor(5,0);
          lcd.print(":");

          if (i<10){
            lcd.setCursor(6,0);
            lcd.print("0");
            lcd.setCursor(7,0);
            lcd.print(i);
          }
          else{
            lcd.setCursor(6,0);
            lcd.print(i);
          }
          lcd.setCursor(8,0);
          lcd.print(".");

          if (ms<10){
            lcd.setCursor(9,0);
            lcd.print("0");
            lcd.setCursor(10,0);
            lcd.print(ms);
          }
          else{
            lcd.setCursor(9,0);
            lcd.print(ms);
          }
          
          lcd.setCursor(0,0);
          delay(100);
        }
      }
    }
    lastStateButton2 = currentStateButton2;
}

void nyalaAlarm() {
  //lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Alarm");

  if (stateAlarm == 0) {
    lcd.setCursor(6, 0);
    lcd.print("OFF  ");
  }
  else if (stateAlarm == 1) {
    lcd.setCursor(6,0);
    lcd.print("ON   ");
  }

  // Baca input button2
  currentStateButton2 = digitalRead(button2);
  if (currentStateButton2 == LOW && lastStateButton2 == HIGH) {
    stateAlarm = 1;     // Button 2 untuk menyalakan alarm
  }
  lastStateButton2 = currentStateButton2;

  // Baca input button3
  currentStateButton3 = digitalRead(button3);
  if (currentStateButton3 == LOW && lastStateButton3 == HIGH) {
    stateAlarm = 0;     // Button 3 untuk mematikan alarm
  }
  lastStateButton3 = currentStateButton3;
}