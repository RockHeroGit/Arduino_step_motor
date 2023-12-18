//#define GS_NO_ACCEL
#include <avr/wdt.h>
#include "GyverStepper2.h"
#include "StringArray.h"

void restartArduino()
{
  wdt_enable(WDTO_15MS);
  while (1) {}
}

enum Commands
{
  DISABLE,
  ENABLE,
  TURN_BY_ANGLE,
  STOP,
  BRAKE,
  ROTATE,
  MAXSPEED,
  ACCELERATION,
  STATUS,
  TOZEROPOINT,
  RESTART
};

enum Control
{
  COMMAND,
  ARG1,
  ARG2,
  ARG3
};

enum MotorStatus
{
  IDLE,
  TURNING,
  GO_TO_PAUSE,
  ROTATING, // автор либы имеет в виду Speed как безпрерывное движение
  STOPING
};

class MotorControll
{
private:
  int const MOTOR_STEPS = 2048*2; // При HALF mode n*2
  float const ANGLE_PER_STEP = 360.0 / MOTOR_STEPS;
  int const MOTOR_PIN1 = 5; // для мотора 28BYJ-48 первый и последний пин меняются местами
  int const MOTOR_PIN2 = 6;
  int const MOTOR_PIN3 = 9;
  int const MOTOR_PIN4 = 10;
  const int NPOS = -1;

  DataArray data;
  QueueArray queue;

  String separator;
  String word;
  String string_parse;

public:
  GStepper2<STEPPER4WIRE_HALF> stepper;
  //GStepper2<STEPPER2WIRE_HALF> stepper;

  MotorControll() : stepper(MOTOR_STEPS, MOTOR_PIN4, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN1), separator(" ")
  {
    stepper.enable();
    stepper.setAcceleration(0); // Максимальная ускорение
    stepper.setMaxSpeed(800); // При HALF mod n*2
    //stepper.setTarget(0);
    //stepper.autoPower(true); // раскоментить для автовыключения по достижению позиции (нарушает работу callback)
  }

  void parse() 
  {
    String string_parse = Serial.readString();

    data._clearData();

    int index = 0;
    while (index != NPOS && string_parse.length() > 0)
    {
      index = string_parse.indexOf(separator);
      word = string_parse.substring(0, index);
      string_parse = string_parse.substring(index + 1);

      data.push_back(word.toFloat());

      //if (word.length() > 0) {Serial.println(word);} //debug
    }
    if(data.size() > 0)
    {
      queue.push_back(data);
      String responce = "The command accepted";
      Serial.println(responce);
    }
    //for(int i = 0; i < data.size() ; i++) {Serial.println(data[i]);} //debug
  }

  int convertAngleToSteps(float acceleration_angle)
  {
    float acceleration_steps = acceleration_angle / ANGLE_PER_STEP;
    return (int)acceleration_steps;
  }

  bool isCommandValid(String& command) { return (command.startsWith("$") && command.endsWith(";")); }

  void axisMove(float ang) { stepper.setTargetDeg(ang, RELATIVE); }

  void setAccel(int value) { stepper.setAcceleration(value); }

  void setSpeed(float value) { stepper.setSpeedDeg(value); }

  void setMaxSpeed(float value) { stepper.setMaxSpeedDeg(value); }

  void tick() { stepper.tick(); } // необходимо вызывать как можно чаще 

  void getStatus() { Serial.println("Current status: (" + String(stepper.getStatus()) + ")");}

  void toZeroPoint() { stepper.setTarget(0); }

  bool stopCheck()
  {
    if(queue.size() > 0 &&
      (int)queue[queue.size()-1][Control::COMMAND] == Commands::STOP ||
      (int)queue[queue.size()-1][Control::COMMAND] == Commands::BRAKE)
     {
        DataArray command = queue.pop_back();

        switch((int)command[Control::COMMAND])
        {
          case Commands::STOP:
            {
              stepper.stop();
              break;
            }
          case Commands::BRAKE:
            {
              stepper.brake();
              break;
            }  
        }
        queue.clearQueue();
        return true;

     }
     else
      return false;
  }

  bool statusCheck()
  {
    if(queue.size() > 0 &&
      (int)queue[queue.size()-1][Control::COMMAND] == Commands::STATUS)
    {
      DataArray command = queue.pop_back();
       
      switch((int)command[Control::COMMAND])
      {
        case Commands::STATUS:
          {
            getStatus();
            break;
          }
      }
        return true;

    }
    else
      return false;

  }

  void execute()
  {
    if(statusCheck())
      return;

    if(stopCheck())
      return;
  
    if(queue.size() > 0 && stepper.getStatus() == MotorStatus::IDLE)
    {
      DataArray command = queue.pop();

      switch((int)command[Control::COMMAND])
      {
        case Commands::DISABLE:
        {
          stepper.disable();
          break;
        }
        case Commands::ENABLE:
        {
          stepper.enable();
          break;
        }
        case Commands::TURN_BY_ANGLE:
        {
          if( command[Control::ARG2] != -1 )
            setMaxSpeed(command[Control::ARG2]);
          if( command[Control::ARG3] != -1 )
            setAccel(convertAngleToSteps(command[Control::ARG3])); //float

          axisMove(command[Control::ARG1]);
          break;
        }
        case Commands::ROTATE: // установить скорость в шагах/сек (float) и запустить вращение
        {
          setSpeed(command[Control::ARG1]);
          break;
        }
        case Commands::MAXSPEED:
        {
          setMaxSpeed(command[Control::ARG1]);
          break;
        }
        case Commands::ACCELERATION:
        {
          setAccel(convertAngleToSteps(command[Control::ARG1]));
          break;
        }
        case Commands::TOZEROPOINT:
        {
          toZeroPoint();
          break;
        }
        case Commands::RESTART:
        {
          restartArduino();
          break;
        }
        default:
        {
          //Serial.println("Unidentified command");
          Serial.println("Unidentified command. Check command list:\nDisable - 0\nEnable - 1\Turn by angle - 2\nStop - 3\nBrake - 4\nRotate - 5\nMaxSpeed - 6\nAcceleration -7\nStatus - 8\nTo zero point - 9\nRestart - 10");
          // Увы данная строка вешает ардуино Uno, вывод всех строк начинает бится из за нехватки памяти...
          break;
        } 
      }
    }
  }

  void callBack() // answer response
  {
    if(stepper.ready())
    {
      String currentPosition = "(" + String(stepper.getCurrent()) + ")";
      Serial.println("The movement is over. Current pos in steps: " + currentPosition);
    }
  }

} motor;

void setup() 
{
  Serial.begin(115200);
  Serial.setTimeout(20);
}

void loop() 
{
  motor.tick();
  motor.callBack();
  if (Serial.available() > 0)
    motor.parse();
  motor.execute();

}
