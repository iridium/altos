#include "Seeed_BME280.h"
#include <Wire.h>
#include <Servo.h>
#include <MPU6050.h>
#include "SD.h"

/* TODO : 
 */

// Constantes
const float pi = 3.14159;
const int Servo_open = 90;
const int Servo_closed = 180;

// Capteur(s)/Actionneur(s)
BME280 bme280; // Capteurs Temp/Pression
Servo spara; // Servo parachute
MPU6050 mpu; // Accéléromètre 

// Grandeurs physiques
int16_t ax, ay, az;
float pression;
float altitude;
float temp;
float alt;
float alt0;
float vz; //vitesse verticale
float vz_filtre;

// SD
int SDPin = 8;
File dataFile;

// Logique
bool retomb = false;
bool aDecol = false;
float t;
bool paradep = false;
String a;

// Variable filtre/derivée 
float Te = 100e-3;
float teTest;
float alta; // Alt((n-1)Te)
float altb; // Alt(nTe)
float vzfa; // vitesse verticale fitltrée Vz((n-1)Te))
float f0 = 1; //Hz
float Tau = 1/(2*pi*f0);

// Pins
int l[] = {2,3,4}; // Pin led RGB {r,g,b}
int buz_p = 10; // Passive buzzer
int servo_pin = 9;

void setup() {
  
  spara.attach(servo_pin);
  spara.write(Servo_closed);
  
  pinMode(l[0], OUTPUT);
  pinMode(l[1], OUTPUT);
  pinMode(l[2], OUTPUT);
  pinMode(buz_p, OUTPUT);

  // INIT-BME
  if (!bme280.init()) {
    led(l,255,0,0);
    delay(1000);
    led(l,0,0,0);
    while (1);
  }

  // INIT-MPU
  mpu.initialize();
  mpu.setFullScaleAccelRange(3); //+- 16g
  mpu.setDLPFMode(2);

  // INIT-SD
  if (!SD.begin(SDPin)) {
    tone(buz_p, 500, 500);
    led(l,255,0,0);
    delay(1000);
    led(l,0,0,0);
  }
  
  led(l,0,255,0);
  delay(200);
  led(l,0,0,0);
  delay(200);
  led(l,0,255,0);
  delay(200);
  led(l,0,0,0);
  alt0 = bme280.calcAltitude(bme280.getPressure());
  
}
void loop() {
  updateData();
  
  if(altb > 5) {
    aDecol = true;
  }
  
  //Détection de la retombé
  if(vz_filtre < 0 && !retomb && aDecol) {
    t = millis();
    retomb = true;
  }
  
  if(retomb) {
      float pos = 1; // Incrémenté à chaque fois que vz > 0
      float neg = 1; //        "      "      "       vz < 0
      while(millis() - t < 1000) {

        updateData();
        
        if(vz_filtre < 0) {
          neg++;
        }else{
          pos++;
      }
      
    }if (neg/(neg+pos) > 0.5) { // Confirmation de la retombé
      // Déploiement du parachute
      while(1){
        tone(buz_p, 1000);
        deploy_para();
        updateData();
      }
      
    }else{
      pos = 0;
      neg = 0;
      t = 0;
      retomb = false;
    }
  }
}


float deriv(float te, float sna, float sn) { // Approximation de la dérivée numériquement
  return (sn-sna)/te;
}

float fpb(float tau,float te,float en ,float sna) {  // Filtre passe bas d'ordre 1
  return (te*en + tau*sna)/(te+tau);
}

void deploy_para() {
   led(l, 255, 180, 0);
   delay(100);
   led(l,0,0,0);
   spara.write(Servo_open);
}

void led(int lp[], int r, int g, int b) {
   analogWrite(lp[0], r);
   analogWrite(lp[1], g);
   analogWrite(lp[2], b);
}

void updateData() {
  dataFile = SD.open("data.txt", FILE_WRITE);
  mpu.getAcceleration(&ax, &ay, &az);
  // Calcul de vz
     
  alta = getAlt();
  delay(Te*1000);
  altb = getAlt();
          
  vz = deriv(Te, alta, altb);
  
  // Filtrage de vz
  delay(Te*1000);
  vz_filtre = fpb(Tau, Te, vz, vzfa);
  vzfa = vz_filtre;

  a = String(ax) + ";" + String(ay) + ";" + String(az);
  // t;alt;vz;vzf;ax;ay;az\r\n
  dataFile.println(String(millis()/1000.0) + ";" + String(altb) + ";" + String(vz) + ";" + String(vz_filtre) + ";" + a + ";" + String(int(paradep)));
  dataFile.close();
}

float getAlt() {
  float p = bme280.getPressure();
  return (bme280.calcAltitude(p)-alt0);
}

