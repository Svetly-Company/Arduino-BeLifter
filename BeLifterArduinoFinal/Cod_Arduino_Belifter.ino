#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Servo.h> // Incluindo a biblioteca do Servo

// Initialize LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize fingerprint sensor
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#else
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Servo myServo; // Cria um objeto Servo
String command;

// Array para armazenar os nomes associados aos IDs
String names[] = {"", "Alvaro Cordeiro", "Nome 2", "Nome 3", "Nome 4", "Nome 5", /* adicione mais nomes conforme necessário */};

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(100);

  // Initialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  myServo.attach(6); // Define o pino do servo
  myServo.write(0); // Inicializa o servo na posição 0

  finger.begin(57600);
  delay(5);
  if (!finger.verifyPassword()) {
    Serial.println("Não foi possível encontrar o sensor de impressão digital :(");
    lcd.clear();
    lcd.print("Sensor não encontrado");
    while (1) { delay(1); }
  }

  Serial.println(F("Lendo parâmetros do sensor"));
  lcd.clear();
  lcd.print("Lendo parâmetros...");

  finger.getParameters();
  if (finger.templateCount == 0) {
    Serial.print("O sensor não contém dados de impressão digital. Execute o exemplo 'cadastrar'.");
    lcd.clear();
    lcd.print("Sem dados cadastrados");
  } else {
    Serial.println("Aguardando dedo válido...");
    lcd.clear();
    lcd.print("Aguardando dedo...");
    Serial.print("Sensor contém "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

void loop() {
  if (Serial.available()) {
    command = Serial.readStringUntil('\n');
    command.trim(); // Remove espaços em branco

    if (command.equalsIgnoreCase("cadastrar")) {
      getFingerprintEnroll();
    } else {
      Serial.println("Comando inválido! Digite 'cadastrar' ou coloque o dedo.");
      lcd.clear();
      lcd.print("Comando inválido!");
      delay(2000);
    }
  }

  getFingerprintID(); // Chama a leitura se nenhum comando foi recebido
  delay(50);
}

uint8_t getFingerprintEnroll() {
  lcd.clear();
  Serial.println("Pronto para inscrever uma impressão digital!");
  lcd.print("Inscrevendo dedo...");

  // Solicita um ID válido e verifica se está dentro do intervalo
  uint8_t id = 0;
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print("ID (1-127): ");
    id = readNumber();
    if (id >= 1 && id <= 127) {
      break; // ID válido, sai do loop
    } else {
      lcd.clear();
      lcd.print("ID inválido! Tente:");
      delay(2000); // Mostra a mensagem por 2 segundos
      lcd.clear();
    }
  }

  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      Serial.println("Imagem capturada");
      lcd.clear();
      lcd.print("Imagem capturada");
    } else if (p == FINGERPRINT_NOFINGER) {
      // Nada a fazer
    } else {
      Serial.println("Erro de comunicação ou imagem");
      lcd.clear();
      lcd.print("Erro de captura");
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return handleEnrollError(p);

  Serial.println("Remova o dedo");
  lcd.clear();
  lcd.print("Remova o dedo");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER) {}

  Serial.println("Coloque o mesmo dedo novamente");
  lcd.clear();
  lcd.print("Coloque o mesmo");
  lcd.setCursor(0, 1);
  lcd.print("dedo novamente");

  while ((p = finger.getImage()) != FINGERPRINT_OK) {
    if (p != FINGERPRINT_NOFINGER) {
      Serial.println("Erro de comunicação ou imagem");
    }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return handleEnrollError(p);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Impressões combinadas!");
    lcd.clear();
    lcd.print("Impressões OK!");
  } else {
    return handleEnrollError(p);
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Armazenado!");
    lcd.clear();
    lcd.print("Cadastrado com sucesso!");
    delay(2000); // Tempo para mostrar a mensagem
  } else {
    return handleEnrollError(p);
  }
  return id;
}

uint8_t handleEnrollError(uint8_t errorCode) {
  switch (errorCode) {
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Imagem muito bagunçada");
      lcd.clear();
      lcd.print("Imagem bagunçada");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro de comunicação");
      lcd.clear();
      lcd.print("Erro de comm.");
      break;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Não foi possível encontrar características da impressão digital");
      lcd.clear();
      lcd.print("Sem características");
      break;
    default:
      Serial.println("Erro desconhecido");
      lcd.clear();
      lcd.print("Erro desconhecido");
      break;
  }
  delay(2000); // Atraso para exibir a mensagem
  return errorCode;
}

void getFingerprintID() {
  Serial.println("Aguardando dedo...");
  lcd.clear();
  lcd.print("Aguardando dedo...");

  while (true) {
    uint8_t p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      Serial.println("Imagem capturada");
      lcd.clear();
      lcd.print("Imagem capturada");
      p = finger.image2Tz();
      if (p == FINGERPRINT_OK) {
        p = finger.fingerSearch();
        if (p == FINGERPRINT_OK) {
          // Se o ID for 1, exibe o nome "Alvaro Cordeiro"
          if (finger.fingerID == 1) {
            lcd.clear();
            lcd.print("Bem-vindo ");
            lcd.setCursor(0, 1);
            lcd.print(names[1]);  // Exibe o nome correspondente ao ID 1
          } else {
            Serial.print("Encontrado ID #"); Serial.print(finger.fingerID);
            Serial.print(" com confiança de "); Serial.println(finger.confidence);

            lcd.clear();
            lcd.print("Bem-vindo ID #");
            lcd.setCursor(0, 1);
            lcd.print(finger.fingerID);
          }

          myServo.write(90); // Ativa o servo
          delay(2000); // Mantém o servo ativo por 2 segundos
          myServo.write(0); // Retorna o servo para a posição inicial

          delay(5000); // Atraso de 5 segundos
          lcd.clear();
          lcd.print("Sistema BeLifter");
          lcd.setCursor(0, 1);
          lcd.print("Seja Bem-vindo!");
          delay(5000); // Mostra a mensagem por 5 segundos
          break; // Sai do loop após encontrar a impressão
        } else {
          Serial.println("Não foi encontrada uma correspondência");
          lcd.clear();
          lcd.print("Sem correspondência");
        }
      } else {
        Serial.println("Erro ao converter imagem");
        lcd.clear();
        lcd.print("Erro ao converter");
      }
    } else if (p == FINGERPRINT_NOFINGER) {
      // Nenhum dedo detectado, apenas continue
    } else {
      Serial.println("Erro de comunicação ou imagem");
      lcd.clear();
      lcd.print("Erro de captura");
    }
    
    // Retorna à tela "Aguardando dedo" após qualquer situação
    lcd.clear();
    lcd.print("Aguardando dedo...");
    delay(100); // Atraso entre as tentativas
  }
}

uint8_t readNumber() {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}