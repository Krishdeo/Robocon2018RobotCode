#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //uses 0x27 register

//pin assignments
const byte MOTOR1_IN1 = 49;
const byte MOTOR1_IN2 = 48;
const byte MOTOR1_PWM = 4;

const byte MOTOR2_IN1 = 23;
const byte MOTOR2_IN2 = 22;
const byte MOTOR2_PWM = 5;

const byte MOTOR3_IN1 = 25;
const byte MOTOR3_IN2 = 24;
const byte MOTOR3_PWM = 6;

const byte MOTOR4_IN1 = 47;
const byte MOTOR4_IN2 = 46;
const byte MOTOR4_PWM = 7;

const byte MTR_ENC_A = 2;   //white
const byte MTR_ENC_B = 3;   //brown

const byte ACT_IN1 = 35;
const byte ACT_IN2 = 34;
const byte ACT_PWM = 8;
const byte ACT_BACK_EMF = 50;
const byte ACT_POT_OUT = 37;  //yellow - VCC
const byte ACT_POT_IN = A15;  //blue -   SIG
const int CELL_IN = A14;

const byte RELAY_IN1 = 26;
const byte RELAY_IN2 = 27;

const byte SWITCH1 = 28;
const byte SWITCH2 = 29;
const byte SWITCH3 = 30;
const byte SWITCH4 = 31;
const byte SWITCH5 = 32;
const byte SWITCH6 = 33;
const byte SWITCH7 = 44;
const byte SWITCH8 = 51;

const byte STICK_X = A9;
const byte STICK_Y = A10;
const int CONT_UPPER_HIGH = 1023;
const int CONT_UPPER_LOW = 550;
const int CONT_LOWER_HIGH = 473;
const int CONT_LOWER_LOW = 0;

const byte C1 = 36; // |C1|C2|C3|C4|R1|R2|R3|R4|
const byte C2 = 39;
const byte C3 = 38;
const byte C4 = 41;
const byte R1 = 40;
const byte R2 = 43;
const byte R3 = 42;
const byte R4 = 45;

//constant variables
const int KEYBOARD_ROWS = 4;
const int KEYBOARD_COLUMNS = 4;
byte ROW_PINS[KEYBOARD_ROWS] = {R1, R2, R3, R4};
byte COLUMN_PINS[KEYBOARD_COLUMNS] = {C1, C2, C3, C4};
char key_map[KEYBOARD_ROWS][KEYBOARD_COLUMNS] = {{'1','2','3','A'}, {'6','5','4','B'}, {'7','8','9','C'}, {'*','0','#','D'}};

Keypad key_pad = Keypad(makeKeymap(key_map), ROW_PINS, COLUMN_PINS, KEYBOARD_ROWS, KEYBOARD_COLUMNS);

const int SHOOT_DELAY = 600;
const int RESET_DELAY = 150;

const int ACT_POT_MIN = 14;   //potentiometer reading
const int ACT_POT_MAX = 940;  //potentiometer reading
const int ACT_LEN_MIN = 0;    //in mm
const int ACT_LEN_MAX = 304;  //12" in mm - max extension of actuator
const int ACT_ERROR = 2;      //+-
const int ACT_SET_SPEED = 255;

const int TZ1_ACT_LEN = 155; //mm //145
const int TZ2_ACT_LEN = 136;
const int TZ3_ACT_LEN = 55; //50
const float CELL_GRAD = 8.4 / 1023; //max reading at 8.4V

const int SPEED_255 = 255;
const int SPEED_200 = 200;
const int SPEED_150 = 150;
const int SPEED_100 = 100;
const int SPEED_50 = 50;
const int SPEED_25 = 25;
const int SPEED_0 = 0;

const long STOPPING_DIST1 = 50;

int speed_ = SPEED_255;          //bot sees this speed when moving
String dirction = "F1";          //F1,L1,D1,R1,L2,R2,R1,R3,F2,L1,D1,R1,L2,R2,R1,L3,L4
int switch_cont = 1;             //activating/deactivating parts of code on certain conditions
volatile long pulse = 0;         //reading from motor encoder
long PULSE_PER_CM = 143;         //value from encoder per centimeter travelled
int pick_pos = 1;                //position where shuttlecock is passed over the bots
long distance_tz1_recv = 40;
int ini = 0;
int fin = 0;
int key_cont = 1;
char key1;
char key2;
bool throw_ = false;

