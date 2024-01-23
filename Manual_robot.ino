#include <Servo.h>
#include <PS2X_lib.h>

Servo servo_left;
Servo servo_right;
PS2X ps2x;

#define PS2_DAT        13 //brown         grey-vibration 5V
#define PS2_CMD        12 //blue          black-ground
#define PS2_SEL        11 //orange        red-vcc 3.3V
#define PS2_CLK        10 //yellow        green- acknowledge

#define pressures   false
#define rumble      false

const byte MOTOR1_IN1 = 53;//blue
const byte MOTOR1_IN2 = 52;//yellow
const byte MOTOR1_PWM = 4;//white

const byte MOTOR2_IN1 = 23;
const byte MOTOR2_IN2 = 22;
const byte MOTOR2_PWM = 7;

const byte MOTOR3_IN1 = 51;
const byte MOTOR3_IN2 = 50;
const byte MOTOR3_PWM = 5;

const byte MOTOR4_IN1 = 25;
const byte MOTOR4_IN2 = 24;
const byte MOTOR4_PWM = 6;

const byte ARM_IN1 = 27; //for arm motor
const byte ARM_IN2 = 26;
const byte ARM_PWM = 8;

const byte ENCODER_A = 18;   //red
const byte ENCODER_B = 19;   //yellow

const byte SERVO_LEFT = 2; //looking from face
const byte SERVO_RIGHT = 3;

const byte SERVO_RIGHT_CLOSE = 40; // reduce to close
const byte SERVO_RIGHT_OPEN = 80; // increase to open
const byte SERVO_LEFT_CLOSE = 130; // increase to close
const byte SERVO_LEFT_OPEN = 90; // reduce to open

const int CONT_UPPER_HIGH = 255;
const int CONT_UPPER_LOW = 165;
const int CONT_LOWER_HIGH = 90;
const int CONT_LOWER_LOW = 0;

const int ARM_SHIFT = 500;
const byte ARM_ERROR = 3;
volatile long arm_pulse = 10000;  //distance measured by the encoder
int cont = 1; // encoder variable
int enc; 
int arm_speed = 80;

int speed_ = 80;
int temp = 0;

void setup()
{
    Serial.begin(57600);
    ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    
    pinMode(MOTOR1_IN1, OUTPUT);
    pinMode(MOTOR1_IN2, OUTPUT);
    pinMode(MOTOR1_PWM, OUTPUT);
    
    pinMode(MOTOR2_IN1, OUTPUT);
    pinMode(MOTOR2_IN2, OUTPUT);
    pinMode(MOTOR2_PWM, OUTPUT);
    
    pinMode(MOTOR3_IN1, OUTPUT);
    pinMode(MOTOR3_IN2, OUTPUT);
    pinMode(MOTOR3_PWM, OUTPUT);
    
    pinMode(MOTOR4_IN1, OUTPUT);
    pinMode(MOTOR4_IN2, OUTPUT);
    pinMode(MOTOR4_PWM, OUTPUT);
   
    pinMode(ARM_IN1, OUTPUT);
    pinMode(ARM_IN2, OUTPUT);
    pinMode(ARM_PWM, OUTPUT);

    servo_left.attach(SERVO_LEFT);
    servo_right.attach(SERVO_RIGHT);

    attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderAChange, FALLING);
    interrupts();


}

