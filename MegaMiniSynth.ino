/***************************************************************************
 * Mega Mini Synth
 * 
 * An Arduino Mega2560 based music synthesizer
 * 
 * See http://megaminisynth.blogspot.co.nz/ for details.
 * 
 * This file is the main routine for the synth - all command and control operations
 * are performed from here.
 * 
 * D. Quentin McDonald
 * dqmcdonald@gmail.com
 * 2014
 * 
 ****************************************************************************/

#include <MIDI.h>


MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// Pin definitions:

#define TIMER1_OUT     11







class Oscillator {

public:
  //
  Oscillator( int timer );        // Create an instance of an Oscillator using Timer "timer"
  void setNote( int current_note );     // Set the current MIDI note
  void setPitchBend( int pitch_bend ); // Set the Pitchbend amount

protected:
  void setPreScaling( int midi_note );// Set up the pre-scaler appropriate for the given note


  int m_note;
  int m_pitch_bend;
  int m_timer;
};




// Create Oscillator instances
Oscillator OSC1(1);



void setup() {


  Serial.begin(9600);



  // Connect the callbacks for MIDI input - these will be called on certain MIDI events:
  // 
  MIDI.setHandleNoteOn(handleNoteOn);  
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandlePitchBend(handlePitchBend);

  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

}





void loop() {

  // Call MIDI.read the fastest you can for real-time performance.
  MIDI.read();

}

//A Note-on event has been received
void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  // Pass the note onto the oscillators
  OSC1.setNote(int(pitch));

}

// A Note-off event has been received
void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  // Do something when the note is released.
  // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.

}

// A pitch-bend event has been received:
void handlePitchBend( byte channel, int bend )
{
  // Pass the pitchbend onto the oscillators
  OSC1.setPitchBend(bend);

}












