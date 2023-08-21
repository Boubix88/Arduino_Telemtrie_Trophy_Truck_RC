#include <SPI.h>
#include <RF24.h>
#include <math.h>
#include <arduino.h>

//Capteur 1: D2
//Capteur 2: D3

#define PIN_TENSION A1
#define PIN_SWITCH 5 //Switch demarreur
#define PIN_CONTACT 4
#define PIN_DEMARREUR 7
#define PIN_VENTILO_MOTEUR 8
#define PIN_VENTILO_ELECTRONIQUE 9
#define LIMITE_TEMPERATURE_MOTEUR 50
#define LIMITE_TEMPERATURE_ELECTRONIQUE 30

#define PIN_CE  10
#define PIN_CSN 6

#define tunnel  "PIPE1"

RF24 radio(PIN_CE, PIN_CSN);

const byte adresse[6] = tunnel;  // Adresse du récepteur
unsigned long packetsSent = 0;

//Rpm et Vitesse
float rev1, rev2 = 0;
int rpm2;
int oldtime1, oldtime2=0;
int time1, time2;

//Switch
bool switchStarter = false;
bool switchContact = false;
int channel4;

const int R1 = 2;
const int R2 = 3;
const double Rref = 10000.0; //Résistance de référence à 25°C
const double V_IN = 5.0; //Alimentation électrique
 
//Information de la thermistance
const double A_1 = 3.354016E-3;
const double B_1 = 2.569850E-4;
const double C_1 = 2.620131E-6;
const double D_1 = 6.383091E-8;

void isr2(){
  rev2++;
}

void isr1()
{
  rev1++;
}

double SteinhartHart(double R){
  //Division de l'équation en 4 parties. La premiere est 
  //uniquement A1
  double equationB1 = B_1 * log(R/Rref);
  double equationC1 = C_1 * pow(log(R/Rref), 2);
  double equationD1 = D_1 * pow(log(R/Rref), 3);
  double equation = A_1 + equationB1 + equationC1 + equationD1;
  return pow(equation, -1);
}

double temperatureCalcul(int pin){
  //Calcul de la tension sur la borne analogique
  double valeurAnalog = analogRead(pin);
  double V =  valeurAnalog / 1024 * V_IN;
 
  //Calcul de la résistance de la thermistance
  double Rth = (Rref * V ) / (V_IN - V);
 
  //Calcul de la température en kelvin( Steinhart and Hart)
  double kelvin = SteinhartHart(Rth);
  return kelvin - 273.15; //Conversion en celsius
}

int calculTension(){
  int TensionBatterie = 0;
  float vin = 0.0;
  int res = 0;
  //Calcul de la tension de la batterie
  TensionBatterie = analogRead(PIN_TENSION);
  vin = (TensionBatterie * 12.6) / 1000; 
  if (vin < 0.09) {
    vin = 0.0;  //Déclaration pour annuler la lecture indésirable !
  }
  res = ((vin - 11.1) / (12.6 - 11.1)) * 100;

  if (res > 100){
    res = 100;
  }else if(res < 0){
    res = 0;
  }
  return res;
}

int calculRpm(){
  detachInterrupt(0); //detaches the interrupt
  time1 = millis() - oldtime1; //finds the time 
  oldtime1 = millis(); //saves the current time
  rev1 = 0;
  attachInterrupt(0, isr1, RISING);
  return (rev1 / time1) * 60000;; //Calcul des rpm
}

int calculVitesse(){
  detachInterrupt(1); //detaches the interrupt
  time2 = millis() - oldtime2; //finds the time 
  rpm2 = (rev2 / time2) * 60000; //calculates rpm
  oldtime2 = millis(); //saves the current time
  rev2 = 0;
  rpm2 = rpm2/9;
  attachInterrupt(1, isr2, RISING);
  return 0.12 * PI * 0.15 * rpm2;
}

