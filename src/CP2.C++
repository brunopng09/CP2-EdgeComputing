#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>
#include <EEPROM.h>
#include "DHT.h"

#define LOG_OPTION 1      // Opção para ativar a leitura do log
#define DHTPIN 2
#define DHTTYPE DHT22

#define UTC_OFFSET 0     // Fuso horário UTC-3

// CONTROLE DA LIMPEZA DA EEPROM
#define CLEAR_EEPROM_ON_BOOT 0

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 RTC;

const int btnNavegar = 3;
const int btnTemp = 4;
const int LDR = A0;
const int ledVerde = 10;
const int ledVermelho = 12;
const int alarme = 13;

// Caracteres personalizados (exemplo com até 8, índices 0-7)
byte shit[8] = { B00000, B10001, B00000, B00000, B01110, B10001, B00000, B00000 };
byte smile[8] = { B00000, B10001, B00000, B00000, B10001, B01110, B00000, B00000 };
byte serious[8] = { B00000, B10001, B00000, B00000, B11111, B00000, B00000, B00000 };
byte helicoptero1[8] = { B11111, B00010, B01111, B10000, B10000, B01111, B00001, B00111 };
byte helicoptero2[8] = { B10000, B00000, B00001, B10010, B11111, B10000, B00000, B11000 };
byte ponto[8] = { B00000, B00000, B00000, B00000, B00100, B01010, B00100, B00000 };
byte lupa2[8] = { B00011, B00110, B00100, B00100, B01111, B11100, B11000, B10000 };
byte lupa1[8] = { B00000, B00000, B00000, B00000, B00000, B00000, B00001, B00001 };

int telaAtual = 0;
int modoTemperatura = 0;

unsigned long tempoAlarme = 3000;
bool buzzerTocando = false;

unsigned long ultimoClique = 0;
const unsigned long debounceDelay = 300;

// EEPROM
const int recordSize = 10;  // 4 bytes timestamp + 2 temp + 2 hum + 2 luz
const int maxRecords = 100;
const int startAddress = 0;
const int endAddress = maxRecords * recordSize;
int enderecoAtual = startAddress;
unsigned long ultimoRegistro = 0;
const unsigned long intervaloRegistro = 30000; // 30 segundos
int enderecoLeitura = startAddress;
unsigned long ultimoLog = 0;
const unsigned long intervaloLog = 30000; // 30 segundos

void clearEEPROM() {
  Serial.println("Limpando a EEPROM...");
  for (int i = startAddress; i < endAddress; i++) {
    if (EEPROM.read(i) != 0xFF) {
      EEPROM.write(i, 0xFF);
    }
  }
  enderecoAtual = startAddress;
  enderecoLeitura = startAddress;
  Serial.println("EEPROM limpa. Mude CLEAR_EEPROM_ON_BOOT para 0 e reinicie.");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Monitor Serial Iniciado");

  lcd.init();
  lcd.backlight();

  // Cria caracteres personalizados (0-7)
  lcd.createChar(0, shit);
  lcd.createChar(1, smile);
  lcd.createChar(2, serious);
  lcd.createChar(3, helicoptero1);
  lcd.createChar(4, helicoptero2);
  lcd.createChar(5, ponto);
  lcd.createChar(6, lupa1);
  lcd.createChar(7, lupa2);
  // Não usar índice 8, LCD suporta até 7

#if CLEAR_EEPROM_ON_BOOT == 1
  clearEEPROM();
#endif

  dht.begin();
  if (!RTC.begin()) {
    Serial.println("Erro ao inicializar RTC");
    while (1);
  }

  // Ajuste do RTC: faça uma vez, depois comente ou remova esta linha!
  // RTC.adjust(DateTime(2025, 5, 23, 19, 47, 0)); // Ajuste para UTC atual

  pinMode(btnNavegar, INPUT_PULLUP);
  pinMode(btnTemp, INPUT_PULLUP);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(alarme, OUTPUT);

  Serial.println("Setup concluido.");
}

void loop() {
  verificarAmbiente();
  lerBotoes();
  mostrarTela();
  registrarDados();

  if (LOG_OPTION == 1) {
    lerEEPROMnoSerial();
  }
  delay(100);
}

void lerBotoes() {
  if (millis() - ultimoClique > debounceDelay) {
    if (digitalRead(btnNavegar) == LOW) {
      telaAtual = (telaAtual + 1) % 4;
      lcd.clear();
      ultimoClique = millis();
    }
    if (digitalRead(btnTemp) == LOW && telaAtual == 2) {
      modoTemperatura = (modoTemperatura + 1) % 3;
      lcd.clear();
      ultimoClique = millis();
    }
  }
}

void mostrarTela() {
  switch (telaAtual) {
    case 0: exibirStatusAmbiente(); break;
    case 1: mostrarUmidade(); break;
    case 2: mostrarTemperatura(); break;
    case 3: mostrarPorcentagemLuminosidade(); break;
  }
}

