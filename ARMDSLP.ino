/*
  ARMDSLP.ino- Arduino Range Measurer and Digital Spirit Level Project code

  Created by      Ferrer, Edrian Jose D.G.
  Co-created by : Maranan, Remzell
                  Filipino, Angelo Kyle
                  Nuarin, George Anthony
                  Sarmiento, Jonil Mark

  Last modified version: June 14, 2021
*/


#include <LiquidCrystal_I2C.h>
#include <ARMDSL.h>

/*
 * Libaries sources
 * LiquidCrystal_I2C  - https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
 * ARMDSL             - https://github.com/Edrian-Jose/ARMDSL
 */

#define trigPin 8
#define echoPin 9
#define buttonPin 10
#define buzzerPin 11
#define shortPressDuration 500
#define longPressDuration 1500

//  Library-defined objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
ARMDSL device;
Vector r;


bool xAxis;
int state, nextState, measurement, unit;
long duration;

float distance, area, angle, angleConstraint;
float savedMeasurements[2];

String measurements[3] = {"Distance", "Area", "Spirit Level"};
String units[4] = {"cm", "m", "in", "ft"};

void DecideOnPress(long duration, void (*callback1)(), long trigger = shortPressDuration);



void setup()
{


  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);


  device.setupAccelerometer();
  lcd.init();
  lcd.backlight();


  ResetToDefaults();
}


void loop()
{
  /*
    Each case of the switch implements different state of the Device
    Each case has a nextState assignment for the program to know where to go
    when the user prompts to continue
  */

  switch (state)
  {
  case 0:
    /* 
    Pick type of measurement tool
    Choices are Distance(a), Area(b), Spirit Level(c)
    */
    PickMeasurement();
    break;


  case 1:
    /* 
    a1
    Pick unit of measurement for Distance
    Choices are cm, m, in, and ft.
    */
    nextState = 2;
    PickUnit();
    break;


  case 2:
    /* 
    a2
    Pick unit of measurement for Distance
    Choices are cm, m, in, and ft.
    */
    nextState = 0;
    MeasureDistance();
    break;


  case 3:
    /* 
    b1
    Pick unit of measurement for Distance to be used in Area Calculation
    Choices are cm, m, in, and ft.
    */
    nextState = 4;
    PickUnit();
    break;


  case 4:
    /* 
    b2
    Measures the distance of the first axis
    and store it savedMeasurements
    */
    nextState = 5;
    lcd.print("(d1) Distance:");
    savedMeasurements[0] = StoreDistance();
    //Area
    break;


  case 5:
    /* 
    b3
    Measures the distance of the second axis
    and store it savedMeasurements
    */
    nextState = 6;
    lcd.print("(d2) Distance:");
    savedMeasurements[1] = StoreDistance();
    //Area
    break;


  case 6:
    /* 
    b4
    Calculates the area based on b3 and b4 procedure measurements.
    */
    nextState = 0;
    MeasureArea();
    break;


  case 7:
    /* 
    c1
    Pick the axis on where to measure the level
    Choices are x (roll), y (pitch)
    */
    nextState = 8;
    PickAxis();
    //Angle
    break;


  case 8:
    /* 
    c2
    Pick the angle constraint 
    Choices are 15, 30, 45, 60, 75, and 90
    */
    nextState = 9;
    PickConstraint();
    //Angle
    break;


  case 9:
    /* 
    c3
    Calculates the angle and turned on the buzzer 
    when the angle exceeds the angle constraint
    */
    nextState = 0;
    MeasureAngle();
    //Angle
    break;


  default:
    /* 
    Reset to default values when state is undefined
    */
    ResetToDefaults();
  }

  /* 
    Clears the LCD screen every user input
    to prepare the next LCD display output
  */
  lcd.clear();
  lcd.setCursor(0, 0);
}

void ResetToDefaults(){
  state = 0;
  nextState = 1;
  measurement = 0;
  unit = 0;
  duration = 0;
  xAxis = true;
  distance = 0;
  area = 0;
  angle = 0;
  angleConstraint = 90;
  savedMeasurements[0]= 0;
  savedMeasurements[1]= 0;
  lcd.clear();
  lcd.setCursor(0, 0);
}

void NextState()
{
  state = nextState;
  if (state == 0)
  {
    nextState = 1;
  }
}


void BeepBeep() {
    tone(buzzerPin, 240);
    delay(100);
    noTone(buzzerPin);
    delay(200);
    tone(buzzerPin, 240);
    delay(100);
    noTone(buzzerPin);
}


void DecideOnPress(long duration, void (*callback1)(), long trigger)
{
  if (duration > trigger)
  {
    NextState();
  }
  else
  {
    tone(buzzerPin, 240);
    delay(100);
    noTone(buzzerPin);
    callback1();
  }
}


// State 0
void PickMeasurement()
{

  lcd.print("Pick Measurement");
  lcd.setCursor(0, 1);
  lcd.print(measurements[measurement]);

  /*
    PLay a beep beep sounds when pressed more than the shortPressDuration
    It was to notify the user to hold out the push button

    buttonPin - the Pin number of push button
    &BeepBeep - memory address of the BeepBeep callback function
    shortPressDuration - trigger duration to play the beep sounds

    returns the duration on which the user pushes the button, 
    can be longer than the trigger duration
  */

  duration = device.waitButtonState(buttonPin,&BeepBeep,shortPressDuration);

  /*
    Calls the ChangeMeasurement callback function when user push the button
    for less than 500ms while continue to next state if longer than 500ms.
  */
  DecideOnPress(duration, &ChangeMeasurement);
}