void loop()
{
    ps2x.read_gamepad(false, 0);
    //while(!Serial.available());
    char x = 'f';
    //Serial.println(arm_pulse);
    if(ps2x.Button(PSB_PAD_RIGHT)) // close claws
    {
      closeServoRight();
      closeServoLeft();
      Serial.println("Claws closed");
    }
    if(ps2x.Button(PSB_PAD_LEFT)) //open claws
    {
      openServoRight();
      openServoLeft();
      Serial.println("Claws opened");
    }
    if(ps2x.Button(PSB_R1)) // CW rotation of Arm
    {
       // Serial.println(arm_pulse);
        
        if(cont == 1)
        { 
            enc = arm_pulse;
            cont = 2;
        }
        if(cont == 2)
        {

            rotateArmCw(arm_speed);
            if((enc + ARM_SHIFT) < arm_pulse)
            {
                armBrake();
                cont = 3;
            }
        }
        if(cont == 3)
        {
            if((enc + ARM_SHIFT) > (arm_pulse + ARM_ERROR))
            {
                rotateArmCw(arm_speed);
            }
            if((enc + ARM_SHIFT) < (arm_pulse - ARM_ERROR))
            {
                rotateArmAcw(arm_speed);
            }
            else
            {
                armBrake();
            }
        }
    }
    if(ps2x.Button(PSB_L1)) // ACW rotation of Arm
    {
             // Serial.println(arm_pulse);
             
        
        if(cont == 1)
        { 
            enc = arm_pulse;
            cont = 2;
        }
        if(cont == 2)
        {

            rotateArmAcw(arm_speed);
            if((enc - ARM_SHIFT) < arm_pulse)
            {
                armBrake();
                cont = 3;
                Serial.println("here4");
            }
        }
        if(cont == 3)
        {
            if((enc - ARM_SHIFT) > (arm_pulse + ARM_ERROR))
            {
                rotateArmCw(arm_speed);
                Serial.println("here5");
            }
            if((enc - ARM_SHIFT) < (arm_pulse - ARM_ERROR))
            {
                rotateArmAcw(arm_speed);
                Serial.println("here6");
            }
            else
            {
                armBrake();
                Serial.println("here7");
            }
        }
    }
    if(ps2x.Button(PSB_R2)) // rotate bot CW
    {
        rotateCw(speed_);
        Serial.print("RCW");
        Serial.println(speed_);
    }
     if(ps2x.Button(PSB_L2)) // rotate bot ACW
    {
        rotateAcw(speed_);
        Serial.print("RACW");
        Serial.println(speed_);
    }
    if((temp = ps2x.Analog(PSS_LY)) < CONT_LOWER_HIGH) // move forward
    {
        temp = CONT_LOWER_HIGH - temp;
        speed_ = mapSpeed(temp, 0, CONT_LOWER_HIGH, 0, 255);
        goForward(speed_);
        Serial.print("FWD");
        Serial.println(speed_);
    }
    if((speed_ = ps2x.Analog(PSS_LY)) > CONT_UPPER_LOW) // move backward
    {
        
        speed_ = mapSpeed(speed_, CONT_UPPER_LOW, 255, 0, 255);
        goBackward(speed_);
        Serial.print("BKW");
        Serial.println(speed_);
    }
    if((speed_=ps2x.Analog(PSS_LX)) > CONT_UPPER_LOW) // move right
    {
        speed_ = mapSpeed(speed_, CONT_UPPER_LOW, 255, 0, 255);
        goRight(speed_);
        Serial.print("RT");
        Serial.println(speed_);
    }
    if((speed_ = ps2x.Analog(PSS_LX)) < CONT_LOWER_HIGH) // move left
    {
        speed_ = CONT_LOWER_HIGH - speed_;
        speed_ = mapSpeed(speed_, 0, CONT_LOWER_HIGH, 0, 255);
        goLeft(speed_);
        Serial.print("LT");
        Serial.println(speed_);
    }
  

}


void encoderAChange() //adds and subtracts depending on the direction
{
    if(digitalRead(ENCODER_A) == 0)
    {
        if(digitalRead(ENCODER_B) == 0) { arm_pulse = arm_pulse - 1; }
        else { arm_pulse = arm_pulse + 1; }
    }
    else
    {
        if(digitalRead(ENCODER_B) == 0) { arm_pulse = arm_pulse + 1; }
        else { arm_pulse = arm_pulse - 1; }
    }
}

void openServoLeft()
{
    servo_left.write(SERVO_LEFT_OPEN);
}

void openServoRight()
{
    servo_right.write(SERVO_RIGHT_OPEN);   
}

void closeServoLeft()
{
    servo_left.write(SERVO_LEFT_CLOSE);
}

