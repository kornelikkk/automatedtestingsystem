#include <SoftwareSerial.h>
#include <GyverPlanner2.h>
#include <EEPROM.h>

int32_t StepsPerRev = 1600;
String str = "__";

// Координаты вращения головы
int32_t path[][3] = {
  {0, 0, 0},
  {0, 1600, 0},
  {-10000, 800, -8000},
  {-33000, 800, -16000},
  {-42000, 1600, -24000},
  {-33000, 800, -16000},
  {-10000, 800, -8000},
  {0, 1600, 0},
};

const int8_t nodeAmount = sizeof(path) / 12; // Количество точек. Определяется как вес всего массива / (4*3) байта
//int8_t nodeAmount = 8;

//Объявление концевиков
const int conPin0 = 3;
const int conPin1 = 4;
const int conPin2 = 2;

// Объявление дисплея
SoftwareSerial serialDisplay(26, 27); //rx tx

int16_t adress = 0;

#define max_adress 4094 //uno == 1023     mega == 4095
#define max_cycle 100000
#define max_value 1000000


//Объявление шаговиков
Stepper<STEPPER2WIRE> stepper1(5, 6); 
Stepper<STEPPER2WIRE> stepper2(8, 9);
Stepper<STEPPER2WIRE> stepper3(11, 12);
GPlanner2<STEPPER2WIRE, 3, nodeAmount> planner;

int j = 0; // Счётчик точек маршрута

void setup() {
  Serial.begin(9600);
  serialDisplay.begin(9600);

 // Экран
  displayCounterOn();

  // Шаговики
  planner.setAcceleration( 400 );      // Ускорение
  planner.setMaxSpeed( 2200 );         // Максимальная скорость

  // Настройка планировщика
  planner.addStepper(0, stepper1);  // ось 0
  planner.addStepper(1, stepper2);  // ось 1
  planner.addStepper(2, stepper3);  // ось 2

  // Концевики
  pinMode(conPin0, INPUT);
  pinMode(conPin1, INPUT);
  pinMode(conPin2, INPUT);

  // enPin
  //pinMode(7, OUTPUT);
  //pinMode(10, OUTPUT);
  //pinMode(13, OUTPUT);

  homing();
  planner.setCurrent(path[0]); // Установка нулевых координат
  planner.start();
}

//-----------------------------Шаговики------------------------------------------------

bool first_on = true;
void loop() {

  if (first_on){
    first_on=false;
    delay(1000);
  }

  planner.tick(); // Тикер для управления шаговиками

  if (planner.available()) {
    // добавляем точку маршрута и является ли она точкой остановки (0 - нет)
      planner.addTarget(path[j], 0, ABSOLUTE);
       ++j;
       if ( j == 5 ) {
         displayCounter();
         //Serial.println("check#1+1");
       }
       if ( j >= nodeAmount ){
          j = 1;
          displayCounter();
          //Serial.println("check#2+1");
       }
  }
}

void yield() {; 
   if (planner.available()) {
      planner.addTarget(path[j], 0, ABSOLUTE);
      ++j;
      if ( j >= nodeAmount - 1 ){
        j = 1;
      }
    }
   Serial.println(j);
}


//--------------Концевики---------------------------------------------------------------

void homing() {
  int count_conPin0 = 0;
  int count_conPin2 = 0;
  //Serial.println(str + sizeof(path) + str);

  planner.setSpeed(1, -800);
  while ( count_conPin2 < 3000 ) {
    planner.tick();
    if ( digitalRead( conPin2 ) == 0 ) ++count_conPin2;
  }
  planner.brake();


  planner.setSpeed(0, 800);      // ось 0
  while ( count_conPin0 < 15000 ) {
    planner.tick();
    if ( digitalRead( conPin0 ) == 0 ) ++count_conPin0;
  }
  planner.brake();                // тормозим, приехали

  planner.reset();    // сбрасываем координаты в 0
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