void ChangeMeasurement()
{
  switch (measurement)
  {
  case 1:
    /*
    From Area measurement
    Sets the measurement to Spirit Level
    and set the Next state to 7 (c1, Picking the Axis)
    */
    measurement = 2;
    nextState = 7;
    break;
  case 2:
    /*
    From Spirit level measurement
    Sets the measurement to Distance
    and set the Next state to 1 (a1, Picking the Unit)
    */
    measurement = 0;
    nextState = 1;
    break;
  default:
  /*
    From Distance measurement
    Sets the measurement to Area
    and set the Next state to 3 (b1, Picking the Unit)
    */
    measurement = 1;
    nextState = 3;
    break;
  }
}

// State 1
void PickUnit()
{
  lcd.print("Measurement Unit:");
  lcd.setCursor(0, 1);
  lcd.print(units[unit]);
  duration = device.waitButtonState(buttonPin,&BeepBeep,shortPressDuration);
  DecideOnPress(duration, &ChangeUnit);
}

void ChangeUnit()
{
  /*
    Toggle unit values between 0,1,2,3,4.
    0 -> 4, and back to 0 again
  */
  unit = (unit < 3) ? unit + 1 : 0;
}

float ConvertDistance(float _distance)
{
  /*
    Default distance reading is cm.
  */
  switch (unit)
  {
  case 1:
    /* m*/
    _distance *= 0.01;
    break;
  case 2:
    /* in*/
    _distance *= 0.393701;
    break;
  case 3:
    /* ft */
    _distance *= 0.0328;
    break;
  default:
    // cm
    _distance = _distance;
    break;
  }
  return _distance;
}

//State 2
void MeasureDistance()
{
  lcd.setCursor(0, 1);
  lcd.print("s:");
  lcd.setCursor(3, 1);
  lcd.print(ConvertDistance(savedMeasurements[0]));
  lcd.setCursor(0, 0);
  lcd.print("d: ");

  /*
    Displays the realtime value of distance while button is not pushed.
  */
  do
  {
    lcd.setCursor(3, 0);
    lcd.print("          ");
    lcd.setCursor(3, 0);
    distance = device.measureDistance(trigPin, echoPin);
    lcd.print(ConvertDistance(distance));
    lcd.setCursor(14, 0);
    lcd.print(units[unit]);
    delay(250);
  } while (!device.readButtonState(buttonPin));

  duration = device.waitButtonState(buttonPin,&BeepBeep,longPressDuration);
  savedMeasurements[0] = distance;
  DecideOnPress(duration, &NullFunc, longPressDuration);
}

float StoreDistance()
{
  lcd.setCursor(1, 1);
  float convertedDistance = distance;
  do
  {
    lcd.setCursor(1, 1);
    lcd.print("                ");
    lcd.setCursor(1, 1);
    distance = device.measureDistance(trigPin, echoPin);
    convertedDistance = ConvertDistance(distance);
    lcd.print(convertedDistance);
    lcd.setCursor(14, 1);
    lcd.print(units[unit]);
    delay(250);
  } while (!device.readButtonState(buttonPin));

  duration = device.waitButtonState(buttonPin,&BeepBeep,shortPressDuration);
  DecideOnPress(duration, &NullFunc);
  return convertedDistance;
}

void MeasureArea()
{
  /*
    savedMeasurements are obtained in StoreDistance function
  */
  area = savedMeasurements[0] * savedMeasurements[1];


  lcd.print("Area:");
  lcd.setCursor(6, 0);
  lcd.print(area);
  lcd.setCursor(12, 0);
  lcd.print(units[unit]);
  lcd.print("^2");
  lcd.setCursor(0, 1);
  lcd.print(savedMeasurements[0]);
  lcd.print(" X ");
  lcd.print(savedMeasurements[1]);

  duration = device.waitButtonState(buttonPin,&BeepBeep,longPressDuration);
  DecideOnPress(duration, &NullFunc, longPressDuration);
}

void PickAxis()
{
  lcd.print("Select Axis: ");

  do
  {
    xAxis = !xAxis;
    lcd.setCursor(13, 0);
    lcd.print(xAxis ? "x" : "y");
    duration = device.waitButtonState(buttonPin,&BeepBeep,shortPressDuration);
  } while (duration < shortPressDuration);

  NextState();
}

void PickConstraint(){

  lcd.print("Angle Constraint:");
  lcd.setCursor(0, 1);
  lcd.print(angleConstraint);
  lcd.print(" deg");
  duration = device.waitButtonState(buttonPin,&BeepBeep,shortPressDuration);
  DecideOnPress(duration, &ChangeConstraint);

}

void ChangeConstraint(){
  angleConstraint = (angleConstraint < 90) ? angleConstraint + 15 : 15;
}

void MeasureAngle()
{

  lcd.print(xAxis ? "Roll" : "Pitch");
  lcd.setCursor(0, 1);
  lcd.print("Angle: ");


  do
  {
    r = device.measureAccel();
    float a1 = xAxis ? r.y : r.x;
    float a2 = xAxis ? r.x : r.y;
    angle = atan(-1 * a1 / sqrt(pow(a2, 2) + pow(r.z, 2))) * 180 / PI;
    
    lcd.setCursor(7, 1);
    lcd.print("          ");
    lcd.setCursor(7, 1);
    lcd.print(angle);
    lcd.print(" deg");

    /*
      Turn on the buzzer when constrained angle is reached
    */
    if (angle >= angleConstraint)
    {
      tone(buzzerPin, 400);
    }else{
      noTone(buzzerPin);
    }

    delay(250);
  } while (!device.readButtonState(buttonPin));


  duration = device.waitButtonState(buttonPin,&BeepBeep,longPressDuration);
  DecideOnPress(duration, &NullFunc, longPressDuration);
}


void NullFunc() {}
