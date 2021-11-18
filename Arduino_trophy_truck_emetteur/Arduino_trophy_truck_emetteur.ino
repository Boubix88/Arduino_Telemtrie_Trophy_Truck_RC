#include <VirtualWire.h>
#include <VirtualWire_Config.h>
#include <math.h>
#include <arduino.h>

//Capteur 1: D2
//Capteur 2: D3

#define PIN_NTC A0
#define PIN_TENSION A1

float value1, value2=0;
float rev1, rev2=0;
int rpm1, rpm2;
int oldtime1, oldtime2=0;
int time1, time2;
int vitesse;
float vin = 0.0;
int TensionBatterie = 0;
int pourcentageBatterie = 0;

const int R1 = 2;
const int R2 = 3;
double Rref = 10000.0; //Résistance de référence à 25°C
double V_IN = 5.0; //Alimentation électrique
double celsius;
 
//Information de la thermistance
double A_1 = 3.354016E-3;
double B_1 = 2.569850E-4;
double C_1 = 2.620131E-6;
double D_1 = 6.383091E-8;

void isr2(){
  rev2++;
}

void isr1()
{
  rev1++;
}

double SteinhartHart(double R)
{
  //Division de l'équation en 4 parties. La premiere est 
  //uniquement A1
  double equationB1 = B_1 * log(R/Rref);
  double equationC1 = C_1 * pow(log(R/Rref), 2);
  double equationD1 = D_1 * pow(log(R/Rref), 3);
  double equation = A_1 + equationB1 + equationC1 + equationD1;
  return pow(equation, -1);
}

void temperatureCalcul(){
  //Calcul de la tension sur la borne analogique
  double valeurAnalog = analogRead(PIN_NTC);
  double V =  valeurAnalog / 1024 * V_IN;
 
  //Calcul de la résistance de la thermistance
  double Rth = (Rref * V ) / (V_IN - V);
 
  //Calcul de la température en kelvin( Steinhart and Hart)
  double kelvin = SteinhartHart(Rth);
  celsius = kelvin - 273.15; //Conversion en celsius
}

void tension(){
   //BATTERIE -- Lecture de la tension de la batterie sur le Pin A1
   TensionBatterie = analogRead(PIN_TENSION);
   vin = (TensionBatterie*12.25)/1000; 
   if (vin<0.09) {
      vin=0.0;  //Déclaration pour annuler la lecture indésirable !
   }
   pourcentageBatterie = ((vin - 11.1)/(12.2-11.1))*100;

  if (pourcentageBatterie > 100){
    pourcentageBatterie = 100;
  }else if(pourcentageBatterie < 0){
    pourcentageBatterie = 0;
  }
}

void setup ()
{
  Serial.begin(9600);
  vw_setup(500);
  attachInterrupt(0, isr1, RISING);
  attachInterrupt(1, isr2, RISING);
}
 
void loop ()
{
  int valeur[4];
  temperatureCalcul();
  //Capteur 1 : RPM
  detachInterrupt(0);           //detaches the interrupt
  time1=millis()-oldtime1;        //finds the time 
  rpm1=(rev1/time1)*60000;         //calculates rpm
  oldtime1=millis();             //saves the current time
  rev1=0;
  /*Serial.print("RPM : ");
  Serial.println(rpm1, DEC);*/
  attachInterrupt(0, isr1, RISING);

  //Capteur 2 : Vitesse
  detachInterrupt(1);           //detaches the interrupt
  time2=millis()-oldtime2;        //finds the time 
  rpm2=(rev2/time2)*60000;         //calculates rpm
  oldtime2=millis();             //saves the current time
  rev2=0;
  rpm2 = rpm2/9;
  vitesse = 0.12*PI*0.15*rpm2;

  attachInterrupt(1, isr2, RISING);
  valeur[0] = rpm1;
  valeur[1] = vitesse;
  valeur[2] = celsius;
  tension();
  valeur[3] = pourcentageBatterie;

  vw_send((byte*)&valeur,sizeof(valeur)); 
  if (vw_tx_active() == true){
    Serial.println("Transmission effectuée");
  }
  Serial.print("Tension batterie : ");
  Serial.println(vin);
  
  Serial.print("Temperature : ");
  Serial.println(valeur[2]);

  Serial.print("RPM : ");
  Serial.println(valeur[0]);

  Serial.print("Vitesse : ");
  Serial.println(valeur[1]);
  vw_wait_tx();
}
