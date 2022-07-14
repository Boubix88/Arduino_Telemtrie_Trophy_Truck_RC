#include <Arduino.h>
#include <U8g2lib.h>
#include <VirtualWire.h>
#include <Wire.h>
#include <stdio.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//Définition des pin d'entrée
#define BUTTON_PIN A1
#define PIN_TENSION A0
// SDK --> A5
// SDA --> A4

#define POLICE_PRINCIPALE u8g2_font_osb18_tn
#define POLICE_VOYANTS u8g2_font_open_iconic_all_2x_t
#define POLICE_CHIFFRES u8g2_font_lubR18_tr
#define POLICE_PETITE u8g2_font_profont11_mr

//Variables pour la reception du signal
int valeur[6];
byte wireless = sizeof(valeur);

float vin = 0.0;

//int timerEcran = 0;
int pourcentageBatterie = 0;

//Pour ecran 3
int rpmMax = 0;
int vitesseMax = 0;
int temperatureMax = 0;
float tempsDeRoulage = 0;

void pre(){
  u8g2.setDrawColor(0);
  u8g2.setFont(POLICE_PETITE);    
  u8g2.setCursor(0,12);
  u8g2.print(F("Trophy Truck"));
}

void afficherRpmVitesse(){
  u8g2.setDrawColor(1);
  u8g2.setFont(POLICE_CHIFFRES);  
  u8g2.setCursor(0, 38);
  u8g2.print(F("RPM"));
  u8g2.setCursor(0, 64);
  u8g2.print(F("V:"));
}

void afficherKmh(){
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_6x12_tr);  
  u8g2.setCursor(95, 64);
  u8g2.print(F("Km/h"));
}

void afficherBatterieTemperature(){
  u8g2.setDrawColor(1);
  u8g2.setFont(POLICE_CHIFFRES);  
  //u8g2.drawGlyph(60, 38, 0x00b0);
  u8g2.drawCircle(72, 21, 2, U8G2_DRAW_ALL);
  u8g2.setCursor(0, 38);
  u8g2.print(F("T:"));
  u8g2.setCursor(75, 38);
  u8g2.print(F("C"));
}

void initAffichage1(){
  pre();
  afficherRpmVitesse();
  afficherKmh();
}

void initAffichage2(){
  pre();
  afficherBatterieTemperature();
}

void initAffichage3(){
  pre();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_6x12_tr);    
  u8g2.setCursor(0, 24);
  u8g2.print(F("rpmMax :"));
  u8g2.setCursor(0, 37);
  u8g2.print(F("vMax :"));
  u8g2.setCursor(0, 50);
  u8g2.print(F("tempMax :"));
  u8g2.setCursor(0, 63);
  u8g2.print(F("tpsRoulage :"));
}

void afficherSignal1(){
  u8g2.setDrawColor(1);
  u8g2.setFont(POLICE_VOYANTS);  

  if (valeur[4] == 1){
    u8g2.drawGlyph(75, 16, 193); //Affiche un icone de clef
  }
  if (valeur[5] == 1){
    u8g2.drawGlyph(94, 16, 235); //Affiche un icone de demarrer
  }
  u8g2.drawGlyph(112, 16, 247); //Affiche un icone de reseau
}

void afficherSignal2(){
  u8g2.setDrawColor(1);
  u8g2.setFont(POLICE_VOYANTS);  
  u8g2.drawGlyph(112, 16, 283);  //Affiche une croix pour le reseau
}

void tension(){
  int TensionBatterie = 0;
  // BATTERIE -- Lecture de la tension de la batterie sur le Pin A0
   TensionBatterie = analogRead(PIN_TENSION);
   vin = (TensionBatterie*4.75)/1000; 
   if (vin < 0.09) {
      vin = 0.0;//Déclaration pour annuler la lecture indésirable !
   }
   pourcentageBatterie = ((vin - 3.7)/(4.12-3.7))*100;

   /*Serial.print(F("Batterie : "));
   Serial.println(vin);*/

  if (pourcentageBatterie > 100){
    pourcentageBatterie = 100;
  }else if(pourcentageBatterie < 0){
    pourcentageBatterie = 0;
  }
}

void afficherTransition(){
  u8g2.setDrawColor(1);
  for (int i = 0; i < 15; i++){
    u8g2.clearBuffer();
    u8g2.drawDisc(128/2, 64/2, i*5, U8G2_DRAW_ALL);
    u8g2.sendBuffer();
  }
}

void afficherEcranDemarrage(){
  u8g2.setDrawColor(1);
  for (int i = 0; i < 25; i++){
    u8g2.clearBuffer();
    u8g2.setFont(POLICE_PETITE);    
    u8g2.setCursor(30,12);
    u8g2.print(F("Trophy Truck"));

    u8g2.setFont(POLICE_VOYANTS); 
    u8g2.drawGlyph(55, 40, 235);

    u8g2.drawFrame(25, 50, i*3, 2);
    u8g2.sendBuffer();
    delay(50);
  }
}