void setup()
{
    Serial.begin(9600);
    lcd.begin(16,2);
    
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

    pinMode(MTR_ENC_A, INPUT);
    pinMode(MTR_ENC_B, INPUT);

    pinMode(ACT_IN1, OUTPUT);
    pinMode(ACT_IN2, OUTPUT);
    pinMode(ACT_PWM, OUTPUT);
    pinMode(ACT_BACK_EMF, OUTPUT);
    pinMode(ACT_POT_OUT, OUTPUT);
    pinMode(ACT_POT_IN, INPUT);
    pinMode(CELL_IN, INPUT);
    digitalWrite(ACT_POT_OUT, HIGH);
    
    pinMode(RELAY_IN1, OUTPUT);
    pinMode(RELAY_IN2, OUTPUT);
    
    pinMode(SWITCH1, INPUT_PULLUP);
    pinMode(SWITCH2, INPUT_PULLUP);
    pinMode(SWITCH3, INPUT_PULLUP);
    pinMode(SWITCH4, INPUT_PULLUP);
    pinMode(SWITCH5, INPUT_PULLUP);
    pinMode(SWITCH6, INPUT_PULLUP);
    pinMode(SWITCH7, INPUT_PULLUP);
    pinMode(SWITCH8, INPUT_PULLUP);
    
    pinMode(STICK_X, INPUT);
    pinMode(STICK_Y, INPUT);

    attachInterrupt(digitalPinToInterrupt(MTR_ENC_A), getMotorEncoder, FALLING);
    interrupts();
    delay(500);
    
    char key;
    bool set = false;
    int start_index = 0;
    int end_index = 1;
    char menu = '0';
    char last_key = 'A';
    int select = 0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Press * to");
    lcd.setCursor(0,1);
    lcd.print("check modules...");

    int time1 = 0;
    while(time1 < 1000)   //wait 1 seconds to press key
    {
        time1 = millis();
        key = key_pad.getKey();
        if(key)
        {
            set = true;
        }
    }
    lcd.clear();
    while(set == true)
    {
        int num_options = 7;
        String options[num_options];
        options[0] = "Switches";
        options[1] = "Motor encoder";
        options[2] = "Actuator";
        options[3] = "Pneumatic";
        options[4] = "Motors";
        options[5] = "Exit";
        options[6] = "";

        char up = 'A';
        char down = 'B';
        while(menu == '0')
        {
            key = key_pad.getKey();
            if(!key)
            {
                continue;
            }
            if((key == up) && (last_key == up))
            {
                if(start_index < 0)
                {
                    start_index = 0;
                    end_index = 1;
                }
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("->");
                lcd.setCursor(2,0);
                lcd.print(options[start_index]);
                lcd.setCursor(2,1);
                lcd.print(options[end_index]);
                select = start_index;
                start_index = start_index - 1;
                end_index = end_index - 1;
            }
            if((key == up) && (last_key == down))
            {
                start_index = start_index - 2;
                end_index = end_index - 2;
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("->");
                lcd.setCursor(2,0);
                lcd.print(options[start_index]);
                lcd.setCursor(2,1);
                lcd.print(options[end_index]);
                select = start_index;
                start_index = start_index - 1;
                end_index = end_index - 1;
            }
            if((key == down) && (last_key == up))
            {
                start_index = start_index + 2;
                end_index = end_index + 2;
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("->");
                lcd.setCursor(2,0);
                lcd.print(options[start_index]);
                lcd.setCursor(2,1);
                lcd.print(options[end_index]);
                select = start_index;
                start_index = start_index + 1;
                end_index = end_index + 1;
            }
            if((key == down) && (last_key == down))
            {
                if(end_index > (num_options - 1))
                {
                    start_index = num_options - 2;
                    end_index = num_options - 1;
                }
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("->");
                lcd.setCursor(2,0);
                lcd.print(options[start_index]);
                lcd.setCursor(2,1);
                lcd.print(options[end_index]);
                select = start_index;
                start_index = start_index + 1;
                end_index = end_index + 1;
            }
            if(key == 'C')
            {
                menu = select + 48 + 1;
            }
            last_key = key;
        }
        if(menu == '1')
        {
            lcd.clear();
            key = 'B';
            while(key != 'A')
            {
                key = key_pad.getKey();
                lcd.setCursor(0,0);
                lcd.print("S 1 2 3 4 5 6 7");
                lcd.setCursor(2,1);
                lcd.print(!digitalRead(SWITCH1));
                lcd.setCursor(4,1);
                lcd.print(!digitalRead(SWITCH2));
                lcd.setCursor(6,1);
                lcd.print(!digitalRead(SWITCH3));
                lcd.setCursor(8,1);
                lcd.print(!digitalRead(SWITCH4));
                lcd.setCursor(10,1);
                lcd.print(!digitalRead(SWITCH5));
                lcd.setCursor(12,1);
                lcd.print(!digitalRead(SWITCH6));
                lcd.setCursor(14,1);
                lcd.print(!digitalRead(SWITCH7));
            }
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Press A");
            lcd.setCursor(0,1);
            lcd.print("to continue...");
            menu = '0';
            last_key = up;
        }
        if(menu == '2')
        {
            lcd.clear();
            key = 'B';
            goForward(255);
            int tim = 100;
            delay(tim);
            while(key != 'A')
            {
                key = key_pad.getKey();
                lcd.setCursor(0,0);
                lcd.print("Encoder speed:");
                ini = pulse;
                delay(tim);
                fin = pulse;
                float s1 = (float)(fin - ini) / tim;
                int s2 = (int)(s1 * (float)tim);
                lcd.setCursor(0,1);
                lcd.print(s2);
            }
            changeMotorSpeed(0);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Press A");
            lcd.setCursor(0,1);
            lcd.print("to continue...");
            menu = '0';
            last_key = up;
        }
        if(menu == '3')
        {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Actuator up");
            lcd.setCursor(0,1);
            lcd.print("Pos - ");
            int pos = 0;
            while(pos < (ACT_LEN_MAX / 2 / 10))
            {
                pos = getActuator();
                pos = pos / 10;
                actuatorUp(100);
                lcd.setCursor(6,1);
                lcd.print(pos);
                if(pos < 10)
                {
                    lcd.setCursor(7,1);
                    lcd.print("cm");
                    lcd.setCursor(9,1);
                    lcd.print(" ");
                }
                else
                {
                    lcd.setCursor(8,1);
                    lcd.print("cm");
                }
            }
            while(pos < ((ACT_LEN_MAX - 4) / 10))
            {
                pos = getActuator();
                pos = pos / 10;
                actuatorUp(255);
                lcd.setCursor(6,1);
                lcd.print(pos);
                lcd.setCursor(8,1);
                lcd.print("cm");
            }
            delay(1000);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Actuator down");
            lcd.setCursor(0,1);
            lcd.print("Pos - ");
            while(pos > 0)
            {
                pos = getActuator();
                pos = pos / 10;
                actuatorDown(255);
                lcd.setCursor(6,1);
                lcd.print(pos);
                if(pos < 10)
                {
                    lcd.setCursor(7,1);
                    lcd.print("cm");
                    lcd.setCursor(9,1);
                    lcd.print(" ");
                }
                else
                {
                    lcd.setCursor(8,1);
                    lcd.print("cm");
                }
            }
            delay(1000);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Press A");
            lcd.setCursor(0,1);
            lcd.print("to continue...");
            menu = '0';
            last_key = up;
        }
        if(menu == '4')
        {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("shoot");
            shootArm();
            delay(500);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("reset");
            resetArm();
            delay(1000);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Press A");
            lcd.setCursor(0,1);
            lcd.print("to continue...");
            menu = '0';
        }
        if(menu == '5')
        {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("goForward");
            lcd.setCursor(0,1);
            lcd.print("PWM - ");
            for(int i = 0; i <= 255; i = i + 3)
            {
                goForward(i);
                if(i < 10)
                {
                    lcd.setCursor(6,1);
                    lcd.print(i);
                    lcd.setCursor(7,1);
                    lcd.print("  ");
                }
                if(i < 100)
                {
                    lcd.setCursor(6,1);
                    lcd.print(i);
                    lcd.setCursor(8,1);
                    lcd.print(" ");
                }
                else
                {
                    lcd.setCursor(6,1);
                    lcd.print(i);
                }
                delay(50);
            }
            delay(1000);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("motorBrake");
            motorBrake();
            delay(1000);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("goBackward");
            lcd.setCursor(0,1);
            lcd.print("PWM - ");
            for(int i = 255; i >= 0; i = i - 3)
            {
                goBackward(i);
                if(i < 10)
                {
                    lcd.setCursor(6,1);
                    lcd.print(i);
                    lcd.setCursor(7,1);
                    lcd.print("  ");
                }
                if(i < 100)
                {
                    lcd.setCursor(6,1);
                    lcd.print(i);
                    lcd.setCursor(8,1);
                    lcd.print(" ");
                }
                else
                {
                    lcd.setCursor(6,1);
                    lcd.print(i);
                }
                delay(50);
            }
            delay(1000);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Press A");
            lcd.setCursor(0,1);
            lcd.print("to continue...");
            menu = '0';
            last_key = up;
        }
        if(menu == '6')
        {
            set = false;
        }
    }

    lcd.setCursor(0,0);
    lcd.print("Busy wait...");
    while((!digitalRead(SWITCH7) != 1) && (!digitalRead(SWITCH6) != 1));
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Exiting...");
    delay(500);
    lcd.clear();

    pulse = 0;
}

