#include "Seeed_BME280.h"
#include <Wire.h>
#include <Servo.h>

/* TODO : 

  - Enregistrement des grandeurs physique sur carte SD
  - Buzzer lors de la retombé
  
*/

// Capteur(s)/Actionneur(s)
BME280 bme280; // Capteurs Temp/Pression
Servo spara; // Servo parachute

// Grandeurs physiques
float pression;
float altitude;
float temp;
float alt;
float alt0;
float vz; //vitesse verticale
float vz_filtre;

// Logique
bool retomb = false;
bool confirm = false;
int t;

// Variable filtre/derivée 
float Te = 10e-3;
float Sna; // Alt((n-1)Te)
float Sn;  // Alt(nTe)
float vzfa;
float f0 = 1; //Hz
float pi = 3.14159;
float Tau = 1/(2*pi*f0);


// Pins
int l[] = {2,3,4}; // Pin led RGB {r,g,b}
int buz_p;
int servo_pin;

void setup() {
  pinMode(l[0], OUTPUT);
  pinMode(l[1], OUTPUT);
  pinMode(l[2], OUTPUT);
  Serial.begin(9600);
  if (!bme280.init()) {
    led(l,255,0,0);
    delay(1000);
    led(l,0,0,0);
    while (1);
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
  //pression = bme280.getPressure();
  Sna = getAlt();
  delay(Te*1000);
  alt = getAlt();
  Sn = alt;
  vz = deriv(Te, Sna, Sn); 
  vz_filtre = fpb(Tau, Te, vz, vzfa);
  vzfa = vz_filtre;
  Serial.print(alt);
  Serial.print(",");
  Serial.print(vz);
  Serial.print(",");
  Serial.print(vz_filtre);
  Serial.println();
  /*temp = bme280.getTemperature();
  Serial.println("Temp : "+String(temp)+" °C");
  pression = bme280.getPressure();
  Serial.println("Pression : "+String(pression)+" Pa");
  alt = bme280.calcAltitude(pression);
  Serial.println("Altitude : "+String(alt-alt0) + " m");
  delay(1000);*/
  //Détection de la retombé
  if(vz_filtre < 0 && !retomb) {
    t = millis();
    retomb = true;
  }
  
  if(retomb) {
    float pos = 1;
    float neg = 1;
    while(millis() - t < 3) {
      Sna = getAlt();
      delay(Te*1000);
      alt = getAlt();
      Sn = alt;
      vz = deriv(Te, Sna, Sn); 
      vz_filtre = fpb(Tau, Te, vz, vzfa);
      vzfa = vz_filtre;
      if(vz_filtre < 0) {
        neg++;
      }else{
        pos++;
      }
    }if (neg/(neg+pos) > 0.5) {
      confirm = true;
    }
    if(confirm) {
      // DECLENCHEMENT PARACHUTE
    }else{
      pos = 0;
      neg = 0;
      t = 0;
      retomb = false;
    }
  }
}

float getAlt() {
  float p = bme280.getPressure();
  return bme280.calcAltitude(p)-alt0;
}

float deriv(float te, float sna, float sn) { 
  return (sn-sna)*te;
}

float fpb(float tau,float te,float en ,float sna) {  // Filtre passe bas d'ordre 1
  return (te*en + tau*sna)/(te+tau);
}

void deploy_para() {
   led(l, 255, 180, 0);
   delay(100);
   led(l,0,0,0);
  //todo 
}

void led(int lp[], int r, int g, int b) {
   analogWrite(lp[0], r);
   analogWrite(lp[1], g);
   analogWrite(lp[2], b);
}

