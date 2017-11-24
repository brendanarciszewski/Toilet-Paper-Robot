const tSensors SOUND_SENSOR = S1;
const int SOUND_THRESHOLD = 50;
const int MAX_WAIT_TIME = 2500; //ms

const tMotor UNROLL_MOTOR = motorB;
const int REV_UNROLL_POWER = 100;
const int FWD_UNROLL_POWER = 50;
const int DRIVEN_WHEEL_RADIUS = 2; //cm
const int TEAR_DISTANCE = 15; //cm //NEEDS TEST

const tMotor TRACK_MOTOR = motorA;
const int TRACK_POWER = -100;
const int TRACK_TIME = 1500; //ms

const tMotor FOLDER_MOTOR = motorC;
const int FOLD_TIME = 400; //ms

const int MAX_LAYERS = 8; //PASS
const int MIN_LAYERS = 4; //PASS

const tMotor PISTON_MOTOR = motorD;

const tSensors COLOUR_SENSOR = S2;

int layersUsed = 0;


bool isPaper()
{
  return SensorValue[COLOUR_SENSOR] != (int)colorRed;
}

bool paperExists()
{
  while (!isPaper())
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
  int position = 10.5*360, motorPower = 100;
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

void foldArmCW(bool rotateCW)
{
  int motorPower = 50;

  if (!rotateCW)
    motorPower = -motorPower;
  motor[FOLDER_MOTOR] = motorPower;

  time1[T1] = 0;
  while (time1[T1] < FOLD_TIME)
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

void ripPaper()
{
  displayBigTextLine(0, "Ripping...");
  foldArmCW(true);
  pistonUp(false);
  unrollPaper(TEAR_DISTANCE, REV_UNROLL_POWER, false);
  pistonUp(true);
  foldArmCW(false);
}

//bool failsafe(){return true;} //ANDREW


void fold(int layers)
{
  unrollPaper(5, FWD_UNROLL_POWER, true); //just past the fold arm
  wait1Msec(300);

  for(; layers > 0 && isPaper(); layers -= 2)
  {
    displayBigTextLine(0, "%d layers remain", layers);

    unrollPaper(15, FWD_UNROLL_POWER, true); //2 layers
    wait1Msec(300);
    foldArmCW(true);
    wait1Msec(300);
    foldArmCW(false);
    wait1Msec(300);

    layersUsed += 2;
  }
  eraseDisplay();
}

int getLayers() //TESTED & PASSED
{
  displayBigTextLine(0, "Clap for %d layers", MIN_LAYERS);
  displayBigTextLine(4, "MIN: %d, MAX: %d", MIN_LAYERS, MAX_LAYERS);
  while (SensorValue[SOUND_SENSOR] < SOUND_THRESHOLD)
  {}
  int layers = MIN_LAYERS;

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
  SensorType[COLOUR_SENSOR] = sensorEV3_Color;
  wait1Msec(50);
  SensorMode[COLOUR_SENSOR] = modeEV3Color_Color;
  wait1Msec(50);

  while (paperExists())
  {
    fold(getLayers());
    if (SensorValue[COLOUR_SENSOR] != (int)colorRed)
      ripPaper();
    else
    {
      displayBigTextLine(0, "Out of Paper!");
      motor[UNROLL_MOTOR] = FWD_UNROLL_POWER;
      wait1Msec(1000);
      motor[UNROLL_MOTOR] = 0;
    }

    motor[TRACK_MOTOR] = TRACK_POWER;
    wait1Msec(TRACK_TIME);
    motor[TRACK_MOTOR] = 0;

    displayBigTextLine(0, "Grab Paper.");
    wait1Msec(2000);
    displayBigTextLine(0, "%d layers used.", layersUsed);
    wait1Msec(8000);
  }
}