void loop()
{
    /*if(!digitalRead(SWITCH8) == 1)
    {
        if((!digitalRead(SWITCH1) == HIGH) && (throw_ == false))
        {
            lcd.setCursor(0,1);
            lcd.print("TZ1 throw");
            setActuator(TZ1_ACT_LEN, ACT_SET_SPEED);
            while(getActuator() != TZ1_ACT_LEN);
            actuatorHold();
            delay(100);
            shootArm();
            throw_ = true;
            delay(1000);
        }
        if((!digitalRead(SWITCH1) == HIGH) && (throw_ == true))
        {
            lcd.setCursor(0,1);
            lcd.print("reset    ");
            resetArm();
            throw_ = false;
            delay(1000);
        }
        if((!digitalRead(SWITCH2) == HIGH) && (throw_ == false))
        {
            lcd.setCursor(0,1);
            lcd.print("TZ2 throw");
            setActuator(TZ2_ACT_LEN, ACT_SET_SPEED);
            while(getActuator() != TZ2_ACT_LEN);
            actuatorHold();
            delay(100);
            shootArm();
            throw_ = true;
            delay(1000);
        }
        if((!digitalRead(SWITCH2) == HIGH) && (throw_ == true))
        {
            lcd.setCursor(0,1);
            lcd.print("reset    ");
            resetArm();
            throw_ = false;
            delay(1000);
        }
        if((!digitalRead(SWITCH3) == HIGH) && (throw_ == false))
        {
            lcd.setCursor(0,1);
            lcd.print("TZ3 throw");
            setActuator(TZ3_ACT_LEN, ACT_SET_SPEED);
            while(getActuator() != TZ3_ACT_LEN);
            actuatorHold();
            delay(100);
            shootArm();
            throw_ = true;
            delay(1000);
        }
        if((!digitalRead(SWITCH3) == HIGH) && (throw_ == true))
        {
            lcd.setCursor(0,1);
            lcd.print("reset    ");
            resetArm();
            throw_ = false;
            delay(1000);
        }
    }*/
    int cell_read = analogRead(CELL_IN);
    float voltage = CELL_GRAD * cell_read;
    lcd.setCursor(0,0);
    lcd.print("CELL - ");
    lcd.setCursor(7,0);
    lcd.print(voltage);
    lcd.setCursor(11,0);
    lcd.print("V");
    
    if(!digitalRead(SWITCH8) == 1)
    {
        dirction = "C";
    }
    if(dirction == "F1")
    {   //448
        lcd.setCursor(0,1);
        lcd.print("Auto - F1");
        
        long distance = 420;
        String dir = "L1";
        
        actuatorDown(ACT_SET_SPEED);
        int s1 = !digitalRead(SWITCH1);
        int s2 = !digitalRead(SWITCH2);
        
        if(switch_cont == 1)
        {
            if((s1 == 1) && (s2 == 1))
            {
                goForward(speed_);
            }
            if((s1 == 0) || (s2 == 0))
            {
                goForwardRightDiagonal(speed_);
            }
        }
        if(pulse > (PULSE_PER_CM * (distance - STOPPING_DIST1)))
        {
            speed_ = SPEED_50;
        }
        if(pulse > (PULSE_PER_CM * (distance)))
        {
            motorBrake();
            switch_cont = 2;
        }
        if(switch_cont == 2)
        {
            speed_ = SPEED_50;
            if((s1 == 0) && (s2 == 0))
            {
                goRight(speed_);
            }
            if((s1 == 1) || (s2 == 1))
            {
                switch_cont = 3;
            }
        }
        if(switch_cont == 3)
        {
            speed_ = SPEED_150;
            if(s1 == 0)
            {
                shiftFrontRight(speed_);
            }
            if(s2 == 0)
            {
                shiftBackRight(speed_);
            }
            if((s1 == 1) && (s2 == 1))
            {
                pulse = 0;
                switch_cont = 1;
                speed_ = SPEED_255;
                dirction = dir;
            }
        }
    }
    if(dirction == "L1")
    {   //210
        lcd.setCursor(0,1);
        lcd.print("Auto - L1");
        
        long distance = 180;
        String dir = "B1";
        
        if(switch_cont == 1)
        {
            goLeft(speed_);
        }
        if(pulse > (PULSE_PER_CM * (distance - STOPPING_DIST1)))
        {
            speed_ = SPEED_25;
        }
        if(pulse > (PULSE_PER_CM * (distance)))
        {
            motorBrake();
            switch_cont = 2;
            pulse = 0;
            switch_cont = 1;
            speed_ = SPEED_25;
            dirction = dir;
        }
    }
    if(dirction == "B1")
    {
        lcd.setCursor(0,1);
        lcd.print("Auto - B1");
        
        String dir;
        
        if(pick_pos == 3)
        {
            dir = "R4";
        }
        else
        {
            dir = "R1";
        }
        int s3 = !digitalRead(SWITCH3);
        int s4 = !digitalRead(SWITCH4);
        int s5 = !digitalRead(SWITCH5);
        int s6 = !digitalRead(SWITCH6);

        if(switch_cont == 1)
        {
            goBackward(speed_);
            if((s3 == 1) || (s6 == 1))
            {
                speed_ = SPEED_100;
                motorBrake();
                switch_cont = 2;
            }
        }
        if(switch_cont == 2)
        {
            if((s3 == 1) && (s6 == 0))
            {
                shiftLeftBack(speed_);
            }
            if((s3 == 0) && (s6 == 1))
            {
                shiftRightBack(speed_);
            }
            if((s3 == 1) && (s6 == 1))
            {
                if((s4 == 1) && (s5 == 0))
                {
                    shiftLeftBack(speed_);
                }
                if((s4 == 0) && (s5 == 1))
                {
                    shiftRightBack(speed_);
                }
                if((s4 == 1) && (s5 == 1))
                {
                    pulse = 0;
                    switch_cont = 1;
                    speed_ = SPEED_50;
                    dirction = dir;
                }
            }
        }
    }
    if(dirction == "R1")
    {
        lcd.setCursor(0,1);
        lcd.print("Auto - R1");
        
        long distance = distance_tz1_recv;
        String dir;
        
        actuatorDown(ACT_SET_SPEED);
        if(pick_pos == 1)
        {
            dir = "L2";
        }
        if(pick_pos == 2)
        {
            dir = "R3";
        }
        if((pick_pos >= 4) && (pick_pos <= 8))
        {
            dir = "L3";
        }
        if(pick_pos > 8)
        {
            dir = "L2";
        }
        int s3 = !digitalRead(SWITCH3);
        int s4 = !digitalRead(SWITCH4);
        int s5 = !digitalRead(SWITCH5);
        
        if(switch_cont == 1)
        {
            if(s4 == 0)
            {
                shiftRightBack(speed_);
            }
            if(s5 == 0)
            {
                shiftLeftBack(speed_);
            }
            if((s4 == 1) && (s5 == 1) && (s3 == 1))
            {
                goRight(speed_);
            }
            if((s4 == 1) && (s5 == 1) && (s3 == 0))
            {
                pulse = 0;
                switch_cont = 2;
            }
        }
        if(switch_cont == 2)
        {
            goRight(speed_);
            if(pulse > (PULSE_PER_CM * (distance)))
            {
                motorBrake();
                while((!digitalRead(SWITCH7)) != 1)
                {
                    actuatorDown(ACT_SET_SPEED);
                }
                pick_pos = pick_pos + 1;
                pulse = 0;
                speed_ = SPEED_255;
                switch_cont = 1;
                dirction = dir;
            }
        }
    }
    if(dirction == "L2")
    {
        lcd.setCursor(0,1);
        lcd.print("Auto - L2");
        
        long distance = distance_tz1_recv + 110;//120;
        String dir = "R2";

        if(pick_pos == 2)
        {
            actuatorUp(ACT_SET_SPEED);
        }
        int s3 = !digitalRead(SWITCH3);
        int s4 = !digitalRead(SWITCH4);
        int s5 = !digitalRead(SWITCH5);
        int s6 = !digitalRead(SWITCH6);
        
        if(switch_cont == 1)
        {
            goLeft(speed_);
            delay(500);
            switch_cont = 2;
        }
        if(switch_cont == 2)
        {
            if(pick_pos == 2)
            {
                setActuator(TZ1_ACT_LEN, ACT_SET_SPEED);
            }
            if(pick_pos == 4)
            {
                setActuator(TZ2_ACT_LEN, ACT_SET_SPEED);
            }
            if((s3 == 1) && (s6 == 1))
            {
                goLeft(speed_);
            }
            if((s3 == 0) || (s6 == 0))
            {
                goBackLeftDiagonal(speed_);
            }
            if(pulse > (PULSE_PER_CM * (distance - STOPPING_DIST1)))
            {
                speed_ = SPEED_25;
            }
            if(pulse > (PULSE_PER_CM * (distance)))
            {
                motorBrake();
                speed_ = SPEED_100;
                switch_cont = 3;
            }
        }
        if(switch_cont == 3)
        {
            if(s5 == 0)
            {
                shiftLeftBack(speed_);
            }
            if(s4 == 0)
            {
                shiftRightBack(speed_);
            }
            if((s5 == 1) && (s4 == 1))
            {
                motorBrake();
                if(pick_pos == 2)
                {
                    while(getActuator() != TZ1_ACT_LEN)
                    {
                        setActuator(TZ1_ACT_LEN, ACT_SET_SPEED);
                        if(getActuator() == TZ1_ACT_LEN)
                        {
                            actuatorHold();
                            break;
                        }
                    }
                }
                if(pick_pos == 4)
                {
                    while(getActuator() != TZ2_ACT_LEN)
                    {
                        setActuator(TZ2_ACT_LEN, ACT_SET_SPEED);
                        if(getActuator() == TZ2_ACT_LEN)
                        {
                            actuatorHold();
                            break;
                        }
                    }
                }
                delay(500);
                shootArm();
                delay(500);
                resetArm();
                pulse = 0;
                switch_cont = 1;
                speed_ = SPEED_255;
                dirction = dir;
            }
        }
    }
    if(dirction == "R2")
    {   //100
        lcd.setCursor(0,1);
        lcd.print("Auto - R2");
        
        long distance = 120;
        String dir = "R1";

        actuatorDown(ACT_SET_SPEED);
        int s4 = !digitalRead(SWITCH4);
        int s5 = !digitalRead(SWITCH5);
        
        if(switch_cont == 1)
        {
            if((s5 == 1) && (s4 == 1))
            {
                goRight(speed_);
            }
            if((s5 == 0) || (s4 == 0))
            {
                goBackRightDiagonal(speed_);
            }
            if(pulse > (PULSE_PER_CM * (distance - STOPPING_DIST1)))
            {
                speed_ = SPEED_50;
                dirction = dir;
            }
        }
    }
    if(dirction == "R3")
    {
        lcd.setCursor(0,1);
        lcd.print("Auto - R3");
        
        long distance = 90; //ATTENTION::::: change back to 90
        String dir = "F2";

        actuatorUp(ACT_SET_SPEED);
        int s1 = !digitalRead(SWITCH1);
        int s2 = !digitalRead(SWITCH2);
        
        if(switch_cont == 1)
        {
            goRight(speed_);
            if(pulse > (PULSE_PER_CM * (distance - STOPPING_DIST1)))
            {
                speed_ = SPEED_25;//50
                if((s1 == 1) || (s2 == 1))
                {
                    motorBrake();
                    speed_ = SPEED_100;
                    switch_cont = 2;
                }
            }
        }
        if(switch_cont == 2)
        {
            if(s1 == 0)
            {
                shiftFrontRight(speed_);
            }
            if(s2 == 0)
            {
                shiftBackRight(speed_);
            }
            if((s1 == 1) && (s2 == 1))
            {
                pulse = 0;
                switch_cont = 1;
                speed_ = SPEED_255;
                dirction = dir;
            }
        }
    }
    if(dirction == "F2")
    {
        lcd.setCursor(0,1);
        lcd.print("Auto - F2");
        
        long distance = 220;
        String dir = "L1";
        
        setActuator(TZ2_ACT_LEN, ACT_SET_SPEED);
        int s1 = !digitalRead(SWITCH1);
        int s2 = !digitalRead(SWITCH2);
        
        if(switch_cont == 1)
        {
            if((s1 == 1) && (s2 == 1))
            {
                goForward(speed_);
            }
            if((s1 == 0) || (s2 == 0))
            {
                goForwardRightDiagonal(speed_); 
            }
        }
        if(pulse > (PULSE_PER_CM * (distance - STOPPING_DIST1)))
        {
            speed_ = SPEED_25;
        }
        if(pulse > (PULSE_PER_CM * (distance)))
        {
            motorBrake();
            switch_cont = 2;
        }
        if(switch_cont == 2)
        {
            speed_ = SPEED_50;
            if((s1 == 0) && (s2 == 0))
            {
                goRight(speed_);
            }
            if((s1 == 1) || (s2 == 1))
            {
                switch_cont = 3;
            }
        }
        if(switch_cont == 3)
        {
            speed_ = SPEED_150;
            if(s1 == 0)
            {
                shiftFrontRight(speed_);
            }
            if(s2 == 0)
            {
                shiftBackRight(speed_);
            }
            if((s1 == 1) && (s2 == 1))
            {
                pulse = 0;
                switch_cont = 1;
                speed_ = SPEED_255;
                dirction = dir;
            }
        }
    }
    if(dirction == "R4")
    {
        lcd.setCursor(0,1);
        lcd.print("Auto - R4");
        
        String dir = "L2";
        
        setActuator(TZ2_ACT_LEN, ACT_SET_SPEED);
        int s3 = !digitalRead(SWITCH3);
        int s4 = !digitalRead(SWITCH4);
        int s5 = !digitalRead(SWITCH5);
        
        if(switch_cont == 1)
        {
            if(s4 == 0)
            {
                shiftRightBack(speed_);
            }
            if(s5 == 0)
            {
                shiftLeftBack(speed_);
            }
            if((s4 == 1) && (s5 == 1) && (s3 == 1))
            {
                goRight(speed_);
            }
            if((s4 == 1) && (s5 == 1) && (s3 == 0))
            {
                switch_cont = 2;
            }
        }
        if(switch_cont == 2)
        {
            motorBrake();
            while((!digitalRead(SWITCH7)) != 1);
            pick_pos = pick_pos + 1;
            pulse = distance_tz1_recv * PULSE_PER_CM;
            speed_ = SPEED_255;
            switch_cont = 1;
            dirction = dir;
        }
    }
    if(dirction == "L3")
    {
        lcd.setCursor(0,1);
        lcd.print("Auto - L3");

        long distance = distance_tz1_recv + 390;
        String dir = "R5";
        
        setActuator(TZ3_ACT_LEN, ACT_SET_SPEED);
        int s3 = !digitalRead(SWITCH3);
        int s4 = !digitalRead(SWITCH4);
        int s5 = !digitalRead(SWITCH5);
        int s6 = !digitalRead(SWITCH6);
        
        if(switch_cont == 1)
        {
            goLeft(speed_);
            delay(500);
            switch_cont = 2;
        }
        if(switch_cont == 2)
        {
            if((s3 == 1) && (s6 == 1))
            {
                goLeft(speed_);
            }
            if((s3 == 0) || (s6 == 0))
            {
                goBackLeftDiagonal(speed_);
            }
            if(pulse > (PULSE_PER_CM * (distance - STOPPING_DIST1)))
            {
                speed_ = SPEED_25;
            }
            if(pulse > (PULSE_PER_CM * (distance)))
            {
                motorBrake();
                speed_ = SPEED_100;
                switch_cont = 3;
            }
        }
        if(switch_cont == 3)
        {
            if(s5 == 0)
            {
                shiftLeftBack(speed_);
            }
            if(s4 == 0)
            {
                shiftRightBack(speed_);
            }
            if((s5 == 1) && (s4 == 1))
            {
                motorBrake();
                while(getActuator() != TZ3_ACT_LEN)
                {
                    setActuator(TZ3_ACT_LEN, ACT_SET_SPEED);
                    if(getActuator() == TZ3_ACT_LEN)
                    {
                        actuatorHold();
                        break;
                    }
                }
                delay(500);
                shootArm();
                delay(500);
                resetArm();
                pulse = 0;
                switch_cont = 1;
                speed_ = SPEED_255;
                dirction = dir;
            }
        }
    }
    if(dirction == "R5")
    {
        lcd.setCursor(0,1);
        lcd.print("Auto - R5");
        
        long distance = 390;
        String dir = "R1";

        actuatorDown(ACT_SET_SPEED);
        int s4 = !digitalRead(SWITCH4);
        int s5 = !digitalRead(SWITCH5);
        
        if(switch_cont == 1)
        {
            if((s5 == 1) && (s4 == 1))
            {
                goRight(speed_);
            }
            if((s5 == 0) || (s4 == 0))
            {
                goBackRightDiagonal(speed_);
            }
            if(pulse > (PULSE_PER_CM * (distance - STOPPING_DIST1)))
            {
                speed_ = SPEED_50;
                dirction = dir;
            }
        }
    }
    if(dirction == "C")
    {
        controller();
    }
}

