#include <Servo.h>  // Biblioteca pentru controlul servo-urilor

Servo myServo;      // Creăm un obiect pentru servo

void setup() {
  myServo.attach(9);  // Conectăm servo-ul la pinul digital 9
}

void loop() {
  // Mișcare lină de la 0° la 180°
  for (int angle = 0; angle <= 180; angle++) {
    myServo.write(angle);  // Setăm unghiul
    delay(15);             // Așteptăm pentru ca servo-ul să se miște
  }

  // Mișcare lină de la 180° la 0°
  for (int angle = 180; angle >= 0; angle--) {
    myServo.write(angle);  // Setăm unghiul
    delay(15);             // Așteptăm pentru ca servo-ul să se miște
  }

  delay(1000);  // Pauză de 1 secundă între cicluri
}
