#include <Arduino.h>
#include <U8g2lib.h>
#include <VirtualWire.h>
#include <Wire.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//Définition des pin d'entrée
#define PIN_BOUTTON A1
#define PIN_TENSION A0
// SDK --> A5
// SDA --> A4

//Variables pour la reception du signal
int valeur[4];
byte wireless = sizeof(valeur);

int timerEcran = 0;

float vin = 0.0;
float R1 = 98100.0; // Résistance de R1 (100K)
float R2 = 9870.0; // Résistance de R2 (10K)
int TensionBatterie = 0;
int pourcentageBatterie = 0;

void pre(void)
{
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_pxplusibmvga8_mr);    
  u8g2.drawStr(0, 12, "Trophy Truck ");
}

void afficherRpmVitesse(){
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_lubR18_tr);  
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
  u8g2.setFont(u8g2_font_lubR18_tr);  
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
  u8g2.setFont(u8g2_font_open_iconic_all_2x_t);  
  u8g2.drawGlyph(112, 16, 247); //Affiche un icone de reseau
}

void afficherSignal2(){
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_open_iconic_all_2x_t);  
  u8g2.drawGlyph(112, 16, 283);  //Affiche une croix pour le reseau
}

void tension(){
  // BATTERIE -- Lecture de la tension de la batterie sur le Pin A0
   TensionBatterie = analogRead(PIN_TENSION);
   vin = (TensionBatterie*4.75)/1000; 
   if (vin<0.09) {
      vin=0.0;//Déclaration pour annuler la lecture indésirable !
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
    Serial.print("Test reception");
    u8g2.setCursor(64,38);
    u8g2.setFont(u8g2_font_osb18_tn);  
    u8g2.print(valeur[0]);

    u8g2.setCursor(35,64);
    u8g2.print(valeur[1]);
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
  }else {
    afficherSignal1();
    u8g2.setFont(u8g2_font_osb18_tn);
  
    u8g2.setCursor(30,38); 
    u8g2.print(valeur[2]);
  }
  
  timerEcran++;
  if (timerEcran == 20){
    timerEcran = 0;
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

void setup(){
  Serial.begin(9600);
  vw_setup(500);
  vw_rx_start();
  u8g2.begin();
  u8g2.enableUTF8Print();
}

void loop(){
  u8g2.firstPage();
  do{ 
    Serial.println("test");
    if (analogRead(PIN_BOUTTON) == 0){
      timerEcran = 1;
        while (timerEcran != 0){
            u8g2.firstPage();
            do{
              afficherEcran2();
            }while(u8g2.nextPage());
        }
    }else {
          afficherEcran1();
    }
  } while(u8g2.nextPage());
}