void checkSwitch(){
  channel4 = pulseIn(PIN_SWITCH, HIGH, 25000); // Lire ch1 (25 ms)
  if (channel4 > 1500 && channel4 < 1700){ //Si position switch 2 demarreur => on, contact => on
    switchStarter = true;
  } else if (channel4 > 1350 && channel4 < 1500){ //Switch demarreur position 1 => demarreur => off, contact => on
    switchStarter = false;
    switchContact = false;
  } else if (channel4 > 1100 && channel4 < 1350){ //Switch demarreur position 0 => tout => off
    switchStarter = false;
    switchContact = true;
  }
  if (switchStarter){
    digitalWrite(PIN_DEMARREUR, HIGH); //5v sur D7 => switch demarreur activé
  } else {
    digitalWrite(PIN_DEMARREUR, LOW);
  }
  if (switchContact){
    digitalWrite(PIN_CONTACT, HIGH); //5v sur D4 => switch contact activé 
  } else {
    digitalWrite(PIN_CONTACT, LOW);
  }
}

void checkFan(double tempMoteur, double tempElec){
  //Serial.print(tempElec);
  if (tempMoteur > LIMITE_TEMPERATURE_MOTEUR){
    digitalWrite(PIN_VENTILO_MOTEUR, HIGH);
  } else {
    digitalWrite(PIN_VENTILO_MOTEUR, LOW);
  }
  if (tempElec > LIMITE_TEMPERATURE_ELECTRONIQUE){
    digitalWrite(PIN_VENTILO_ELECTRONIQUE, HIGH);
  } else {
    digitalWrite(PIN_VENTILO_ELECTRONIQUE, LOW);
  }
}

void setup ()
{
  Serial.begin(9600);

  // Initialisation du module RF24
  radio.begin();
  radio.openWritingPipe(adresse);
  radio.setPALevel(RF24_PA_HIGH);
  radio.stopListening(); 

  attachInterrupt(0, isr1, RISING);
  attachInterrupt(1, isr2, RISING);
  pinMode(PIN_SWITCH, INPUT);  // ch4 sur Arduino pin5 switch
  pinMode(PIN_CONTACT, OUTPUT);
  pinMode(PIN_DEMARREUR, OUTPUT);
  pinMode(PIN_VENTILO_MOTEUR, OUTPUT);
  pinMode(PIN_VENTILO_ELECTRONIQUE, OUTPUT);
  switchContact = true;
}
 
void loop ()
{
  int valeur[9];
  checkSwitch();

  //Initialisation des données à envoyer
  int rpm = calculRpm();
  int vitesse = calculVitesse();
  double tempMoteur = temperatureCalcul(A0);
  double tempElectronique = temperatureCalcul(A2);
  int pourcentageBatterie = calculTension();

  checkFan(tempMoteur, tempElectronique);

  //Copie des données dans valeur
  valeur[0] = rpm;
  valeur[1] = vitesse;
  valeur[2] = tempMoteur;
  valeur[3] = pourcentageBatterie;
  valeur[6] = tempElectronique;

  switch (switchContact){
    case false:
      valeur[4] = 1;
      break;
    default :
      valeur[4] = 0;
      break;
  }

  switch (switchStarter){
    case true:
      valeur[5] = 1;
      break;
    default :
      valeur[5] = 0;
      break;
  }

  // Envoyer un paquet de contrôle tous les 10 envois
  if (packetsSent % 10 == 0) {
    Serial.println("Envoi d'un packet de controle");
    valeur[7] = packetsSent; // Contenu du paquet de contrôle : nombre de paquets envoyés
    valeur[8] = 500;

    packetsSent = 0; // On remet à 0;
  } else {
    valeur[7] = 0;
    valeur[8] = 0;
  }

  // Envoi des données via RF24
  //radio.write(&valeur, sizeof(valeur));
  if (radio.write(&valeur, sizeof(valeur))) {
    Serial.println("Données envoyées avec succès !");
    packetsSent++; // Incrémente le compteur de paquets envoyés
    // Attendre la réponse du récepteur (acknowledgment)
    if (radio.isAckPayloadAvailable()) {
      byte ackPayloadSize = radio.getDynamicPayloadSize();
      byte ackPayload[ackPayloadSize];
      radio.read(&ackPayload, ackPayloadSize);
      Serial.print("Réponse du récepteur : ");
      /*for (byte i = 0; i < ackPayloadSize; i++) {
        Serial.print(ackPayload[i]);
        Serial.print(" ");
      }
      Serial.println();*/
    } else {
      Serial.println("Pas de réponse du récepteur");
    }
  } else {
    Serial.println("Erreur lors de l'envoi des données");
  }
}