void controller()
{
    int stick_xin = analogRead(STICK_X);
    int stick_yin = analogRead(STICK_Y);
    if((stick_xin >= CONT_UPPER_LOW) && (stick_xin <= CONT_UPPER_HIGH))
    {
        byte speed_ = mapSpeed(stick_xin, CONT_UPPER_LOW, CONT_UPPER_HIGH, 0, 255);
        goForward(speed_);
        lcd.setCursor(0,1);
        lcd.print("Manual - Forward");
    }
    else if((stick_xin >= CONT_LOWER_LOW) && (stick_xin <= CONT_LOWER_HIGH))
    {
        byte speed_ = mapSpeed(stick_xin, CONT_LOWER_LOW, CONT_LOWER_HIGH, 0, 255);
        goBackward(255 - speed_);
        lcd.setCursor(0,1);
        lcd.print("Manual - Bckward");
    }
    else if((stick_yin >= CONT_UPPER_LOW) && (stick_yin <= CONT_UPPER_HIGH))
    {
        byte speed_ = mapSpeed(stick_yin, CONT_UPPER_LOW, CONT_UPPER_HIGH, 0, 255);
        goRight(speed_);
        lcd.setCursor(0,1);
        lcd.print("Manual - Left   ");
    }
    else if((stick_yin >= CONT_LOWER_LOW) && (stick_yin <= CONT_LOWER_HIGH))
    {
        byte speed_ = mapSpeed(stick_yin, CONT_LOWER_LOW, CONT_LOWER_HIGH, 0, 255);
        goLeft(255 - speed_);
        lcd.setCursor(0,1);
        lcd.print("Manual - Right  ");
    }
    else
    {
        changeMotorSpeed(0);
        /*lcd.setCursor(0,1);
        lcd.print("Manual - Brake  ");*/
    }
}

