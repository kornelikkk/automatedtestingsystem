#include <SoftwareSerial.h>

SoftwareSerial serialDisplay(6,7); //rx tx

void setup() {
  Serial.begin(9600);
  serialDisplay.begin(9600);
}

void loop() {
    if(false)
    {
      displayCounter()//очень прошу быть осторожным, кол-во записаей ограничено. 
      //Память можно перезаписать 10000^(1022/4) раз. 
      //если будет бесконечный цикл, то eeprom сломается
    }
}

void comandEnd() {
  for (int i = 0; i < 3; i++) {
    serialDisplay.write(0xff);
  }
}
void displayCounter(){ 
  serialDisplay.print("page0.counter.val=page0.counter.val+1");
  comandEnd();
}