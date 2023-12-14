//#define GS_NO_ACCEL
#include "GyverStepper2.h"
#include "StringArray.h"

enum Commands
{
  DISABLE,
  ENABLE,
  MOVE,
  STOP,
  BRAKE,
  SPEED,
  MAXSPEED,
  ACCELERATION,
  STATUS
};

enum Control
{
  ENTERCHAR,
  COMMAND,
  ARG1,
  ARG2,
  ARG3,
  ENDCHAR
};

enum MotorStatus
{
  WAIT,
  GOING,
  GO_TO_PAUSE,
  ROTATE_FROM_SPEED, // автор либы имеет в виду Speed как безпрерывное движение
  STOPING
};

class MotorControll
{
private:
  int const MOTOR_STEPS = 2048*2; // При HALF mode n*2
  int const MOTOR_PIN1 = 5; // для мотора 28BYJ-48 первый и последний пин меняются местами
  int const MOTOR_PIN2 = 6;
  int const MOTOR_PIN3 = 9;
  int const MOTOR_PIN4 = 10;
  const int NPOS = -1;
  DataArray data;
  QueueArray queue;

  String separator;
  String end_command;
  String word;
  String string_parse;

  void errorHandler(String &string_parse) { Serial.println("Command \""+ string_parse +"\", is not valid. Try use pattern \"$ command arg ;\"");}

public:
  GStepper2<STEPPER4WIRE_HALF> stepper;

  MotorControll() : stepper(MOTOR_STEPS, MOTOR_PIN4, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN1), end_command(";"), separator(" ")
  {
    stepper.enable();
    stepper.setAcceleration(0); // Максимальная ускорение
    stepper.setMaxSpeed(800); // При HALF mod n*2
    stepper.setTarget(0);
    //stepper.autoPower(true); // раскоментить для автовыключения по достижению позиции (нарушает работу callback)
  }

  void parse() 
  {
    String string_parse = Serial.readString();
    if(!isCommandValid(string_parse)) 
    {
      errorHandler(string_parse);
      return;
    }
  
    data._clearData();

    int index = 0;
    while (index != NPOS && string_parse.length() > 0)
    {
      index = string_parse.indexOf(separator);
      word = string_parse.substring(0, index);
      string_parse = string_parse.substring(index + 1);
      
      data.push_back(word);
      
      if(word.equals(end_command))
        break;

      //if (word.length() > 0) {Serial.println(word);} //debug
    }
    if(word.length() > 0)
      queue.push_back(data);
    //for(int i = 0; i < data.size() ; i++) {Serial.println(data[i]);} //debug
  }

  bool isCommandValid(String& command) { return (command.startsWith("$") && command.endsWith(";")); }

  void axisMove(float ang) { stepper.setTargetDeg(ang, RELATIVE); }

  void setAccel(int value) { stepper.setAcceleration(value); }

  void setSpeed(int value) { stepper.setSpeed(value); }

  void setMaxSpeed(int value) { stepper.setMaxSpeed(value); }

  void tick() { stepper.tick(); } // необходимо вызывать как можно чаще 

  void getStatus() { Serial.println("Current status: (" + String(stepper.getStatus()) + ")");}

  bool stopCheck()
  {
    if(queue.size() > 0 &&
      queue[queue.size()-1][Control::COMMAND].toInt() == Commands::STOP ||
      queue[queue.size()-1][Control::COMMAND].toInt() == Commands::BRAKE)
     {
        DataArray command = queue.pop_back();

        switch(command[Control::COMMAND].toInt())
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
      queue[queue.size()-1][Control::COMMAND].toInt() == Commands::STATUS)
    {
      DataArray command = queue.pop_back();
       
      switch(command[Control::COMMAND].toInt())
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
  
    if(queue.size() > 0 && stepper.getStatus() == MotorStatus::WAIT)
    {
      DataArray command = queue.pop();

      switch(command[Control::COMMAND].toInt())
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
        case Commands::MOVE:
        {
          axisMove(command[Control::ARG1].toFloat());

          if(command[Control::ARG2].length() > 0 && !command[Control::ARG2].equals(end_command))
            setMaxSpeed(command[Control::ARG2].toInt());
          if(command[Control::ARG3].length() > 0 && !command[Control::ARG3].equals(end_command))
            setAccel(command[Control::ARG3].toInt());

          break;
        }
        case Commands::SPEED: // установить скорость в шагах/сек (float) и запустить вращение
        {
          setSpeed(command[Control::ARG1].toFloat());
          break;
        }
        case Commands::MAXSPEED:
        {
          setMaxSpeed(command[Control::ARG1].toInt());
          break;
        }
        case Commands::ACCELERATION:
        {
          setAccel(command[Control::ARG1].toInt());
          break;
        }
        default:
        {
          Serial.println("Unidentified command");
          //Serial.println("Unidentified command. Check command list:\nDisable - 0\nEnable - 1\nMove - 2\nStop - 3\nBrake - 4\nSpeed - 5\nMaxSpeed - 6\nAcceleration -7\nStatus - 8");
          // Увы данная строка вешает ардуино Uno, вывод всех строк начинает бится из за нехватки памяти...
          break;
        } 
      }
    }
  }

  void callBack()
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
  Serial.begin(9600);
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
