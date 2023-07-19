#include <SoftwareSerial.h>
#include <GyverPlanner2.h>
#include <EEPROM.h>

// Координаты вращения головы
int32_t path[][1] = {0, 4000, 800};

const int8_t nodeAmount = 2; // Количество точек. Определяется как вес всего массива / (4*3) байта

// Объявление дисплея
SoftwareSerial serialDisplay(0, 1); //rx tx

int16_t adress = 0;

#define max_adress 4094 //uno == 1023     mega == 4095
#define max_cycle 100000
#define max_value 1000000

//Объявление шаговиков
Stepper<STEPPER2WIRE> stepper1(50, 51); 
GPlanner2<STEPPER2WIRE, 1, nodeAmount> planner;

int j = 0; // Счётчик точек маршрута

void setup() {
  Serial.begin(9600);
  serialDisplay.begin(9600);

 // Экран
  displayCounterOn();

  // Шаговики
  planner.setAcceleration( 800 );      // Ускорение
  planner.setMaxSpeed( 1600 );         // Максимальная скорость

  // Настройка планировщика
  planner.addStepper(0, stepper1);  // ось 0

  // Базирование и обнуление координат
  planner.setCurrent(path[0]); // Установка нулевых координат
  planner.start();
}

//-----------------------------Шаговики------------------------------------------------

bool first_on = true;
void loop() {
  //Serial.println(nodeAmount);
  if (first_on){
    first_on=false;
    delay(1000);
  }

  planner.tick(); // Тикер для управления шаговиками

  if (planner.available()) {
    // добавляем точку маршрута и является ли она точкой остановки (0 - нет)
      planner.addTarget(path[j], 0, ABSOLUTE);
      ++j;
      if ( j >= nodeAmount ){
          j = 1;
          //Serial.println("check main");
          displayCounter();
       }
  }
}

void yield() {; 
   if (planner.available()) {
      planner.addTarget(path[j], 0, ABSOLUTE);
      ++j;
      if ( j >= nodeAmount - 1 ){
        //Serial.println("check koncevik");
        j = 1;
      }
    }
}


//--------------Дисплей---------------------------------------------------------------
// Отображение на дисплей (от Али)

void comandEnd() {
  for (int i = 0; i < 3; i++) {
    serialDisplay.write(0xff);
  }
}

void displayCounterOn() {
  long k;
  EEPROM.get(max_adress-4,k);
  if(k!=0){
    for (int i = 0; i<max_adress+1; i += 1)
     { 
      EEPROM.write(i,0);
     }
  }
  int adr;
  while(true)
  {
    EEPROM.get(adress+4, adr);
    if (adr >= max_cycle)
    {
      adress+=6;
    }else
    {
      break;
    }
  }
  long a;
  EEPROM.get(adress,a);
  String str = String(a);
  serialDisplay.print("page0.counter.val=" + str);
  comandEnd();
}

void displayCounter(){ 
  long a;
  EEPROM.get(adress,a);
  Serial.print(a);
  int16_t b;
  EEPROM.get(adress + 4, b);
  if (b >= max_cycle-1)
  {
     if (adress + 6 >= max_adress-6)
     {
      for (int i = 0; i<max_adress+1; i += 1)
      {
        EEPROM.write(i,0);
      }
      adress = 0;
      EEPROM.put(adress, a);
     }else
     {
      adress += 6;
      b = 0;
      a++;
      EEPROM.put(adress, a);
      EEPROM.put(adress + 4, b);
      String str = String(a);
      serialDisplay.print("page0.counter.val=" + str);
      comandEnd();
      return;
     }
  }
  if (a >= max_value)
  {
    b++;
    EEPROM.put(adress, 0);
    EEPROM.put(adress + 4, b);
    String str = String(a);
    serialDisplay.print("page0.counter.val=" + str);
    comandEnd();
    while(true){ }
  }
  a++;
  b++;
  EEPROM.put(adress, a);
  EEPROM.put(adress + 4, b);
  String str = String(a);
  serialDisplay.print("page0.counter.val=" + str);
  comandEnd();
}