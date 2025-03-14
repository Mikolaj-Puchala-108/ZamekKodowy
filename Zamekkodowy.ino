#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <string.h>
#include <EEPROM.h>


// Piny
#define ledGreen 11          
#define ledRed 12             
#define lock 10         
#define buzzer 13           

// Klawiatura
const byte ROWS = 4;           
const byte COLS = 4;           

byte rowPins[ROWS] = {9, 8, 7, 6};  
byte colPins[COLS] = {5, 4, 3, 2};  

char keys[ROWS][COLS] = {    // Mapa klawiszy na klawiaturze
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad klawiatura = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);  // Obiekt do obsługi klawiatury
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Obiekt do obsługi wyświetlacza LCD

int pozycjaCursor = 0;  // Zmienna do śledzenia pozycji kursora na LCD


// Hasło
String kod = "";          // Zmienna przechowująca aktualnie wpisane hasło
String wczytajHaslo();    // Prototyp funkcja do odczytu hasła z EEPROM

String key = wczytajHaslo();  // Wczytanie hasła z EEPROM (domyslne 1234)

void setup() {
  lcd.init();                 
  lcd.backlight();            
  lcd.clear();                

  //konfiguracja
  pinMode(buzzer, OUTPUT);    
  pinMode(ledRed, OUTPUT);    
  pinMode(ledGreen, OUTPUT);  
  pinMode(lock, OUTPUT);      

  // Sekwencja testowa LED i zamka
  for(int i = 0; i < 4; i++){
    digitalWrite(ledGreen, HIGH); 
    digitalWrite(ledRed, HIGH);    
    digitalWrite(lock, HIGH);      
    delay(100);                   
    digitalWrite(ledGreen, LOW);   
    digitalWrite(ledRed, LOW);     
    digitalWrite(lock, LOW);       
    delay(100);                    
  }

  digitalWrite(lock, HIGH);  
}

void loop() {
  char klawisz = klawiatura.getKey();  // Odczytanie naciśniętego klawisza
  int wiersz = 0;  
  if (klawisz) {   
    obsluzKlawisz(klawisz, wiersz);  // Obsłuż naciśnięty klawisz
  }
}


void obsluzKlawisz(char klawisz, int wiersz) {
  
  // Obsługa naciśniętych klawiszy
  if ((klawisz >= '0' && klawisz <= '9') || klawisz == '*' || klawisz == '#') {
    dodajCyfre(klawisz, wiersz);   // Dodaj cyfrę do hasła
  } else if (klawisz == 'D') {
    usunZnak(wiersz);   // Usuń ostatni znak z hasła
  } else if (klawisz == 'A') {
    sprawdzKod(wiersz);  // Sprawdź poprawność wpisanego hasła
  } else if (klawisz == 'C') {
    wyczyscEkran(wiersz);  // Wyczyść ekran LCD
  } else if (klawisz == 'B') {
    zmienHaslo();  // Zmień hasło
  }
}


void dodajCyfre(char cyfra, int wiersz) {
  lcd.setCursor(pozycjaCursor, wiersz);  
  kod += cyfra;  // Dodanie cyfry do hasła
  pozycjaCursor++;  // Przesunięcie kursora o jeden w prawo
  lcd.print(cyfra);  // Wyświetlenie cyfry na LCD

  // Jeżeli kursor przekroczy 16, przesuń go na początek linii
  if (pozycjaCursor >= 16) {
    pozycjaCursor = 0;
    lcd.setCursor(pozycjaCursor, wiersz);
  }
}

void usunZnak(int wiersz) {
  // Jeśli hasło zawiera znaki, usuń ostatni
  if (kod.length() > 0) {
    kod.remove(kod.length() - 1);
  }
  
  // Przesunięcie kursora w lewo
  if (pozycjaCursor > 0) {
    pozycjaCursor--;
  } else if (wiersz > 0) {
    wiersz = 0;
    pozycjaCursor = 15;
  }
  
  lcd.setCursor(pozycjaCursor, wiersz);  // Ustawienie kursora
  lcd.print(" ");  // Wyczyść znak na LCD
  lcd.setCursor(pozycjaCursor, wiersz);  // Powrót do ustawionego kursora
}