void mostrarTemperatura() {
  float tempC = dht.readTemperature();
  float valor = tempC;
  String suf = "C";

  if (modoTemperatura == 1) {
    valor = tempC * 1.8 + 32;
    suf = "F";
  } else if (modoTemperatura == 2) {
    valor = tempC + 273.15;
    suf = "K";
  }

  lcd.setCursor(1, 1);
  lcd.print(valor, 1);
  lcd.print(" ");
  lcd.print(suf);
}

void mostrarUmidade() {
  float h = dht.readHumidity();
  lcd.setCursor(0, 1);
  if (isnan(h)) {
    lcd.print("Erro DHT       ");
  } else {
    lcd.print(h, 1);
    lcd.print("%            ");
  }
}

void mostrarPorcentagemLuminosidade() {
  int ldr = analogRead(LDR);
  int porcentagem = map(ldr, 0, 1023, 100, 0);
  lcd.setCursor(3, 1);
  lcd.print(porcentagem);
  lcd.print("%          ");
}

void exibirStatusAmbiente() {
  float temp = dht.readTemperature();
  float umid = dht.readHumidity();
  int ldr = analogRead(LDR);
  int luzPercent = map(ldr, 0, 1023, 100, 0);

  lcd.setCursor(0, 0);
  lcd.print("Status:");

  if (!isnan(temp) && !isnan(umid)) {
    if (temp >= 22 && temp <= 26 && umid >= 40 && umid <= 60 && luzPercent >= 40 && luzPercent <= 80) {
      digitalWrite(ledVerde, HIGH);
      digitalWrite(ledVermelho, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Ambiente OK  ");
      desligarAlarme();
    } else {
      digitalWrite(ledVerde, LOW);
      digitalWrite(ledVermelho, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Ambiente Ruim");
      ligarAlarme();
    }
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Erro Sensor  ");
  }
}

void ligarAlarme() {
  digitalWrite(alarme, HIGH);
  buzzerTocando = true;
}

void desligarAlarme() {
  digitalWrite(alarme, LOW);
  buzzerTocando = false;
}

void verificarAmbiente() {
  // Pode implementar algo extra aqui se quiser
}

void registrarDados() {
  if (millis() - ultimoRegistro >= intervaloRegistro) {
    ultimoRegistro = millis();

    DateTime now = RTC.now();
    long timestamp = now.unixtime();  // Em UTC

    float temp = dht.readTemperature();
    float umid = dht.readHumidity();
    int ldr = analogRead(LDR);
    int luz = map(ldr, 0, 1023, 0, 100);

    if (isnan(temp) || isnan(umid)) {
      Serial.println("Erro ao ler sensores, registro ignorado.");
      return;
    }

    int tempInt = (int)(temp * 100);
    int umidInt = (int)(umid * 100);
    int luzInt = luz;  // luz já está em 0-100, não multiplique

    EEPROM.put(enderecoAtual, timestamp);
    EEPROM.put(enderecoAtual + 4, tempInt);
    EEPROM.put(enderecoAtual + 6, umidInt);
    EEPROM.put(enderecoAtual + 8, luzInt);

    Serial.print("Dados gravados na EEPROM no endereco ");
    Serial.println(enderecoAtual);

    enderecoAtual += recordSize;
    if (enderecoAtual >= endAddress) {
      enderecoAtual = startAddress;
      Serial.println("Ponteiro da EEPROM (escrita) voltou ao inicio.");
    }
  }
}

void lerEEPROMnoSerial() {
  if (millis() - ultimoLog >= intervaloLog) {
    ultimoLog = millis();

    long timestamp;
    int tempInt, umidInt, luzInt;

    EEPROM.get(enderecoLeitura, timestamp);
    EEPROM.get(enderecoLeitura + 4, tempInt);
    EEPROM.get(enderecoLeitura + 6, umidInt);
    EEPROM.get(enderecoLeitura + 8, luzInt);

    Serial.print("Lendo EEPROM no endereco: ");
    Serial.println(enderecoLeitura);

    if (timestamp != 0xFFFFFFFF && timestamp != 0) {
      // Aplica o fuso horário na exibição (somente)
      DateTime dt(timestamp + (UTC_OFFSET * 3600UL));
      float temp = tempInt / 100.0;
      float umid = umidInt / 100.0;
      float luz = (float)luzInt;

      Serial.println("======= Registro EEPROM =======");
      Serial.print("Data/Hora: ");
      Serial.println(dt.timestamp(DateTime::TIMESTAMP_FULL));
      Serial.print("Temperatura: ");
      Serial.print(temp);
      Serial.println(" C");
      Serial.print("Umidade: ");
      Serial.print(umid);
      Serial.println(" %");
      Serial.print("Luminosidade: ");
      Serial.print(luz);
      Serial.println(" %");
      Serial.println("===============================");
    } else {
      Serial.println("Registro vazio ou invalido na EEPROM.");
    }

    enderecoLeitura += recordSize;
    if (enderecoLeitura >= endAddress) {
      enderecoLeitura = startAddress;
      Serial.println("Ponteiro da EEPROM (leitura) voltou ao inicio.");
    }
  }
}
