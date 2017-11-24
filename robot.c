const tSensors SOUND_SENSOR = S1;
const int SOUND_THRESHOLD = 50;
const int MAX_WAIT_TIME = 2500; //ms

const tMotor ROLLER_MOTOR = motorB;
const int DRIVEN_WHEEL_RADIUS = 2; //cm
const int UNROLL_POWER = 50;
const int TWO_LAYER_DIST = 15; //cm
const int TEAR_POWER = 35;
const int TEAR_DIST = 5; //cm

const tMotor TRACK_MOTOR = motorA;
const int TRACK_POWER = -75;
const int TRACK_TIME = 1250; //ms

const tMotor FOLDER_MOTOR = motorC;
const int FOLD_TIME = 300; //ms

const int MAX_LAYERS = 8;
const int MIN_LAYERS = 4;

const tMotor PISTON_MOTOR = motorD;

const tSensors COLOUR_SENSOR = S2;

const int TIME_BETWEEN_REQUESTS = 10000; //ms

int layersUsed = 0;


bool isPaper()
{
  return SensorValue[COLOUR_SENSOR] != (int)colorRed;
}

bool paperExists()
{
  eraseDisplay();
  while (!isPaper())
  {
    displayTextLine(0, "Press any button");
    displayTextLine(1, "to acknowledge the");
    displayTextLine(2, "roll is empty,");
    displayTextLine(3, "or place a new");
    displayTextLine(4, "roll in the");
    displayTextLine(5, "designated place.");
    playSound(soundDownwardTones);
    time1[T1] = 0;
    while (time1[T1] < MAX_WAIT_TIME)
    {
      if (getButtonPress(buttonAny))
        return false;
    }
  }
  return true;
}

void pistonUp(bool direction)
{
  int position = 7.5*360, motorPower = 100;
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

  nMotorEncoder[ROLLER_MOTOR] = 0;
  int encoderStop = distanceCM * 360 / (2*PI*DRIVEN_WHEEL_RADIUS);
  motor[ROLLER_MOTOR] = motorPower;

  if (unroll)
    while (nMotorEncoder[ROLLER_MOTOR] > -encoderStop)
    {}
  else
    while (nMotorEncoder[ROLLER_MOTOR] < encoderStop)
    {}

  motor[ROLLER_MOTOR] = 0;
}

void ripPaper()
{
  eraseDisplay();
  displayBigTextLine(0, "Ripping...");
  foldArmCW(true);
  pistonUp(false);
  unrollPaper(TEAR_DIST, TEAR_POWER, false);
  pistonUp(true);
  foldArmCW(false);
}

void fold(int layers)
{
  eraseDisplay();
  motor[TRACK_MOTOR] = TRACK_POWER/4;
  unrollPaper(4, UNROLL_POWER, true);
  motor[TRACK_MOTOR] = 0;
  wait1Msec(300);

  for(; layers > 0 && isPaper(); layers -= 2)
  {
    displayBigTextLine(0, "%d layers remain", layers);
    wait1Msec(300);

    unrollPaper(TWO_LAYER_DIST, UNROLL_POWER, true);
    wait1Msec(300);
    foldArmCW(true);
    wait1Msec(300);
    foldArmCW(false);
    wait1Msec(300);

    layersUsed += 2;
  }
}

int getLayers()
{
  eraseDisplay();
  displayBigTextLine(0, "Clap for %d layers", MIN_LAYERS);
  displayBigTextLine(4, "MIN: %d, MAX: %d", MIN_LAYERS, MAX_LAYERS);
  while (SensorValue[SOUND_SENSOR] < SOUND_THRESHOLD)
  {}
  int layers = MIN_LAYERS;

  while (layers < MAX_LAYERS)
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
  }
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
      eraseDisplay();
      displayBigTextLine(0, "Out of Paper!");
      unrollPaper(TWO_LAYER_DIST, UNROLL_POWER, true);
    }

    motor[TRACK_MOTOR] = TRACK_POWER;
    wait1Msec(TRACK_TIME);
    motor[TRACK_MOTOR] = 0;

    displayBigTextLine(0, "Grab Paper.");
    wait1Msec(2000);
    displayBigTextLine(0, "%d layers used.", layersUsed);
    wait1Msec(TIME_BETWEEN_REQUESTS - 2000);
  }
}