void sprawdzKod(int wiersz) {
  lcd.clear();  // Wyczyść ekran
  if (kod == key) {  // Sprawdzenie, czy hasło jest poprawne
    lcd.setCursor(0, 0);
    lcd.print("Dobry kod!!!");
    digitalWrite(lock, LOW);  // Otwórz zamek
    for (int i = 0; i < 4; i++) {
      digitalWrite(ledGreen, HIGH);  
      tone(buzzer, 1000);  
      delay(200);
      digitalWrite(ledGreen, LOW);  
      noTone(buzzer);  
      delay(200);
    }
    delay(1000);
    digitalWrite(lock, HIGH);  // Zablokuj zamek
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Zly kod!!!!");
    digitalWrite(ledRed, HIGH); 
    tone(buzzer, 1000);  
    delay(2000);
    digitalWrite(ledRed, LOW);  
    noTone(buzzer);  
  }
  wyczyscEkran(wiersz);  // Wyczyść ekran po sprawdzeniu
}

void wyczyscEkran(int wiersz) {
  lcd.clear();  // Wyczyść ekran
  pozycjaCursor = 0;  // Reset kursora
  wiersz = 0;  // Ustawienie wiersza na początek
  kod = "";  // Resetowanie wpisanego hasła
}

// Funkcja zmieniająca hasło
void zmienHaslo() {
    lcd.clear();
    lcd.print("Podaj akt.haslo:");  
    lcd.setCursor(0, 1);  // Ustawienie kursora na drugiej linii
    pozycjaCursor = 0;

    String wpisaneHaslo = wprowadzHaslo();  // Wczytanie wpisanego hasła

    // Sprawdzenie, czy użytkownik anulował zmianę hasła
    if (wpisaneHaslo == "\x1B") { 
        return;  // Anulowanie zmiany hasła
    }

    // Sprawdzanie, czy wprowadzone hasło jest poprawne
    if (wpisaneHaslo == key) {
        lcd.clear();
        lcd.print("Podaj nowe:"); 
        lcd.setCursor(0, 1);
        pozycjaCursor = 0;

        String noweHaslo = wprowadzHaslo();  // Wczytanie nowego hasła
        
        if (noweHaslo == "\x1B") {  // Sprawdzenie, czy anulowano zmianę hasła
            return; 
        }

        zapiszHaslo(noweHaslo);  // Zapisanie nowego hasła do EEPROM
        key = noweHaslo;  // Ustawienie nowego hasła jako aktualne

        lcd.clear();
        lcd.print("Haslo zmienione");  
        digitalWrite(ledGreen, HIGH);  
        tone(buzzer, 1000); 
        delay(500);
        digitalWrite(ledGreen, LOW);  
        noTone(buzzer);  
        delay(500);

    } else {
        lcd.clear();
        lcd.print("Zle haslo");  
        digitalWrite(ledRed, HIGH);  
        tone(buzzer, 1000); 
        delay(2000);
        digitalWrite(ledRed, LOW);  
        noTone(buzzer); 
    }

    wyczyscEkran(1);  
}


String wprowadzHaslo() {
    String haslo = "";
    char klawisz;

    while (true) {
        klawisz = klawiatura.getKey();  
        if (!klawisz){
          continue;  
        } 
        delay(50);  // Krótkie opóźnienie dla debouncingu

        if ((klawisz >= '0' && klawisz <= '9') || klawisz == '*' || klawisz == '#') {
            haslo += klawisz;  
            dodajCyfre(klawisz, 1);  
        } 
        else if (klawisz == 'D' && haslo.length() > 0) {
            haslo.remove(haslo.length() - 1); 
            usunZnak(1);  
        } 
        else if (klawisz == 'C') {  
            wyczyscEkran(1);  
            return "\x1B";  // Zwróć specjalny kod oznaczający anulowanie
        } 
        else if (klawisz == 'A') {
            return haslo;  // Zwróć wprowadzone hasło
        }
    }
}

// Funkcja do zapisywania nowego hasła w EEPROM
void zapiszHaslo(String noweHaslo) {
    int dlugosc = noweHaslo.length();  
    int aktualnaDlugosc = EEPROM.read(0);  // Odczytanie obecnej długości hasła

    // Sprawdzenie, czy nowe hasło jest takie samo jak obecne
    if (noweHaslo == key) {
        return;  // Nie zapisuj, jeżeli hasło się nie zmieniło
    }

    // Zapisanie nowego hasła do EEPROM
    EEPROM.write(0, dlugosc);  
    for (int i = 0; i < dlugosc; i++) {
        EEPROM.write(i + 1, noweHaslo[i]);
    }
}

// Funkcja do odczytu hasła z EEPROM
String wczytajHaslo() {
    int dlugosc = EEPROM.read(0);  // Odczytanie długości zapisanego hasła
    String haslo = "";
    for (int i = 0; i < dlugosc; i++) {
        haslo += char(EEPROM.read(i + 1));  // Odczytanie poszczególnych znaków hasła
    }
    return haslo;  // Zwrócenie wczytanego hasła
}




