
// DCO. Based on code from http://www.sinneb.net/?p=161

#include <MIDI.h>


MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// frequencies for midi notes 0 - 127
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


int current_note = 69;
int current_pitch_bend = 0;





void setup() {
  pinMode(11, OUTPUT);

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
  set_prescaling(current_note);

  // set output compare register A to curr_midinote (init: 69)
  OCR1A = musical_freqs[current_note];

  pinMode(A0,INPUT);

  Serial.begin(9600);

  pinMode( 13, OUTPUT);
  digitalWrite( 13, LOW );


  // Connect the callbacks:
  // 
  MIDI.setHandleNoteOn(handleNoteOn);  
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandlePitchBend(handlePitchBend);

  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

}

void set_prescaling (int midinote) {
  cli(); // interrupts off

  if (midinote < 11) {
    // prescaling * 64
    bitWrite(TCCR1B, CS10, 1);
    bitWrite(TCCR1B, CS11, 1);
    bitWrite(TCCR1B, CS12, 0);
    //Serial.println("prescale64");
  }
  if (midinote > 10 && midinote < 47) {
    // prescaling * 8
    bitWrite(TCCR1B, CS10, 0);
    bitWrite(TCCR1B, CS11, 1);
    bitWrite(TCCR1B, CS12, 0);
    //Serial.println("prescale8");
  }
  if (midinote > 46) {
    // prescaling * 1
    bitWrite(TCCR1B, CS10, 1);
    bitWrite(TCCR1B, CS11, 0);
    bitWrite(TCCR1B, CS12, 0);
    //Serial.println("prescale1");
  }

  sei(); // interrupts on

}

void play_current_note( int current_note ){
  float ratio;
  int offset;

  if( current_pitch_bend == 0 ) {
    set_prescaling(current_note);
    OCR1A = musical_freqs[current_note];
  } 
  else {
    set_prescaling(current_note);
    ratio = (float)current_pitch_bend/ MIDI_PITCHBEND_MAX;
    if( current_note < 125 ) {
      offset = int((musical_freqs[current_note+2] - musical_freqs[current_note]) * ratio);

      OCR1A = musical_freqs[current_note] + offset;
    }
  }
}


void loop() {

  // Call MIDI.read the fastest you can for real-time performance.
  MIDI.read();

}


void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  // Do whatever you want when a note is pressed.
  // Try to keep your callbacks short (no delays ect)
  // otherwise it would slow down the loop() and have a bad impact
  // on real-time performance.

  current_note = pitch;
  play_current_note(current_note);
  digitalWrite( 13, HIGH );
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  // Do something when the note is released.
  // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.
  digitalWrite( 13, LOW );
}

void handlePitchBend( byte channel, int bend )
{
  current_pitch_bend = bend;
  play_current_note( current_note );

}