void setActuator(int len, byte motor_speed)
{
    int actuator = getActuator();
    if(actuator < (len - ACT_ERROR))
    {
        actuatorUp(motor_speed);
    }
    if(actuator > (len + ACT_ERROR))
    {
        actuatorDown(motor_speed);
    }
    if((actuator > (len - ACT_ERROR)) && (actuator < (len + ACT_ERROR)))
    {
        actuatorHold();
    }
}

int getActuator()
{
    int value = analogRead(ACT_POT_IN);
    value = mapSpeed(value, ACT_POT_MIN, ACT_POT_MAX, ACT_LEN_MIN, ACT_LEN_MAX);
    return value;
}

void actuatorUp(byte motor_speed)
{
    digitalWrite(ACT_BACK_EMF, HIGH);
    digitalWrite(ACT_IN1, HIGH);
    digitalWrite(ACT_IN2, LOW);
    analogWrite(ACT_PWM, motor_speed);
}

void actuatorDown(byte motor_speed)
{
    digitalWrite(ACT_BACK_EMF, HIGH);
    digitalWrite(ACT_IN1, LOW);
    digitalWrite(ACT_IN2, HIGH);
    analogWrite(ACT_PWM, motor_speed);
}

void actuatorHold()
{
    digitalWrite(ACT_IN1, LOW);
    digitalWrite(ACT_IN1, LOW);
    digitalWrite(ACT_BACK_EMF, LOW);
    analogWrite(ACT_PWM, 0);
}

