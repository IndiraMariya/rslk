// MotorSimple.c
// Runs on MSP432
// Provide mid-level functions that initialize ports and
// set motor speeds to move the robot.
// Starter code for Lab 12, uses Systick software delay to create PWM
// Daniel Valvano
// July 7, 2017

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
       ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
       ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
       ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2017
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2017, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// Sever VCCMD=VREG jumper on Motor Driver and Power Distribution Board and connect VCCMD to 3.3V.
//   This makes P3.7 and P3.6 low power disables for motor drivers.  0 to sleep/stop.
// Sever nSLPL=nSLPR jumper.
//   This separates P3.7 and P3.6 allowing for independent control
// Left motor direction connected to P1.7 (J2.14)
// Left motor PWM connected to P2.7/TA0CCP4 (J4.40)
// Left motor enable connected to P3.7 (J4.31)
// Right motor direction connected to P1.6 (J2.15)
// Right motor PWM connected to P2.6/TA0CCP3 (J4.39)
// Right motor enable connected to P3.6 (J2.11)

#include <stdint.h>
#include "msp.h"
#include "../inc/SysTick.h"
#include "../inc/Bump.h"
#define P2_5 (*((volatile uint8_t *)(0x42098074)))
// *******Lab 12 solution*******

void Motor_InitSimple(void){
// Initializes the 6 GPIO lines and puts driver to sleep
// Returns right away
// initialize P1.6 and P1.7 and make them outputs

    /* P5.4, P5.5:    Left, Right Dir
     * P2.7, P2.6:    Left, Right PWM
     * P3.7, P3.6:    Left, Right SLP
     */
    P5->SEL0 &= 0xCF;
    P2->SEL0 &= 0x3F;
    P3->SEL0 &= 0x3F;
    P5->SEL1 &= 0xCF;
    P2->SEL1 &= 0x3F;
    P3->SEL1 &= 0x3F;
    P5->OUT &= 0xCF;
    P2->OUT &= 0x3F;
    P3->OUT &= 0x3F;
    P5->DIR |= ~0xCF;
    P2->DIR |= ~0x3F;
    P3->DIR |= ~0x3F;
}

void Motor_StopSimple(void){
// Stops both motors, puts driver to sleep, sets direction to forward
// Returns right away
    P2->OUT &= 0b00111111;
    P5->OUT &= 0b00110000;
    P7->OUT &= 0b00111111;
}

void Motor_ForwardSimple(uint16_t duty, uint32_t time){
// Drives both motors forward at duty (100 to 9900), period is 10000 (10mS)
// Runs for time duration (units=10ms) using a loop, and then stops
// Stop the motors and return if any bumper switch is active
// Check this at the BEGINING of each loop.
// Returns after time*10ms or if a bumper switch is hit

// TODO: Write this function
    P5->OUT &= 0b00110000;
    P3->OUT |= 0b11000000;

    uint32_t high = duty * 48;
    uint32_t low = 480000 - 48 * duty;
    uint32_t loop = time;
    while(loop--) {
        if (Bump_Read()) {
            Motor_StopSimple();
            return;
        }
        P2->OUT |= 0b11000000;
        Systick_Wait(high);
        P2->OUT &= 0b00111111;
        Systick_Wait(low);
    }
    Motor_StopSimple();
}

void Motor_BackwardSimple(uint16_t duty, uint32_t time){
// Drives both motors backward at duty (100 to 9900)
// Runs for time duration (units=10ms), and then stops
// Runs even if any bumper switch is active
// Returns after time*10ms
    P5->OUT |= 0b00110000;
    P3->OUT |= 0b11000000;

    uint32_t high = duty * 48;
    uint32_t low = 480000 - 48 * duty;
    uint32_t loop = time;

    while(loop--) {
        P2->OUT |= 0b11000000;
        Systick_Wait(high);
        P2->OUT &= 0b00111111;
        Systick_Wait(low);
    }
    Motor_StopSimple();

}

void Motor_LeftSimple(uint16_t duty, uint32_t time){
// Drives just the left motor forward at duty (100 to 9900)
// Right motor is stopped (sleeping)
// Runs for time duration (units=10ms), and then stops
// Stop the motor and return if any bumper switch is active
// Returns after time*10ms or if a bumper switch is hit

    P5->OUT &= 0b00010000;
    P3->OUT |= 0b10000000;

    uint32_t high = duty * 48;
    uint32_t low = 480000 - 48 * duty;
    uint32_t loop = time;
    while(loop--) {
        if (Bump_Read()) {
            Motor_StopSimple();
            return;
        }
        P2->OUT |= 0b10000000;
        Systick_Wait(high);
        P2->OUT &= 0b00111111;
        Systick_Wait(low);
    }
    Motor_StopSimple();
}

void Motor_RightSimple(uint16_t duty, uint32_t time){
// Drives just the right motor forward at duty (100 to 9900)
// Left motor is stopped (sleeping)
// Runs for time duration (units=10ms), and then stops
// Stop the motor and return if any bumper switch is active
// Returns after time*10ms or if a bumper switch is hit

    P5->OUT &= 0b00100000;
    P3->OUT |= 0b01000000;

    uint32_t high = duty * 48;
    uint32_t low = 480000 - 48 * duty;
    uint32_t loop = time;
    while(loop--) {
        if (Bump_Read()) {
            Motor_StopSimple();
            return;
        }
        P2->OUT |= 0b01000000;
        Systick_Wait(high);
        P2->OUT &= 0b00111111;
        Systick_Wait(low);
    }
    Motor_StopSimple();
}
