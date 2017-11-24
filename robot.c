const tSensors SOUND_SENSOR = S1;
const tSensors COLOUR_SENSOR = S2;
const tMotor UNROLL_MOTOR = motorB;
const tMotor TRACK_MOTOR = motorA;
const tMotor FOLDER_MOTOR = motorC;
const tMotor PISTON_MOTOR = motorD;

const int SOUND_THRESHOLD = 45; //TESTED & PASSED
const int MAX_LAYERS = 8;
const int MAX_WAIT_TIME = 2500; //ms
const int DRIVEN_WHEEL_RADIUS = 2; //cm
const int TEAR_DISTANCE = -30; //cm
const int REV_UNROLL_POWER = 100;
const int FWD_UNROLL_POWER = 50;
const int FOLDARM_LIMIT = 115; //degrees
const int TRACK_POWER = -100;
int layersUsed = 0;

bool paperExists()
{
  while (SensorValue[COLOUR_SENSOR] == (int)colorRed)
  {
    displayTextLine(0, "Press a button to acknowledge the roll is empty,");
    displayTextLine(1, "or place a new roll in the designated place.");
    playSound(soundDownwardTones);
    time1[T1] = 0;
    while (time1[T1] < MAX_WAIT_TIME)
    {
      if (getButtonPress(buttonAny))
        return false;
    }
  }
  displayTextLine(0, "");
  displayTextLine(1, "");
  return true;
}

void pistonUp(bool direction)
{
  int position = 10.5*360, motorPower = 100; //modify==============================================================
  if (!direction)
    motorPower = -motorPower;

  motor[PISTON_MOTOR] = motorPower;
  nMotorEncoder[PISTON_MOTOR] = 0;
  if (!direction)
    while (nMotorEncoder[PISTON_MOTOR] > -position)
    {}
  else
    while (nMotorEncoder[PISTON_MOTOR] < position)
    {}

  motor[PISTON_MOTOR] = 0;
}

void foldArm135CW(bool rotateCW)
{
  int power = 50; //modify

  if (!rotateCW)
    power = -power;
  nMotorEncoder[FOLDER_MOTOR] = 0;
  motor[FOLDER_MOTOR] = power;

  if (rotateCW)
    while (nMotorEncoder[FOLDER_MOTOR] < FOLDARM_LIMIT)
    {}
  else
    while (nMotorEncoder[FOLDER_MOTOR] > -FOLDARM_LIMIT)
    {}

  motor[FOLDER_MOTOR] = 0;
}

void unrollPaper(int distanceCM, int motorPower, bool unroll)
{
  if (motorPower > 100)
    motorPower = 100;
  else if (motorPower < 0)
    motorPower = -motorPower;
  if (distanceCM < 0)
    distanceCM = -distanceCM;

  if (unroll)
    motorPower = -motorPower;
  else
    distanceCM = -distanceCM;

  nMotorEncoder[UNROLL_MOTOR] = 0;
  int encoderStop = distanceCM * 360 / (2*PI*DRIVEN_WHEEL_RADIUS);
  motor[UNROLL_MOTOR] = motorPower;

  if (unroll)
    while (nMotorEncoder[UNROLL_MOTOR] > -encoderStop)
    {}
  else
    while (nMotorEncoder[UNROLL_MOTOR] < encoderStop)
    {}

  motor[UNROLL_MOTOR] = 0;
}

/*void unrollPaper(int distanceCM, int motorPower)
{
  if (motorPower > 100)
    motorPower = 100;
  else if (motorPower < 0)
    motorPower = 0;
  if (distanceCM > 0)
    motorPower = -motorPower;
  else
    distanceCM = -distanceCM;

  nMotorEncoder[UNROLL_MOTOR] = 0;
  int encoderStop = distanceCM * 360 / (2*PI*DRIVEN_WHEEL_RADIUS);
  motor[UNROLL_MOTOR] = motorPower;

  if (distanceCM > 0)
    while (nMotorEncoder[UNROLL_MOTOR] > -encoderStop)
    {}
  else
    while (nMotorEncoder[UNROLL_MOTOR] < encoderStop)
    {}

  motor[UNROLL_MOTOR] = 0;
}*/

void outputPaper()
{
  rotate135CW(true);
  pistonUp(false);
  unrollPaper(TEAR_DISTANCE, REV_UNROLL_POWER);
  pistonUp(true);
  rotate135CW(false);
}

bool failsafe(){return true;} //ANDREW


void fold(int layers) //ANDREW DO DEBUGGING HERE
{
  unrollPaper(5, FWD_UNROLL_POWER); //just past the fold arm
  wait1Msec(300);

  for(; layers > 0 && SensorValue[COLOUR_SENSOR] != (int)colorRed; layers -= 2)
  {

    unrollPaper(15, FWD_UNROLL_POWER); // 1.5layers ish
    wait1Msec(300);
    foldArm135CW(true);
    wait1Msec(300);

    //unrollPaper(5, FWD_UNROLL_POWER); //return to having fold which will start just past the SOMEDIST
    //wait1Msec(300);
    foldArm135CW(false);
    wait1Msec(300);

    layersUsed += 2;
    displayBigTextLine(0, "%d layers remain", layers);
    wait1Msec(300);
    //while(true) playSound(soundBlip);
  }
  eraseDisplay();
}

int getLayers() //TESTED & PASSED
{
  displayBigTextLine(0, "Clap for 4 layers.");
  displayBigTextLine(4, "MIN: 4, MAX: %d", MAX_LAYERS);
  while (SensorValue[SOUND_SENSOR] < SOUND_THRESHOLD)
  {}
  int layers = 4;

  do
  {
    displayBigTextLine(0, "%d layers.", layers);
    displayBigTextLine(2, "Clap for 2 more.");
    wait1Msec(500);
    time1[T1] = 0;
    while (SensorValue[SOUND_SENSOR] < SOUND_THRESHOLD && time1[T1] <= MAX_WAIT_TIME)
    {}

    if (time1[T1] > MAX_WAIT_TIME)
      return layers;
    layers += 2;


  } while (layers < MAX_LAYERS);
  eraseDisplay();
  return layers;
}

task main()
{
  SensorType[SOUND_SENSOR] = sensorSoundDBA;
  wait1Msec(50);
  SensorType[COLOUR_SENSOR] = sensorEV3_Color;
  wait1Msec(50);
  SensorMode[COLOUR_SENSOR] = modeEV3Color_Color;
  wait1Msec(50);

  while (paperExists())
  {
    fold(getLayers());
    displayBigTextLine(0, "%d layers used.", layersUsed);
    if (SensorValue[COLOUR_SENSOR] != (int)colorRed)
      outputPaper();
    else
    {
      motor[UNROLL_MOTOR] = FWD_UNROLL_POWER;
      wait1Msec(1000);
      motor[UNROLL_MOTOR] = 0;
    }

    motor[TRACK_MOTOR] = TRACK_POWER;
    wait1Msec(3000);
    motor[TRACK_MOTOR] = 0;
  }
}
