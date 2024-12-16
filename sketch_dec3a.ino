#include <Servo.h>

Servo myservo;
int angle = 90;             // Начальный угол серво — 90 градусов, принимаем за условный ноль.
int maxAngle = 90;          // Максимальное отклонение от 90 градусов (0 до 90 или 90 до 180).
int delayTimeUp = 20;       // Начальная задержка при возвращении к 90 (умножается на множитель).
int delayTimeDown = 20;     // Начальная задержка при движении от 90 до maxAngle (умножается на множитель).
float multiplierUp = 1.0;   // Множитель для задержки delayTimeUp.
float multiplierDown = 1.0; // Множитель для задержки delayTimeDown.

#define LED_PIN 13
#define BUTTON_PIN 2

byte lastButtonState = LOW;
byte currentButtonState = LOW;
byte ledState = LOW;

unsigned long debounceDuration = 50; // Длительность антидребезга для кнопки (50 миллисекунд).
unsigned long lastTimeButtonStateChanged = 0;
bool isServoRunning = false;
int phase = 0;  // Фаза движения: 0 — влево, 1 — возврат влево, 2 — вправо, 3 — возврат вправо.

void setup() {
  myservo.attach(9);         // Подключение сервопривода к пину 9.
  pinMode(LED_PIN, OUTPUT);  // Установка пина LED_PIN (13) как выход.
  pinMode(BUTTON_PIN, INPUT);// Установка пина BUTTON_PIN (2) как вход.
  Serial.begin(9600);        // Инициализация последовательного порта с скоростью 9600 бод.
  Serial.setTimeout(50);     // Установка тайм-аута для чтения данных из Serial (50 миллисекунд).
  
  // Устанавливаем серво в начальное положение (90 градусов) при подаче питания
  myservo.write(angle);      
  Serial.println("Servo initialized at 90 degrees.");
  Serial.println("Setup complete. Ready to start.");
}

void loop() {
  unsigned long currentTime = millis(); // Получение текущего времени в миллисекундах с момента старта программы.

  // Обработка кнопки с антидребезгом
  if (currentTime - lastTimeButtonStateChanged > debounceDuration) {
    currentButtonState = digitalRead(BUTTON_PIN);
    if (currentButtonState != lastButtonState) {
      lastButtonState = currentButtonState;
      if (currentButtonState == LOW) {
        ledState = (ledState == HIGH) ? LOW : HIGH;
        digitalWrite(LED_PIN, ledState);
        isServoRunning = !isServoRunning;  // Переключаем состояние сервопривода.
        
        if (isServoRunning) {
          Serial.println("Servo started.");
          Serial.println("Waiting for 5 seconds before starting the movement...");
          delay(5000);                // Задержка перед началом движения
          phase = 0;                  // Начинаем цикл с движения влево
        } else {
          Serial.println("Servo stopped. Returning to center.");
          // Возвращаем серво в центр при остановке
          while (angle != 90) {
            if (angle < 90) {
              angle++;
            } else if (angle > 90) {
              angle--;
            }
            myservo.write(angle);
            delay(20); // Мягкая задержка при возврате
          }
        }
      }
      lastTimeButtonStateChanged = currentTime;
    }
  }

  // Проверка наличия данных в Serial для изменения параметров
  if (Serial.available()) {
    char command = Serial.read(); // Читаем команду (первый символ).
    float val = Serial.parseFloat();  // Считываем число с плавающей точкой, которое следует за командой.

    if (command == 'A') { // Команда для изменения угла.
      if (val > 0 && val <= 90) { // Ограничение угла до 90 градусов.
        maxAngle = val;
        Serial.print("Max angle set to: ");
        Serial.println(maxAngle);
      } else {
        Serial.println("Invalid angle value! Please enter a value between 1 and 90.");
      }
    } else if (command == 'S') { // Команда для изменения множителей задержки.
      if (val > 0) {
        multiplierUp = val;      // Устанавливаем множитель для delayTimeUp.
        multiplierDown = val;    // Устанавливаем множитель для delayTimeDown.
        Serial.print("Multiplier set for both delay times: ");
        Serial.println(val);
      } else {
        multiplierUp = 1.0;      // Сбрасываем множитель к 1, если не введено значение.
        multiplierDown = 1.0;
        Serial.println("Multipliers set to default (1.0).");
      }
    } else if (command == 'U') { // Команда для изменения множителя для delayTimeUp.
      if (val > 0) {
        multiplierUp = val;      // Устанавливаем множитель для delayTimeUp.
        Serial.print("Multiplier for delayTimeUp set to: ");
        Serial.println(multiplierUp);
      } else {
        Serial.println("Invalid multiplier value! Please enter a value greater than 0.");
      }
    } else if (command == 'D') { // Команда для изменения множителя для delayTimeDown.
      if (val > 0) {
        multiplierDown = val;    // Устанавливаем множитель для delayTimeDown.
        Serial.print("Multiplier for delayTimeDown set to: ");
        Serial.println(multiplierDown);
      } else {
        Serial.println("Invalid multiplier value! Please enter a value greater than 0.");
      }
    } else {
      Serial.println("Unknown command!Use'A'for angle,'S'for both multipliers,'U'for up multiplier,or'D'for down multiplier.");
    }
  }

  // Движение сервопривода, если он активен
  if (isServoRunning) {
    switch (phase) {
      case 0: // Движение от 90 до 0 градусов.
        if (angle > (90 - maxAngle)) {
          angle--;
          myservo.write(angle);
          Serial.print("Delaying Down: ");
          Serial.println(delayTimeDown * multiplierDown);
          delay(delayTimeDown * multiplierDown);
        } else {
          phase = 1; // Переход к фазе возврата к 90.
        }
        break;

      case 1: // Возвращение к 90 градусов.
        if (angle < 90) {
          angle++;
          myservo.write(angle);
          Serial.print("Delaying Up: ");
          Serial.println(delayTimeUp * multiplierUp);
          delay(delayTimeUp * multiplierUp);
        } else {
          phase = 2; // Переход к фазе движения вправо.
        }
        break;

      case 2: // Движение от 90 до 180 градусов.
        if (angle < (90 + maxAngle)) {
          angle++;
          myservo.write(angle);
          Serial.print("Delaying Down: ");
          Serial.println(delayTimeDown * multiplierDown);
          delay(delayTimeDown * multiplierDown);
        } else {
          phase = 3; // Переход к фазе возврата к 90.
        }
        break;

      case 3: // Возвращение к 90 градусов.
        if (angle > 90) {
          angle--;
          myservo.write(angle);
          Serial.print("Delaying Up: ");
          Serial.println(delayTimeUp * multiplierUp);
          delay(delayTimeUp * multiplierUp);
        } else {
          phase = 0; // Переход на начальную фазу движения влево.
        }
        break;
    }
  }
}
      
