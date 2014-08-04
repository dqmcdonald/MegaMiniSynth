/***************************************************************************
 * Mega Mini Synth
 * 
 * An Arduino Mega2560 based music synthesizer
 * 
 * See http://megaminisynth.blogspot.co.nz/ for details.
 * 
 * This file is used for the timer based oscillator class. This is what is used
 * to generate the pulses for the waveshapers. The oscillator frequency is determined
 * by incoming MIDI messages.
 * 
 * 
 * D. Quentin McDonald
 * dqmcdonald@gmail.com
 * 2014
 * 
 ****************************************************************************/

#include <math.h>


// frequencies for midi notes 0 - 127
// This data and much of the code in this file is based on the work described in
// http://www.sinneb.net/?p=161
// The idea is that for certain frequency ranges different values of the prescaler of the 
// timer will give more accurate results. Hence the values in this array are not increasing
// monotonically as MIDI notes -10 use prescale 64, the next 35 using 8 and the rest with 1.
// The method setPreScaling implements this.
int musical_freqs[] = { 
  15289, 14431, 13621, 12857, 12135, 11454, 10811, 10204, 9631, 9091, 8581, 64792, 61156,
  57724, 54484, 51426, 48540, 45815, 43244, 40817, 38526, 36364, 34323, 32396, 30578, 28862,
  27242, 25713, 24270, 22908, 21622, 20408, 19263, 18182, 17161, 16198, 15289, 14431, 13621,
  12856, 12135, 11454, 10811, 10204, 9631, 9091, 8581, 64793, 61156, 57724, 54484, 51426, 48540,
  45815, 43244, 40817, 38526, 36364, 34323, 32396, 30578, 28862, 27242, 25713, 24270, 22908, 21622,
  20408, 19263, 18182, 17161, 16198, 15289, 14431, 13621, 12856, 12135, 11454, 10811, 10204, 9631, 9091,
  8581, 8099, 7645, 7215, 6810, 6428, 6067, 5727, 5405, 5102, 4816, 4545, 4290, 4050, 3822, 3608, 3405,
  3214, 3034, 2863, 2703, 2551, 2408, 2273, 2145, 2025, 1911, 1804, 1703, 1607, 1517, 1432, 1351, 1276,
  1204, 1136, 1073, 1012, 956, 902, 851, 804, 758, 716, 676, 638 };


// Set up an oscillator on the specified Timer

Oscillator::Oscillator( int timer ) 
{
  m_note = 69; // A 440 Hz
  m_pitch_bend = 0; // Start with no pitch bend

  m_timer = timer;
}

void Oscillator::begin() {
  if( m_timer == 1 ) {

    // Timer1 is on pin 11
    pinMode(TIMER1_OUT, OUTPUT);
    // Timer/Counter 1 :: registers TCCR1A and TCCR1B

    // Clear Timer on Compare Match (CTC) Mode
    bitWrite(TCCR1A, WGM10, 0);
    bitWrite(TCCR1A, WGM11, 0);
    bitWrite(TCCR1B, WGM12, 1);
    bitWrite(TCCR1B, WGM13, 0);

    // Toggle OC1A on Compare Match. p.134
    // arduino mega hw pin 11
    bitWrite(TCCR1A, COM1A0, 1);
    bitWrite(TCCR1A, COM1A1, 0);

    // prescaling
    setPreScaling(m_note);

    // set output compare register A to curr_midinote (init: 69)
    OCR1A = musical_freqs[m_note];

  }
}


// Set the current MIDI note and update the oscillator to the corresponding frequency
// How we do this depends on whether there is any pitch-bend in action.
void Oscillator::setNote( int current_note )    
{
  m_note = current_note;
  float ratio;
  float factor;
  int freq;

  setPreScaling(m_note);

  if( m_pitch_bend == 0 ) { 
    OCR1A = musical_freqs[m_note];
  } 
  else {
    // Implement pitch bend. The MIDI standard suggests that the pitch bend is +/- two 
    // semitones (or two MIDI notes). 
    // To figure out the bend factor we need to find out what multiplication factor
    // we need. This will be. Each octave doubles in frequency and there are tweleve
    // semi-tones per octave. Therefore we need a factor of
    // 2**(r*2/12) or 2**(r/6) where r is the ratio 
    ratio = fabs((float)m_pitch_bend/ MIDI_PITCHBEND_MAX);
    factor = pow(2,(ratio/6));
    if( m_pitch_bend < 0 ){
      freq = musical_freqs[m_note] * factor;
    } else {
      freq = musical_freqs[m_note] / factor;
    }
    OCR1A = freq;
  }
  
} 

// Set the amount of pitch bend that is currently being applied, then replay the current
// note to enact that.
void Oscillator::setPitchBend( int pitch_bend ) 
{
  m_pitch_bend = pitch_bend;
  setNote( m_note );
}


void Oscillator::setPreScaling (int midinote) {
  cli(); // interrupts off

  if( m_timer == 1 ) {
    if (midinote < 11) {
      // prescaling * 64
      bitWrite(TCCR1B, CS10, 1);
      bitWrite(TCCR1B, CS11, 1);
      bitWrite(TCCR1B, CS12, 0);
    }
    if (midinote > 10 && midinote < 47) {
      // prescaling * 8
      bitWrite(TCCR1B, CS10, 0);
      bitWrite(TCCR1B, CS11, 1);
      bitWrite(TCCR1B, CS12, 0);
    }
    if (midinote > 46) {
      // prescaling * 1
      bitWrite(TCCR1B, CS10, 1);
      bitWrite(TCCR1B, CS11, 0);
      bitWrite(TCCR1B, CS12, 0);
      //Serial.println("prescale1");
    }
  }
  sei(); // interrupts on

}

// Test the pitch bend scaling strategy
// 0 == linear, 1 = power
void Oscillator::testScaling(int scaling) {
  
  int  start_freq= 30578;
  int end_freq = 27242;
  int diff = end_freq - start_freq;
  int freq;
  float ratio;
  
  const int NUM_STEPS = 50;
  setPreScaling(60);
  OCR1A = freq;
  for( int i=0; i<= NUM_STEPS; i++)
  {
    ratio = (float)i/NUM_STEPS;
    if( scaling == 0 ) {
      freq = start_freq + int(diff*ratio);
    } else {
      freq = start_freq/pow(2,(ratio/6));
      
    }
    Serial.println(freq);
    OCR1A = freq;
    delay(100);
  }
  
  
  
}


