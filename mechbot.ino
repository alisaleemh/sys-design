#include <mechbotShield.h>
#include <USART.h>
#include <avr/io.h>

int vel = 300, tvel = 500, lsen_hi = 800, lsen_low = 600;
int error = 0 ; int previousError = 0 ;

double Kp = 65 ; double Kd = 20 ; double Ki = 0.001 ;
int motorSpeed = 300 ;
int leftMotorSpeed = 0, rightMotorSpeed = 0;
double P ;
double I ;
double D ;
double PIDvalue;


/* 2-d array [ sensor1[on-line, not-on-line, edge], sensor2[on-line, not-on-line, edge]// */
int sensorCalib[4][3] = {
    { 900, 600, 600 },
    { 900, 600, 600 },
    { 900, 600, 600 },
    { 900, 800, 800 }
};


// line sensor array
int lineSensor[4] ;

void moveStraight();
void turnRight();
void turnLeft();
void initialize();
void readLineSensor();
void calculatePidError();
double calculatePidValue();
void PidMotorControl(double PIDvalue);

int main(void)
{
    initialize();

    while (1) {
        readLineSensor();
        calculatePidError();
        PidMotorControl(calculatePidValue());
    }

}

// function definitions

void calculatePidError () {
     // line sensors = 1 1 1 1
     if (lineSensor[0] >= sensorCalib[0][0] && lineSensor[1] >= sensorCalib[1][0] && lineSensor[0] >= sensorCalib[2][0] && lineSensor[3] >= sensorCalib[3][0] ) {
         error = 10 ;
     }
     // line sensors = 1 0 0 1
     else if (lineSensor[0] >= sensorCalib[0][0] && lineSensor[1] < sensorCalib[1][0] && lineSensor[2] < sensorCalib[2][0] && lineSensor[3] >= sensorCalib[3][0] ) {
         error = 10 ;
     }
    // // line sensors = 1 1 0 1
    // else if (lineSensor[0] >= sensorCalib[0][2] && lineSensor[1] >= sensorCalib[1][2] && lineSensor[2] < sensorCalib[2][2] && lineSensor[3] >= sensorCalib[3][2] ) {
    //     stopIntersection() ;
    //     error = 10 ;
    //
    // }
    // // line sensors = 1 0 1 1
    // else if (lineSensor[0] >= sensorCalib[0][2] && lineSensor[1] < sensorCalib[1][2] && lineSensor[2] >= sensorCalib[2][2] && lineSensor[3] >= sensorCalib[3][2] ) {
    //     stopIntersection() ;
    //     error = 10 ;
    //
    // }

    //line sensors = 1 0 0 0
    if (lineSensor[0] >= sensorCalib[0][2] && lineSensor[1] < sensorCalib[1][2] && lineSensor[2] < sensorCalib[2][2] && lineSensor[3] < sensorCalib[3][2]) {
        error = -3;
    }
    // line sensors = 1 1 0 0
    else if (lineSensor[0] >= sensorCalib[0][2] && lineSensor[1] >= sensorCalib[1][2] && lineSensor[2] < sensorCalib[2][2] && lineSensor[3] < sensorCalib[3][2]) {
        error = -2;
    }
    // line sensors = 0 1 0 0
    else if (lineSensor[0] < sensorCalib[0][2] && lineSensor[1] >= sensorCalib[1][2] && lineSensor[2] < sensorCalib[2][2] && lineSensor[3] < sensorCalib[3][2] ) {
        error = -1;
    }

    // line sensors = 0 1 1 0
    else if (lineSensor[0] < sensorCalib[0][2] && lineSensor[1] >= sensorCalib[1][2] && lineSensor[2] >= sensorCalib[2][2] && lineSensor[3] < sensorCalib[3][2]) {
        error = 0;
    }

    // line sensors = 0 0 0 1
    else if (lineSensor[0] < sensorCalib[0][2] && lineSensor[1] < sensorCalib[1][2] && lineSensor[2] < sensorCalib[2][2] && lineSensor[3] >= sensorCalib[3][2]) {
        error = 3;
    }
    // line sensors = 0 0 1 1
    else if (lineSensor[0] < sensorCalib[0][2] && lineSensor[1] < sensorCalib[1][2] && lineSensor[2] >= sensorCalib[2][2] && lineSensor[3] >= sensorCalib[3][2]) {
        error = 2;
    }
    // line sensors = 0 1 0 0
    else if (lineSensor[0] < sensorCalib[0][2] && lineSensor[1] < sensorCalib[1][2] && lineSensor[2] >= sensorCalib[2][2] && lineSensor[3] < sensorCalib[3][2] ) {
        error = -1;
    }
}

double calculatePidValue () {

    if(error == 10) {
        return -100000 ;
    }

    P = error;
    I = I + error;
    D = error-previousError;
    PIDvalue = (Kp*P) + (Ki*I) + (Kd*D);
    previousError = error;


    return PIDvalue ;
}

void PidMotorControl(double PIDvalue) {

    if (PIDvalue == -100000) {
        stopIntersection();
        return;
    }

    leftMotorSpeed = motorSpeed + PIDvalue;
    rightMotorSpeed = motorSpeed - PIDvalue;

    constrain(leftMotorSpeed, 0, 1000);
    constrain(rightMotorSpeed, 0 , 1000);

    motor(leftMotorSpeed, rightMotorSpeed);

}




void initialize()
{
    //ADC lineSensor sensors
    DDRD &= ~(1 << PD3);
    PORTD |= 0b00000000;

    //setting bumper button as input
    DDRD &= ~(1 << PD4);
    PORTD |= (1 << PD4);

    //setting motors as output
    DDRB |= ((1 << PB1) | (1 << PB2));
    PORTB &= ~((1 << PB1) | (1 << PB2));

    initSoftSerial();
    initMotor();
    setLCDBackLight(255);
    moveLCDCursor(0);
    initADC();
}

void readLineSensor()
{
    for (int i=0 ; i <= 3 ; i++) {
        lineSensor[i] = analog(i);
    }
}
void stopIntersection()
{
    motor(0, 0);
    delay(5000);

    moveLCDCursor(0);
    lcdPrint("Stopped");
    motor(500,500);
    delay(1000);


}