void closeServoRight()
{
    servo_right.write(SERVO_RIGHT_CLOSE);
}

void armBrake()
{
    digitalWrite(ARM_IN1, LOW);
    digitalWrite(ARM_IN2, LOW);
    analogWrite(ARM_PWM, 0);
}

void rotateArmCw(byte motor_speed)
{
    digitalWrite(ARM_IN1, HIGH);
    digitalWrite(ARM_IN2, LOW);
    analogWrite(ARM_PWM, motor_speed);
}

void rotateArmAcw(byte motor_speed)
{
    digitalWrite(ARM_IN1, LOW);
    digitalWrite(ARM_IN2, HIGH);
    analogWrite(ARM_PWM, motor_speed);
}

void changeMotorSpeed(byte motor_speed)
{
    analogWrite(MOTOR1_PWM, motor_speed);
    analogWrite(MOTOR2_PWM, motor_speed);
    analogWrite(MOTOR3_PWM, motor_speed);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void rotateAcw(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, HIGH);
    digitalWrite(MOTOR1_IN2, LOW);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, HIGH);
    digitalWrite(MOTOR2_IN2, LOW);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, HIGH);
    digitalWrite(MOTOR3_IN2, LOW);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, HIGH);
    digitalWrite(MOTOR4_IN2, LOW);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void rotateCw(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, LOW);
    digitalWrite(MOTOR1_IN2, HIGH);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, LOW);
    digitalWrite(MOTOR2_IN2, HIGH);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, LOW);
    digitalWrite(MOTOR3_IN2, HIGH);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, LOW);
    digitalWrite(MOTOR4_IN2, HIGH);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void goRight(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, HIGH);
    digitalWrite(MOTOR1_IN2, LOW);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, LOW);
    digitalWrite(MOTOR2_IN2, HIGH);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, HIGH);
    digitalWrite(MOTOR3_IN2, LOW);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, LOW);
    digitalWrite(MOTOR4_IN2, HIGH);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void goLeft(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, LOW);
    digitalWrite(MOTOR1_IN2, HIGH);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, HIGH);
    digitalWrite(MOTOR2_IN2, LOW);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, LOW);
    digitalWrite(MOTOR3_IN2, HIGH);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, HIGH);
    digitalWrite(MOTOR4_IN2, LOW);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void goBackward(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, HIGH);
    digitalWrite(MOTOR1_IN2, LOW);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, LOW);
    digitalWrite(MOTOR2_IN2, HIGH);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, LOW);
    digitalWrite(MOTOR3_IN2, HIGH);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, HIGH);
    digitalWrite(MOTOR4_IN2, LOW);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void goForward(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, LOW);
    digitalWrite(MOTOR1_IN2, HIGH);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, HIGH);
    digitalWrite(MOTOR2_IN2, LOW);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, HIGH);
    digitalWrite(MOTOR3_IN2, LOW);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, LOW);
    digitalWrite(MOTOR4_IN2, HIGH);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void motorBrake()
{
    digitalWrite(MOTOR1_IN1, LOW);
    digitalWrite(MOTOR1_IN2, LOW); 
    digitalWrite(MOTOR1_PWM, 0);
    
    digitalWrite(MOTOR2_IN1, LOW);
    digitalWrite(MOTOR2_IN2, LOW);
    digitalWrite(MOTOR2_PWM, 0);
    
    digitalWrite(MOTOR3_IN1, LOW);
    digitalWrite(MOTOR3_IN2, LOW);
    digitalWrite(MOTOR3_PWM, 0);
    
    digitalWrite(MOTOR4_IN1, LOW);
    digitalWrite(MOTOR4_IN2, LOW);
    digitalWrite(MOTOR4_PWM, 0);
}

int mapSpeed(int var_x, int in_min, int in_max, int out_min, int out_max)
{
    float m = (float) (out_max - out_min) / (in_max - in_min);
    int y =  m * (var_x - in_min) + out_min;
    return y;
}
