const tSensors SOUND_SENSOR = S1;
const tSensors COLOUR_SENSOR = S2;
const tMotor UNROLL_MOTOR = motorA;
const tMotor TRACK_MOTOR = motorD;
const tMotor FOLDER_MOTOR = motorB;
const tMotor PISTON_MOTOR = motorC;

const int SOUND_THRESHOLD = 150;
const int MAX_LAYERS = 7;
const int MAX_WAIT_TIME = 5000; //ms
const float ROLL_RADIUS = 5; //cm
const int TEAR_DISTANCE = -30; //cm
const int REV_UNROLL_POWER = 100;
const int FWD_UNROLL_POWER = 75;
const float FOLDARM_LIMIT = 135; //degrees
const int TRACK_POWER = 100;
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
  int position = 900; //modify
  if (!direction)
    position = -position;

  motor[PISTON_MOTOR] = 100;
  while (nMotorEncoder[PISTON_MOTOR] < position)
  {}

  motor[PISTON_MOTOR] = 0;
}

void foldArm135CW(bool rotateCW)
{
  int power = 20; //modify

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

void unrollPaper(int distanceCM, int power)
{
  if (power > 100)
    power = 100;
  else if (power < 0)
    power = 0;
  if (distanceCM < 0)
    power = -power;

  nMotorEncoder[UNROLL_MOTOR] = 0;
  int encoderStop = distanceCM * 360 / (2*PI*ROLL_RADIUS);
  motor[UNROLL_MOTOR] = power;
  while (nMotorEncoder[UNROLL_MOTOR] < encoderStop)
  {}

  motor[UNROLL_MOTOR] = 0;
}

void outputPaper()
{
  pistonUp(false);
  unrollPaper(TEAR_DISTANCE, REV_UNROLL_POWER);
  pistonUp(true);
}

bool failsafe(){return true;} //ANDREW
void fold(int layers) //ANDREW DO DEBUGGING HERE
{
  unrollPaper(7, FWD_UNROLL_POWER); //just past the fold arm
  for(; layers > 0 && SensorValue[COLOUR_SENSOR] != (int)colorRed; layers--)
  {
    foldArm135CW(false);
    unrollPaper(70, FWD_UNROLL_POWER); // 1.5layers ish

    foldArm135CW(true);
    unrollPaper(40, FWD_UNROLL_POWER); //return to having fold which will start just past the SOMEDIST

    layersUsed += 2;
    displayTextLine(0, "%d layers used on this roll.", layersUsed);
  }
}

int getLayers()
{
  displayTextLine(0, "Clap for layers");
  while (SensorValue[SOUND_SENSOR] < SOUND_THRESHOLD)
  {}
  int layers = 1;
  displayTextLine(0, "%d layers selected. Clap for more.", layers);

  time1[T1] = 0;
  do
  {
    while (SensorValue[SOUND_SENSOR] < SOUND_THRESHOLD  && time1[T1] <= MAX_WAIT_TIME)
    {}

    if (time1[T1] > MAX_WAIT_TIME)
      return layers;
    layers++;

    displayTextLine(0, "%d layers selected. Clap for more.", layers);
    time1[T1] = 0;
  } while (layers <= MAX_LAYERS);
  return layers;
}

task main()
{
  SensorType[SOUND_SENSOR] = sensorSoundDB;
  SensorType[COLOUR_SENSOR] = sensorEV3_Color;
  wait1Msec(50);
  SensorMode[COLOUR_SENSOR] = modeEV3Color_Color;
  wait1Msec(50);

  while (paperExists())
  {
    fold(getLayers());
    if (SensorValue[COLOUR_SENSOR] == (int)colorWhite)
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
