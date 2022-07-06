#include <Arduino.h>
#include <U8g2lib.h>
#include <VirtualWire.h>
#include <Wire.h>
#include <stdio.h>
#include <OneButton.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//Définition des pin d'entrée
#define BUTTON_PIN A1
#define PIN_TENSION A0
// SDK --> A5
// SDA --> A4

#define POLICE_PRINCIPALE u8g2_font_osb18_tn
#define POLICE_VOYANTS u8g2_font_open_iconic_all_2x_t
#define POLICE_CHIFFRES u8g2_font_lubR18_tr

/*OneButton btn = OneButton(
  BUTTON_PIN,  // Input pin for the button
  true,        // Button is active LOW
  true         // Enable internal pull-up resistor
);*/

//Variables pour la reception du signal
int valeur[6];
byte wireless = sizeof(valeur);

int timerEcran = 0;
float vin = 0.0;
float R1 = 98100.0; // Résistance de R1 (100K)
float R2 = 9870.0; // Résistance de R2 (10K)
int TensionBatterie = 0;
int pourcentageBatterie = 0;

//Pour ecran 3
int rpmMax = 0;
int vitesseMax = 0;
int temperatureMax = 0;
float tempsDeRoulage = 0;


void pre(){
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_profont11_mr);    
  u8g2.drawStr(0, 12, "Trophy Truck");
}

void afficherRpmVitesse(){
  u8g2.setDrawColor(1);
  u8g2.setFont(POLICE_CHIFFRES);  
  u8g2.drawStr(0, 38, "RPM:");
  u8g2.drawStr(0, 64, "V:");
}

void afficherKmh(){
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_6x12_tr);  
  u8g2.drawStr(95, 64, "Km/h");
}

void afficherBatterieTemperature(){
  u8g2.setDrawColor(1);
  u8g2.setFont(POLICE_CHIFFRES);  
  //u8g2.drawGlyph(60, 38, 0x00b0);
  u8g2.drawCircle(72, 21, 2, U8G2_DRAW_ALL);
  u8g2.drawStr(0, 38, "T:");
  u8g2.drawStr(75, 38, "C");
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
  // BATTERIE -- Lecture de la tension de la batterie sur le Pin A0
   TensionBatterie = analogRead(PIN_TENSION);
   vin = (TensionBatterie*4.75)/1000; 
   if (vin < 0.09) {
      vin = 0.0;//Déclaration pour annuler la lecture indésirable !
   }
   pourcentageBatterie = ((vin - 3.7)/(4.12-3.7))*100;

   Serial.print("Batterie : " );
   Serial.println(vin);

  if (pourcentageBatterie > 100){
    pourcentageBatterie = 100;
  }else if(pourcentageBatterie < 0){
    pourcentageBatterie = 0;
  }
}

void afficherEcran1(){  
  //On attend max 1s de recevoir un message
  vw_wait_rx_max(1000);

  initAffichage1();

  if (vw_get_message((byte*)&valeur, &wireless) == false){
    afficherSignal2();
  }else {
    afficherSignal1();
    //Serial.print("Test reception");
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
  vw_wait_rx_max(1000);
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
  vw_wait_rx_max(1000);

  initAffichage1();

  if (vw_get_message((byte*)&valeur, &wireless) == false){
    afficherSignal2();
  }else {
    afficherSignal1();
    u8g2.setFont(u8g2_font_Pixellari_tr);    
    u8g2.drawStr(0, 25, "rpmMax : ");
    u8g2.drawStr(0, 38, "vMax : ");
    u8g2.drawStr(0, 51, "tempMax : ");
    u8g2.drawStr(0, 64, "tpsRoulage : ");
  }
}

void afficherBatterie(){
  u8g2.setFont(u8g2_font_battery19_tn);
  if (pourcentageBatterie == 0){
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
  /*if (valeur[5] == 1){
    tempsDeRoulage += 0.24;
  }*/
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

static void handleClick() {
  Serial.println("Clicked!");
  if (timerEcran == 0){
    timerEcran = 1;
  } else if (timerEcran == 1){
    timerEcran = 2;
  } else if (timerEcran == 2){
    timerEcran = 0;
  }
}

void setup(){
  Serial.begin(9600);
  vw_setup(400);
  vw_rx_start();
  u8g2.begin();
  u8g2.enableUTF8Print();
  //btn.attachClick(handleClick);
}

void loop(){
  //while (timerEcran == 0){
  if (timerEcran == 3){
    timerEcran = 0;
  }
    Serial.println("Timer : ");
    Serial.print(timerEcran);
    u8g2.clearBuffer();
    switch (timerEcran){
      case 0:
        afficherEcran1();
        break;
    }    
    u8g2.sendBuffer();
    if (analogRead(BUTTON_PIN) == 0){
      timerEcran += 1;
    }

    delay(2000);
    Serial.println("Timer : ");
    Serial.print(timerEcran);
    u8g2.clearBuffer();
    switch (timerEcran){
      case 0:
        afficherEcran2();
        break;
    }    
    u8g2.sendBuffer();
    if (analogRead(BUTTON_PIN) == 0){
      timerEcran += 1;
    }
    delay(2000);
}
