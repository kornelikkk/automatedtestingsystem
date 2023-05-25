
#include <SoftwareSerial.h>
#include <GyverPlanner2.h>
#include <GyverStepper2.h>
#define GS_FAST_PROFILE 10

// Координаты вращения головы
int pathFDir[][3] = {
  {0, 0, 0},
  {6400, 6400, 0},
  {12800, 12800, 0},
};
int pathSDir[][3] = {
  {12800, 12800, 0},
  {6400, 6400, 0},
  {0, 0, 0},
};

int nodeAmount = sizeof(pathFDir) / 6; // Количество точек. Определяется как вес всего массива / (2+2) байта

//Объявление концевиков
int conPin0 = 2;
int conPin1 = 3;
int conPin2 = 4;

// Объявление дисплея
SoftwareSerial serialDisplay(0,1); //rx tx

//Объявление шаговиков
GStepper2<STEPPER2WIRE> stepper1(6400, 5, 8); 
GStepper2<STEPPER2WIRE> stepper2(6400, 6, 9);
GStepper2<STEPPER2WIRE> stepper3(6400, 7, 10);
GPlanner2<STEPPER2WIRE, 3> planner;


void setup() {
  Serial.begin(9600);
  serialDisplay.begin(9600);

  // Шаговики
  planner.setAcceleration(250);      // Ускорение
  planner.setMaxSpeed(1000);         // Максимальная скорость
  
  // Настройка планировщика
  planner.addStepper(0, stepper1);  // ось 0
  planner.addStepper(1, stepper2);  // ось 1
  planner.addStepper(2, stepper3);  // ось 2

  // Концевики
  pinMode(conPin0, INPUT);
  pinMode(conPin1, INPUT);
  pinMode(conPin2, INPUT);

  homing(); // Приходим в нулевые координаты (до концевиков)
  planner.setCurrent(pathFDir[0]); // Установка нулевых координат
  planner.start();
}



//-----------------------------Шаговики------------------------------------------------
//Основное управление шаговиками
int count = 0; // Счётчик точек маршрута
bool dir = 1; // Направление вращения
void loop() {
  planner.tick(); // Тикер для управления шаговиками


  if (planner.available()) {
    // добавляем точку маршрута и является ли она точкой остановки (0 - нет)
    if ( dir ){
      planner.addTarget(pathFDir[count], 0, ABSOLUTE);
      if ( ++count >= nodeAmount ) {
        count = 0; // Закольцевать
        dir != dir; // Смена направления
      }
    }
    else{
      planner.addTarget(pathSDir[count], 0, ABSOLUTE);
      if ( ++count >= nodeAmount ) {
        count = 0; // Закольцевать
        dir != dir; // Смена направления
      }
    }
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
    planner.setSpeed(0, -10);       // ось 0, -10 шаг/сек
    while (digitalRead(conPin0)) {  // пока кнопка не нажата
      planner.tick();               // крутим
    }
    // кнопка нажалась - покидаем цикл
    planner.brake();                // тормозим, приехали
  }
  
  if (digitalRead(conPin1)) {   
    planner.setSpeed(1, -10);       
    while (digitalRead(conPin1)) planner.tick();
    planner.brake();                
  }

  if (digitalRead(conPin2)) {       
    planner.setSpeed(2, -10);       
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