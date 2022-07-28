#include <Wire.h>
#include <U8g2lib.h>
#include <VirtualWire.h>
#include <Arduino.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//Définition des pin d'entrée
#define BUTTON_PIN A1
#define PIN_TENSION A0
// SDK --> A5
// SDA --> A4

#define POLICE_PRINCIPALE u8g2_font_osb18_tn
#define POLICE_CHIFFRES u8g2_font_lubR18_tr
#define POLICE_PETITE u8g2_font_profont11_mr

//Pour générer un bitmap, ouvrir l'image avec GIMP et l'exporter en .xbm
static const unsigned char Engine[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x20, 0x40, 0x00,
  0x00, 0xe0, 0x79, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x0d, 0x00,
  0x00, 0xf0, 0xff, 0x07, 0x00, 0x08, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x0c,
  0x00, 0x06, 0x00, 0xfc, 0xcf, 0x03, 0x0e, 0xfc, 0x6d, 0x00, 0x07, 0x9c,
  0x29, 0x00, 0x07, 0x9c, 0x29, 0x80, 0x03, 0x98, 0x39, 0x80, 0x03, 0x80,
  0x21, 0xc0, 0x0f, 0x80, 0x39, 0x00, 0x06, 0x9c, 0x29, 0x00, 0x06, 0x9c,
  0x29, 0x00, 0x03, 0x9c, 0x69, 0x00, 0x01, 0xf6, 0xef, 0x03, 0x00, 0xf3,
  0x00, 0x06, 0x00, 0x01, 0x00, 0x04, 0x80, 0x00, 0x00, 0x0c, 0x40, 0x00,
  0x00, 0xf8, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char Fan[] PROGMEM = {
  0x00, 0x00, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x03, 0xc0, 0x01, 0x06, 0x00,
  0x8e, 0x79, 0xde, 0x79, 0xde, 0x79, 0x0e, 0x71, 0x00, 0x60, 0x80, 0x03,
  0xc0, 0x03, 0xe0, 0x03, 0xe0, 0x03, 0x00, 0x00
};

//Variables pour la reception du signal
int valeur[6];
byte wireless = sizeof(valeur);

float vin = 0.0;
int pourcentageBatterie = 0;

//Pour ecran 3
int rpmMax = 0;
int vitesseMax = 0;
int temperatureMax = 0;
float tempsDeRoulage = 0;

void pre(){
  //u8g2.setDrawColor(0);
  u8g2.setFont(POLICE_PETITE);    
  u8g2.setCursor(0,12);
  u8g2.print(F("RC"));
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
  u8g2.setFont(u8g2_font_unifont_t_77); 
  u8g2.drawGlyph(18, 15, 9981); //Affiche un icone de pompe à essence
  u8g2.drawXBMP(37, 0, 16, 16, Fan); //Ventilo 1
  if (valeur[2] > 28){
    u8g2.drawXBMP(56, 0, 16, 16, Fan); //Ventilo 2
  }
  if (valeur[4] == 1){
    u8g2.setFont(u8g2_font_open_iconic_thing_2x_t); 
    u8g2.drawGlyph(75, 16, 67); //Affiche un icone de clef
  }
  if (valeur[5] == 1){
    u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);
    u8g2.drawGlyph(94, 16, 78); //Affiche un icone de demarrer
  }
  u8g2.setFont(u8g2_font_open_iconic_www_2x_t);  
  u8g2.drawGlyph(112, 16, 72); //Affiche un icone de reseau
}

void afficherSignal2(){
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_open_iconic_check_2x_t);  
  u8g2.drawGlyph(112, 16, 68);  //Affiche une croix pour le reseau
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
  for (int i = 0; i < 8; i++){
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_streamline_interface_essential_action_t);
    u8g2.drawGlyph(55, 37, 53);
    u8g2.setFont(POLICE_PETITE);  
    u8g2.setCursor(38,58);
    u8g2.print(F("Chargement"));
    u8g2.sendBuffer();
    delay(50);
  }
}

void afficherEcranDemarrage(){
  u8g2.setDrawColor(1);
  for (int i = 0; i < 25; i++){
    u8g2.clearBuffer();
    u8g2.setFont(POLICE_PETITE);    
    u8g2.setCursor(30,12);
    u8g2.print(F("Trophy Truck"));

    u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);
    u8g2.drawGlyph(55, 40, 78);

    u8g2.drawFrame(25, 55, i*3, 2);
    u8g2.sendBuffer();
    delay(50);
  }
}

void afficherDemarrage(){
  u8g2.setDrawColor(1);
  u8g2.drawXBMP(48, 19, 32, 32, Engine);
  u8g2.setFont(POLICE_PETITE);  
  u8g2.setCursor(40, 61);
  u8g2.print(F("Demarrage"));
}

void afficherEcran1(){  
  //On attend max 600ms de recevoir un message
  vw_wait_rx_max(600);

  if (vw_get_message((byte*)&valeur, &wireless) == false){
    afficherSignal2();
    initAffichage1();
  }else {
    afficherSignal1();
    if (valeur[5] == 1){
      pre();
      afficherDemarrage();
    } else { 
      initAffichage1();
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
}

void afficherEcran2(){
  //On attend max 1s de recevoir un message
  vw_wait_rx_max(600);
  tension();

  if (vw_get_message((byte*)&valeur, &wireless) == false){
    afficherSignal2();
    initAffichage2();
  afficherBatterie();
  } else {
    afficherSignal1();
    if (valeur[5] == 1){
      pre();
      afficherDemarrage();
    }
    else { 
      initAffichage2();
      afficherBatterie();
      u8g2.setFont(POLICE_PRINCIPALE);
    
      char text[3];
      sprintf(text, "%d", valeur[2]); 
      u8g2.setCursor(30,38); 
      u8g2.print(text);
    }
  }
}

void afficherEcran3(){
  //On attend max 1s de recevoir un message
  vw_wait_rx_max(600);

  if (vw_get_message((byte*)&valeur, &wireless) == false){
    afficherSignal2();
    initAffichage3();
    afficherDonnees();
  }else {
    afficherSignal1();
    if (valeur[5] == 1){
      pre();
      afficherDemarrage();
    } else {
      initAffichage3();
      afficherDonnees();
    }
  }
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

  sprintf(text, "%d", (char)tempsDeRoulage); 
  Serial.print(F("Temps : "));
  Serial.println(text);
  u8g2.setCursor(75,63);
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
    tempsDeRoulage += 0.6;
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
    //Serial.println(F("Ecran 1")); //Debug
    u8g2.clearBuffer();
    calculDonnees();
    afficherEcran1();
    u8g2.sendBuffer();
  }

  delay(500);
  afficherTransition();

  while (analogRead(BUTTON_PIN) != 0){
    //Serial.println(F("Ecran 2")); //Debug
    u8g2.clearBuffer();
    calculDonnees();
    afficherEcran2();
    u8g2.sendBuffer();
  }

  delay(500);
  afficherTransition();

  while (analogRead(BUTTON_PIN) != 0){
    //Serial.println(F("Ecran 3")); //Debug
    u8g2.clearBuffer();
    calculDonnees();
    afficherEcran3();
    u8g2.sendBuffer();
  }

  delay(500);
  afficherTransition();
}