void shootArm()
{
    digitalWrite(RELAY_IN2, HIGH);
    delay(SHOOT_DELAY);
    digitalWrite(RELAY_IN2, LOW);
}

void resetArm()
{
    digitalWrite(RELAY_IN1, HIGH);
    delay(RESET_DELAY);
    digitalWrite(RELAY_IN1, LOW);
}

void getMotorEncoder()
{
    pulse = pulse + 1;
}

void changeMotorSpeed(byte motor_speed)
{
    analogWrite(MOTOR1_PWM, motor_speed);
    analogWrite(MOTOR2_PWM, motor_speed);
    analogWrite(MOTOR3_PWM, motor_speed);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void shiftRightBack(byte motor_speed)
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
    digitalWrite(MOTOR4_IN2, LOW);
    analogWrite(MOTOR4_PWM, 0);
}

void shiftLeftBack(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, HIGH);
    digitalWrite(MOTOR1_IN2, LOW);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, HIGH);
    digitalWrite(MOTOR2_IN2, LOW);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, LOW);
    digitalWrite(MOTOR3_IN2, LOW);
    analogWrite(MOTOR3_PWM, 0);
    
    digitalWrite(MOTOR4_IN1, HIGH);
    digitalWrite(MOTOR4_IN2, LOW);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void shiftBackRight(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, HIGH);
    digitalWrite(MOTOR1_IN2, LOW);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, LOW);
    digitalWrite(MOTOR2_IN2, LOW);
    analogWrite(MOTOR2_PWM, 0);
    
    digitalWrite(MOTOR3_IN1, HIGH);
    digitalWrite(MOTOR3_IN2, LOW);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, HIGH);
    digitalWrite(MOTOR4_IN2, LOW);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void shiftFrontRight(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, LOW);
    digitalWrite(MOTOR1_IN2, HIGH);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, LOW);
    digitalWrite(MOTOR2_IN2, HIGH);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, LOW);
    digitalWrite(MOTOR3_IN2, LOW);
    analogWrite(MOTOR3_PWM, 0);
    
    digitalWrite(MOTOR4_IN1, LOW);
    digitalWrite(MOTOR4_IN2, HIGH);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void goBackRightDiagonal(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, LOW);
    digitalWrite(MOTOR1_IN2, HIGH);
    analogWrite(MOTOR1_PWM, 0);
    
    digitalWrite(MOTOR2_IN1, LOW);
    digitalWrite(MOTOR2_IN2, HIGH);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, HIGH);
    digitalWrite(MOTOR3_IN2, LOW);
    analogWrite(MOTOR3_PWM, 0);
    
    digitalWrite(MOTOR4_IN1, HIGH);
    digitalWrite(MOTOR4_IN2, LOW);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void goForwardLeftDiagonal(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, HIGH);
    digitalWrite(MOTOR1_IN2, LOW);
    analogWrite(MOTOR1_PWM, 0);
    
    digitalWrite(MOTOR2_IN1, HIGH);
    digitalWrite(MOTOR2_IN2, LOW);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, LOW);
    digitalWrite(MOTOR3_IN2, HIGH);
    analogWrite(MOTOR3_PWM, 0);
    
    digitalWrite(MOTOR4_IN1, LOW);
    digitalWrite(MOTOR4_IN2, HIGH);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void goBackLeftDiagonal(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, HIGH);
    digitalWrite(MOTOR1_IN2, LOW);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, HIGH);
    digitalWrite(MOTOR2_IN2, LOW);
    analogWrite(MOTOR2_PWM, 0);
    
    digitalWrite(MOTOR3_IN1, LOW);
    digitalWrite(MOTOR3_IN2, HIGH);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, LOW);
    digitalWrite(MOTOR4_IN2, HIGH);
    analogWrite(MOTOR4_PWM, 0);
}