void afficherEcran1(){  
  //On attend max 1s de recevoir un message
  vw_wait_rx_max(600);

  initAffichage1();

  if (vw_get_message((byte*)&valeur, &wireless) == false){
    afficherSignal2();
  }else {
    afficherSignal1();
    char text[5];
    sprintf(text, "%d", valeur[0]);  
    u8g2.setCursor(64,38);
    u8g2.setFont(POLICE_PRINCIPALE);  
    u8g2.print(text);

    sprintf(text, "%d", valeur[1]);  
    u8g2.setCursor(35,64);
    u8g2.print(text);
  }
}

void afficherEcran2(){
  //On attend max 1s de recevoir un message
  vw_wait_rx_max(600);
  tension();
  
  initAffichage2();
  
  afficherBatterie();

  if (vw_get_message((byte*)&valeur, &wireless) == false){
    afficherSignal2();
  } else {
    afficherSignal1();
    u8g2.setFont(POLICE_PRINCIPALE);
  
    char text[3];
    sprintf(text, "%d", valeur[2]); 
    u8g2.setCursor(30,38); 
    u8g2.print(text);
  }
}

void afficherEcran3(){
  //On attend max 1s de recevoir un message
  vw_wait_rx_max(600);

  initAffichage3();

  if (vw_get_message((byte*)&valeur, &wireless) == false){
    afficherSignal2();
  }else {
    afficherSignal1();
  }

  afficherDonnees();
}

void afficherDonnees(){
  u8g2.setFont(u8g2_font_6x12_tr);  

  char text[8];
  sprintf(text, "%d", rpmMax); 
  u8g2.setCursor(50,24);
  u8g2.print(text);

  sprintf(text, "%d", vitesseMax); 
  u8g2.setCursor(40,37);
  u8g2.print(text);

  sprintf(text, "%d", temperatureMax); 
  u8g2.setCursor(60,50);
  u8g2.print(text);

  sprintf(text, "%f", tempsDeRoulage); 
  u8g2.setCursor(70,63);
  u8g2.print(text);
}

void afficherBatterie(){
  u8g2.setFont(u8g2_font_battery19_tn);
  if (vin >= 4.12){
    u8g2.drawGlyph(110, 64, 54);
  }else if (pourcentageBatterie == 0){
    u8g2.drawGlyph(110, 64, 48);
  }else if (pourcentageBatterie > 0 && pourcentageBatterie <= 20){
    u8g2.drawGlyph(110, 64, 49);
  }else if (pourcentageBatterie >= 20 && pourcentageBatterie <= 40){
    u8g2.drawGlyph(110, 64, 50);
  }else if(pourcentageBatterie >= 40 && pourcentageBatterie <= 60){
    u8g2.drawGlyph(110, 64, 51);
  }else if(pourcentageBatterie >= 60 && pourcentageBatterie <= 80){
    u8g2.drawGlyph(110, 64, 52);
  }else if(pourcentageBatterie >= 80 && pourcentageBatterie <= 100){
    u8g2.drawGlyph(110, 64, 53);
  }
  
  if (pourcentageBatterie == 0){
    u8g2.drawGlyph(37, 64, 48);
  }else if (valeur[3] > 0 && valeur[3] <= 20){
    u8g2.drawGlyph(37, 64, 49);
  }else if (valeur[3] >= 20 && valeur[3] <= 40){
    u8g2.drawGlyph(37, 64, 50);
  }else if (valeur[3] >= 40 && valeur[3] <= 60){
    u8g2.drawGlyph(37, 64, 51);
  }else if (valeur[3] >= 60 && valeur[3] <= 80){
    u8g2.drawGlyph(37, 64, 52);
  }else if (valeur[3] >= 80 && valeur[3] <= 100){
    u8g2.drawGlyph(37, 64, 53);
  }
  u8g2.setFont(u8g2_font_streamline_transportation_t);
  u8g2.drawGlyph(0, 66, 53);

  u8g2.setFont(u8g2_font_streamline_phone_t);
  u8g2.drawGlyph(80, 64, 48);
}

void calculDonnees(){
  if (valeur[5] == 1){
    tempsDeRoulage += 0.24;
  }
  if (valeur[0] > rpmMax){
    rpmMax = valeur[0];
  }
  if (valeur[1] > vitesseMax){
    vitesseMax = valeur[1];
  }
  if (valeur[2] > temperatureMax){
    temperatureMax = valeur[2];
  }
}

void setup(){
  Serial.begin(9600);
  vw_setup(400);
  vw_rx_start();
  u8g2.begin();
  u8g2.enableUTF8Print();
  afficherEcranDemarrage();
}

void loop(){
  while (analogRead(BUTTON_PIN) != 0){
    Serial.println(F("Ecran 1"));
    u8g2.clearBuffer();
    calculDonnees();
    afficherEcran1();
    u8g2.sendBuffer();
  }

  delay(500);
  afficherTransition();

  while (analogRead(BUTTON_PIN) != 0){
    Serial.println(F("Ecran 2"));
    u8g2.clearBuffer();
    calculDonnees();
    afficherEcran2();
    u8g2.sendBuffer();
  }

  delay(500);
  afficherTransition();

  while (analogRead(BUTTON_PIN) != 0){
    Serial.println(F("Ecran 3"));
    u8g2.clearBuffer();
    calculDonnees();
    afficherEcran3();
    u8g2.sendBuffer();
  }

  delay(500);
  afficherTransition();
}