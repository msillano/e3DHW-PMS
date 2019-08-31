/*
NiMH_decharger.ino - program for ATtiny85 (Digispark)
  Copyright (c) 2019 Marco Sillano.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/*
 Sketch for Digispark ATtiny85
 Application: NiMH Battery safe decharger
 */
// --------------------- hardware
#define AIN      A1          /* analog input      board P2*/
#define MOSFET   0           /* MOSFET output     board P0*/
#define RLEDOUT  1           /* RED LED output    board P1*/
#define GLEDOUT  4           /* GREEN LED output  board P4, do not use P3, pullup*/

#define  ADC_STEPS 1024    /* 10 bit ADC */
#define  ADC_REF   1100    /* internal reference, mV */
//---------------------- NiMH batteries
#define  NIMH_NONE  400    /* no battery mV */
#define  NIMH_LOW   900    /* low battery mV */
#define  ADC_ADJ    -76    /* added to NIMH_LOW value, adjust */
//---------------------- timing
#define  TBLINK   600
#define  TBEEP    200
//----------------------- finite-state automaton
typedef enum {aempty = 0, alow = 1, ahigh = 2, adone = 3} status;

status automa = aempty;
int cycles = 0;

void setup() {
  pinMode(MOSFET, OUTPUT);
  pinMode(RLEDOUT, OUTPUT);
  pinMode(GLEDOUT, OUTPUT);
  digitalWrite(MOSFET, LOW);
  digitalWrite(RLEDOUT, LOW);
  digitalWrite(GLEDOUT, LOW);
  analogReference(INTERNAL1V1);
}

void loop() {
  int value = analogRead(AIN);
  //to mV
  value = map(value, 0, 1024, 0, ADC_REF);
  //================= Moore machine: automaton status changes
  if (value < NIMH_NONE) {
    automa = aempty;
  } else if (value < (NIMH_LOW + ADC_ADJ)) {

    if ((automa == ahigh) || (automa == adone)) {
      if (cycles++ > 3)
        automa = adone;
    }
    else
      automa = alow;
  } else if (automa != adone) {
    cycles = 0;
    automa = ahigh;
  }
  //================ Moore machine: automaton outputs(status).
  if (automa ==  aempty) {
    // LEDs OFF
    digitalWrite(MOSFET, LOW);
    digitalWrite(RLEDOUT, LOW);
    digitalWrite(GLEDOUT, LOW);
  }
  if (automa ==  alow) {
    digitalWrite(MOSFET, LOW);
    // RED LED ON
    digitalWrite(RLEDOUT, HIGH);
    digitalWrite(GLEDOUT, LOW);
  }
  if (automa ==  ahigh) {
    // RED LED BLINK, charge ON
    digitalWrite(MOSFET, HIGH);
    digitalWrite(GLEDOUT, LOW);
    digitalWrite(RLEDOUT, !digitalRead(RLEDOUT));
  }
  if (automa ==  adone) {
    digitalWrite(MOSFET, LOW);
    digitalWrite(RLEDOUT, LOW);
    // RED GREEN + BEEP
    for (int i = 0; i < TBEEP; i++) {
      digitalWrite(GLEDOUT, !digitalRead(GLEDOUT));
      delay(1);
    }
    digitalWrite(GLEDOUT, HIGH);
    delay(TBLINK - TBEEP);
  }
  else
    delay(TBLINK);
}