void goForwardRightDiagonal(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, LOW);
    digitalWrite(MOTOR1_IN2, HIGH);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, HIGH);
    digitalWrite(MOTOR2_IN2, LOW);
    analogWrite(MOTOR2_PWM, 0);
    
    digitalWrite(MOTOR3_IN1, HIGH);
    digitalWrite(MOTOR3_IN2, LOW);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, LOW);
    digitalWrite(MOTOR4_IN2, HIGH);
    analogWrite(MOTOR4_PWM, 0);
}

void goRight(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, LOW);
    digitalWrite(MOTOR1_IN2, HIGH);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, LOW);
    digitalWrite(MOTOR2_IN2, HIGH);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, HIGH);
    digitalWrite(MOTOR3_IN2, LOW);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, HIGH);
    digitalWrite(MOTOR4_IN2, LOW);
    analogWrite(MOTOR4_PWM, motor_speed);
}

void goLeft(byte motor_speed)
{
    digitalWrite(MOTOR1_IN1, HIGH);
    digitalWrite(MOTOR1_IN2, LOW);
    analogWrite(MOTOR1_PWM, motor_speed);
    
    digitalWrite(MOTOR2_IN1, HIGH);
    digitalWrite(MOTOR2_IN2, LOW);
    analogWrite(MOTOR2_PWM, motor_speed);
    
    digitalWrite(MOTOR3_IN1, LOW);
    digitalWrite(MOTOR3_IN2, HIGH);
    analogWrite(MOTOR3_PWM, motor_speed);
    
    digitalWrite(MOTOR4_IN1, LOW);
    digitalWrite(MOTOR4_IN2, HIGH);
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
