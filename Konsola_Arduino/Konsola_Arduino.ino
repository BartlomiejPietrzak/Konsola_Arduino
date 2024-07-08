#include <Arduino.h>

// biblioteka ekranu. 
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif 
// biblioteka ekranu


// STAŁE
#define SZEROKOSC_EKRANU 128
#define WYSOKOSC_EKRANU 64
#define LICZBA_PRZYCISKOW 4 // LICZBA UZYTYCH PRZYCISKOW
// STAŁE


// PRZYCISKI I JOY 
const int przyciskLewy = 4; 
const int przyciskPrawy = 2; 
const int przyciskGora = 5; 
const int przyciskDol = 3; 
const int joyOX = A6;
// const int joyOY = A7;


// GŁÓWNE USTAWIENIA GRY I WYSWIETLANIA

const int uzytePrzyciski[LICZBA_PRZYCISKOW] = {2, 3, 4, 5}; // TABLICA UZYTYCH PRZYCISKOW Z ICH PODPISANYMI WEJ

// Rozmiary paletki
int wysPaletki = 16;
int szerPaletki = 2;

//Pozycja paletek i wynik 
int pozG1 = 32; // pozycja gracza 1
int wynG1 = 0; // wynik gracza 1
int pozG2 = 32; // pozycja gracza 2
int wynG2 = 0; // wynik gracza 2

// Rozmiar i pozycja piłki 

float pozXpilki = SZEROKOSC_EKRANU/2; // pozycja piłki na ekranie
float pozYpilki = WYSOKOSC_EKRANU/2;  // -,-

float predXpilki = 1; // predkosc piłki ox 
float predYpilki = 2;  // predkosc piłki oy

int pilkaR = 2; // promien piłki 
int pilkaGol = 2; // o ile pikseli piłka musi opóścić ekran żeby gol był uznany 

int stanPrzyciskow[14];

/* KOSTRUKTOR OLED I PODŁĄCZENIE EKRANU

GND -> GND;VCC-> 3.3; SCK->D13; SDA->D11; RES->D8; DC-> D9; CS->D10

     OLED CONNECTION 

*/

U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);

void u8g2_prepare(void) { // ustawienia czcionki, i wyswietlania teksu 
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}
 
void u8g2_str(int x,int y, String string) // Wyswietlanie wyniku na ekranie 
{
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setCursor(x,y);
  u8g2.print(string);
}


void wysWyniku(int wyn1, int wyn2) // Rozmieszczenie wyniku na ekranie z przypisaniem do graczy 
{
  u8g2_str(32, 5, String(wyn1));
  u8g2_str(96, 5, String(wyn2));
}

int sprawdzanieStanuPrzyciskow(int przycisk) // sprawdzanie stanu przycisków
{
  if(digitalRead(przycisk) == LOW)
  {
    Serial.println("true");
    return 1;
  }
  else if(digitalRead(przycisk) == HIGH)
  {
    Serial.println("false");
    return 0;
  }
}

void pozycjaStartowa(void) // POZYCJE STARTOWE
{
  pozXpilki = 64;
  pozYpilki = 32;
  pozG1 = 32;
  pozG2 = 32;
}


void logikaGry(void) // głowna funkcja gry
{
  for(int i = 0; i < LICZBA_PRZYCISKOW; i++)
  {
    stanPrzyciskow[uzytePrzyciski[i]] = sprawdzanieStanuPrzyciskow(uzytePrzyciski[i]);
  }
  
  if (pozG1 > 0){pozG1 -= stanPrzyciskow[4] * 3;} // ruch paletki w dół gracza 1
  if (pozG1 < 63 - wysPaletki){pozG1 += stanPrzyciskow[3] * 3;} // ruch paletki do góry gracza 1

  if (pozG2 > 0){pozG2 -= stanPrzyciskow[5] * 3;}// ruch paletki w dół gracza 2
  if (pozG2 < 63 - wysPaletki){pozG2 += stanPrzyciskow[2] * 3;}// ruch paletki w górę gracza 2

  //zdobywanie punktów 
  if (pozXpilki >= 128 + pilkaGol){wynG1++; pozycjaStartowa();}
  if (pozXpilki <= 0 - pilkaGol){wynG2++; pozycjaStartowa();}

  //kolizja piłki ze górą i dołem ekranu 
  if (pozYpilki >= 63 - pilkaR){predYpilki = -1.0;}
  if (pozYpilki <= 0 + pilkaR){predYpilki = 1.0;}
  
// kolizja piłki z graczem 1 
  if (pozXpilki >= (128 - pilkaR - szerPaletki))
  {
    if(pozG2 <= pozYpilki and pozYpilki <= pozG2 + wysPaletki)
    {
      predXpilki = -2;
    }
  }

// kolizja piłki z graczem 2
  if (pozXpilki <= (0 + pilkaR + szerPaletki))
  {
    if(pozG1 <= pozYpilki and pozYpilki <= pozG1 + wysPaletki)
    {
      predXpilki = 2;
    }
  }
  // ruch piłki 
  pozXpilki += predXpilki;
  pozYpilki += predYpilki;
  
 
  //wyswietlanie piłki  
  u8g2.drawDisc(pozXpilki, pozYpilki, pilkaR, U8G2_DRAW_ALL);

  //wyswietlanie paletki gracza 1
  u8g2.drawBox(0, pozG1, szerPaletki, wysPaletki);

  //wyswietlanie paletki gracza 2
  u8g2.drawBox(128-szerPaletki, pozG2, szerPaletki, wysPaletki);
  
  // wyswietlanie wyniku
  wysWyniku(wynG1, wynG2);

   if(wynG1 == 4 || wynG2 == 4) // koniec gry 
  {
    wynG1 = 0;
    wynG2 = 0;
    delay(500);
  }
}

void setup(void) {
  Serial.begin(9600); // testownie przypisanych przyciskow
  u8g2.begin(); // komunikacja z ekranem 
  // przypisanie przyciskow 
  pinMode(przyciskLewy,INPUT_PULLUP);
  pinMode(przyciskPrawy, INPUT_PULLUP);
  pinMode(przyciskGora,INPUT_PULLUP);
  pinMode(przyciskDol,INPUT_PULLUP);
  pinMode(joyOX,INPUT);

}

void loop(void) {
  u8g2_prepare();
  u8g2.clearBuffer(); // obsługa ekranu 

  /* SPRAWDZANIE POPRAWNOSCI FUNKCJI
     sprawdzanieStanuPrzyciskow(przyciskGora); // sprawdzanie poprawnosci przyciskow
    wysWyniku(1,1);
    -;- główny cykl -;- 
  */
  
  logikaGry(); // pętla gry 
  u8g2.sendBuffer(); // obsługa ekranu 
  delay(1); 

}