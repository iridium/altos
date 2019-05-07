#include "Seeed_BME280.h"
#include <Wire.h>
#include <Servo.h>

/* TODO : 

  - Enregistrement des grandeurs physiques sur carte SD
  
*/
// Constantes
const float pi = 3.14159;
const int Servo_open = 90;
const int Servo_closed = 180;

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
int t;

// Variable filtre/derivée 
float Te = 10e-3;
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
  //Serial.begin(9600);
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
  
  // Calcul de vz
  alta = getAlt();
  delay(Te*1000);
  altb = getAlt();
  vz = deriv(Te, alta, altb);
  
  // Filtrage de vz
  vz_filtre = fpb(Tau, Te, vz, vzfa);
  vzfa = vz_filtre;

  //  ^
  //  | A implémenter dans une fonction
  
  /*
  // Affichage des grandeurs
  Serial.print(alt);
  Serial.print(",");
  Serial.print(vz);
  Serial.print(",");
  Serial.print(vz_filtre);
  Serial.println();
  
  temp = bme280.getTemperature();
  Serial.println("Temp : "+String(temp)+" °C");
  pression = bme280.getPressure();
  Serial.println("Pression : "+String(pression)+" Pa");
  alt = bme280.calcAltitude(pression);
  Serial.println("Altitude : "+String(alt-alt0) + " m");
  */
  
  //Détection de la retombé
  if(vz_filtre < 0 && !retomb) {
    t = millis();
    retomb = true;
  }
  
  if(retomb) {
    float pos = 1; // Incrémenté à chaque fois que vz > 0
    float neg = 1; //        "      "      "       vz < 0
    while(millis() - t < 3000) {

      // Calcul de vz
      alta = getAlt();
      delay(Te*1000);
      altb = getAlt();
      vz = deriv(Te, alta, altb); 

      // Filtrage de vz
      vz_filtre = fpb(Tau, Te, vz, vzfa);
      vzfa = vz_filtre;

      //  ^ 
      //  | Remplacer par la fonction lorsqu'elle sera créee
      
      if(vz_filtre < 0) {
        neg++;
      }else{
        pos++;
      }
    }if (neg/(neg+pos) > 0.5) { // Confirmation de la retombé
      // Déploiement du parachute
      while(1){
        delay(100);
        digitalWrite(buz_p, HIGH);
        deploy_para();
        digitalWrite(buz_p, LOW).
      }
      
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

float deriv(float te, float sna, float sn) { // Approximation de la dérivée numériquement
  return (sn-sna)*te;
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

