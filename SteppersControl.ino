
#include <SoftwareSerial.h>
#include <GyverPlanner2.h>
//#include <GyverStepper2.h>
//#define GS_FAST_PROFILE 100

int32_t StepsPerRev = 1600;

// Координаты вращения головы
int32_t path[][3] = {
  {0, 0, 0},
  {-43000, -16000, -1600},
  {0, 0, 0}
};

int nodeAmount = sizeof(path) / 12; // Количество точек. Определяется как вес всего массива / (4*3) байта

//Объявление концевиков
const int conPin0 = 2;
const int conPin1 = 3;
const int conPin2 = 4;

// Объявление дисплея
SoftwareSerial serialDisplay(0,1); //rx tx

//Объявление шаговиков
Stepper<STEPPER2WIRE> stepper1(5, 6); 
Stepper<STEPPER2WIRE> stepper2(7, 8);
Stepper<STEPPER2WIRE> stepper3(9, 10);
GPlanner2<STEPPER2WIRE, 3> planner;


void setup() {
  Serial.begin(9600);
  serialDisplay.begin(9600);

  // Шаговики
  planner.setAcceleration( 400 );      // Ускорение
  planner.setMaxSpeed( 2400 );         // Максимальная скорость
  
  // Настройка планировщика
  planner.addStepper(0, stepper1);  // ось 0
  planner.addStepper(1, stepper2);  // ось 1
  planner.addStepper(2, stepper3);  // ось 2

  // Концевики
  pinMode(conPin0, INPUT);
  pinMode(conPin1, INPUT);
  pinMode(conPin2, INPUT);

  homing(); // Приходим в нулевые координаты (до концевиков)
  planner.setCurrent(path[0]); // Установка нулевых координат
  planner.start();
}



//-----------------------------Шаговики------------------------------------------------
//Основное управление шаговиками
int count = 0; // Счётчик точек маршрута

void loop() {
  planner.tick(); // Тикер для управления шаговиками
//  Serial.println(digitalRead(conPin0));

  if (planner.available()) {
    // добавляем точку маршрута и является ли она точкой остановки (0 - нет)
      planner.addTarget(path[count], 0, ABSOLUTE);
      if ( ++count >= nodeAmount ) count = 0; // Закольцевать
  }

  // Дисплей
  if(false) {
      displayCounter();//очень прошу быть осторожным, кол-во записаей ограничено. 
      //Память можно перезаписать 10000^(1022/4) раз. 
      //если будет бесконечный цикл, то eeprom сломается
    };
}



//--------------Концевики---------------------------------------------------------------
// Проверка концевиков
void homing() {
  if (digitalRead(conPin0)) {       // если концевик 0 не нажат
    planner.setSpeed(0, 1600);       // ось 0
    while (digitalRead(conPin0)) {  // пока кнопка не нажата
      planner.tick();               // крутим
    }
    // кнопка нажалась - покидаем цикл
    planner.brake();                // тормозим, приехали
  }
  
//  if (digitalRead(conPin1)) {   
//    planner.setSpeed(1, 1600);       
//    while (digitalRead(conPin1)) planner.tick();
//    planner.brake();                
//  }

  if (digitalRead(conPin2)) {       
    planner.setSpeed(2, 800);       
    while (digitalRead(conPin2)) planner.tick();               
    planner.brake();                
  }

  planner.reset();    // сбрасываем координаты в 0
}



//--------------Дисплей---------------------------------------------------------------
// Отображение на дисплей (от Али)
void commandEnd() {
  for (int i = 0; i < 3; i++) {
    serialDisplay.write(0xff);
  }
}

void displayCounter(){ 
  serialDisplay.print("page0.counter.val=page0.counter.val+1");
  commandEnd();
}