Command signature:

command(int) arg1(float) arg2(float) arg3(float)

The command and arguments must be separated by a space from the start character and the end character.

Command list: 
Disable - 0, None arg command.
Enable - 1, None arg command.
Turn by angle - 2, The first argument is required, the rest are optional.
Stop - 3, None arg command.
Brake - 4, None arg command.
Rotate - 5, One arg command.
MaxSpeed - 6, One arg command.
Acceleration - 7, One arg command. if set 0 acc == max
Status - 8, None arg command.
To zero point - 9, None arg command.
Restart - 10, None arg command.

Status list:
IDLE- 0.
Turning- 1.
Go to pause - 2.
Always rotate - 3 
Stoping - 4

example:
8 - get status
1 - enable 
2 360 800 200 - movement 360 angls with a maximum speed of 800 and acceleration of 200
2 360 -  movement 360 angls
5 400 - always rotate 400 steps per second

Status, Brake, Stop  executed out of order, others in queue.

After the move command, the number of motor steps is written to the port.

