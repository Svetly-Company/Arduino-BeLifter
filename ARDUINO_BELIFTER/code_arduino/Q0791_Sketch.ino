/*
     CÓDIGO:    Q0791
     AUTOR:     BrincandoComIdeias
     ACOMPANHE: https://www.youtube.com/brincandocomideias ; https://www.instagram.com/canalbrincandocomideias/
     APRENDA:   https://cursodearduino.net/ ; https://cursoderobotica.net
     COMPRE:    https://www.arducore.com.br/
     SKETCH:    Trava com leitor biometrico
     DATA:      26/10/2021
*/
#include <Adafruit_Fingerprint.h>
#include <VarSpeedServo.h>

#define pinLed    4
#define pinServo  8

#define ABERTO    10
#define FECHADO   105
#define LIBERADO  3

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial);

VarSpeedServo trava;

unsigned long ultimaLeitura = 0;
int digital = -1;

void setup() {
  finger.begin(57600);

  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, LOW);

  if (!finger.verifyPassword()) {
    while (true) {
      digitalWrite(pinLed, bitRead(millis() , 5));
    }
  }

  trava.attach(pinServo);
  trava.slowmove(FECHADO, 20);

  finger.getTemplateCount();

  if (finger.templateCount > 0) {
    digitalWrite(pinLed, HIGH); //SETUP CONCLUIDO
  } else {
    while (true) {
      digitalWrite(pinLed, bitRead(millis() , 5));
    }
  }

}

void loop() {
  digital = getFingerprintID();

  if (digital > 0) {
    digitalWrite(pinLed, LOW);
    delay(500);
    if (digital == LIBERADO) {
      trava.slowmove(ABERTO, 20);
      delay(5000);
      trava.slowmove(FECHADO, 20);
    } else {
      Serial.println ("Acesso Não Autorizado!");
    }
    digitalWrite(pinLed, HIGH);
  }

  delay(100);
}

int getFingerprintID() {
  uint8_t fp = finger.getImage();
  if (fp != FINGERPRINT_OK)  return -1;

  fp = finger.image2Tz();
  if (fp != FINGERPRINT_OK)  return -1;

  fp = finger.fingerFastSearch();
  if (fp != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("\nEncontrado ID #"); Serial.println(finger.fingerID);
  return finger.fingerID;
}
