#include <Arduino.h>
#include "ESP8266WiFi.h"
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <i2s.h>
#include <i2s_reg.h>
#include <pgmspace.h>
#include <Ticker.h>
#include <Drum.h>


extern "C" {
#include "user_interface.h"
}

// Forward declaration
void OnAppleMidiNoteOn(byte channel, byte note, byte velocity);
void ICACHE_RAM_ATTR onTimerISR(void);
uint16_t SYNTH909(void);

void setup() {
  WiFi.forceSleepBegin();
  delay(1);

  system_update_cpu_freq(160);

  i2s_begin();
  i2s_set_rate(44100);

  timer1_attachInterrupt(onTimerISR); //Attach our sampling ISR
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(2000); //Service at 2mS intervall
}

void loop() {
 for(int i=35;i<=51;i++)
 {
  OnAppleMidiNoteOn(10, i, 0);
  /*
  OnAppleMidiNoteOn(10, (((i+1)>51)?51-1:i+1), 0);
  OnAppleMidiNoteOn(10, (((i+3)>51)?51-3:i+3), 0);
  OnAppleMidiNoteOn(10, (((i+5)>51)?51-5:i+5), 0);
  delay(random(150, 300));
  OnAppleMidiNoteOn(10, i, 0);
  OnAppleMidiNoteOn(10, (((i+6)>51)?51-6:i+6), 0);
  OnAppleMidiNoteOn(10, (((i+9)>51)?51-9:i+9), 0);
  OnAppleMidiNoteOn(10, (((i+12)>51)?51-12:i+12), 0);
  delay(random(300, 400));
  OnAppleMidiNoteOn(10, i, 0);
  OnAppleMidiNoteOn(10, (((i+2)>51)?51-2:i+2), 0);
  OnAppleMidiNoteOn(10, (((i+4)>51)?51-4:i+4), 0);
  OnAppleMidiNoteOn(10, (((i+8)>51)?51-8:i+8), 0);
  delay(random(400, 500));
  */
  delay(1000);
 }
}

void ICACHE_RAM_ATTR onTimerISR(){
  while (!(i2s_is_full())) { //Don't block the ISR
    DAC=SYNTH909();
    //----------------- Pulse Density Modulated 16-bit I2S DAC --------------------
     for (uint8_t i=0;i<32;i++) {
      i2sACC=i2sACC<<1;
      if(DAC >= err) {
        i2sACC|=1;
        err += 0xFFFF-DAC;
      }
      else{
        err -= DAC;
      }
     }
     bool flag=i2s_write_sample(i2sACC);
    //-----------------------------------------------------------------------
  }
  timer1_write(2000);//Next in 2mS
}

void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
  if (channel==10) {
    if(note==35) BD16CNT=0;   //Bass Drum MIDI-35
    if(note==36) BD16CNT=0;   //Bass Drum MIDI-36
    if(note==37) RS16CNT=0;   //Rim Shot MIDI-37
    if(note==38) SD16CNT=0;   //Snare Drum MIDI-38
    if(note==39) CP16CNT=0;   //Hand Clap MIDI-39
    if(note==40) SD16CNT=0;   //Snare Drum MIDI-40
    if(note==41) LT16CNT=0;   //Low Tom MIDI-41
    if(note==42) HH16CNT=0;   //Closed Hat MIDI-42
    if(note==43) LT16CNT=0;   //Low Tom MIDI-43
    if(note==44) HH16CNT=0;   //Closed Hat MIDI-44
    if(note==45) MT16CNT=0;   //Mid Tom MIDI-45
    if(note==46) OH16CNT=0;   //Open Hat MIDI-46
    if(note==47) MT16CNT=0;   //Mid Tom MIDI-47
    if(note==48) HT16CNT=0;   //Hi Tom MIDI-48
    if(note==49) CR16CNT=0;   //Crash Cymbal MIDI-49
    if(note==50) HT16CNT=0;   //Hi Tom MIDI-50
    if(note==51) RD16CNT=0;   //Ride Cymbal MIDI-51
  }
}

uint16_t SYNTH909() {
  int32_t DRUMTOTAL=0;
  if (BD16CNT<BD16LEN) DRUMTOTAL+=(pgm_read_word_near(BD16 + BD16CNT++)^32768)-32768;
  if (CP16CNT<CP16LEN) DRUMTOTAL+=(pgm_read_word_near(CP16 + CP16CNT++)^32768)-32768;
  if (CR16CNT<CR16LEN) DRUMTOTAL+=(pgm_read_word_near(CR16 + CR16CNT++)^32768)-32768;
  if (HH16CNT<HH16LEN) DRUMTOTAL+=(pgm_read_word_near(HH16 + HH16CNT++)^32768)-32768;
  if (HT16CNT<HT16LEN) DRUMTOTAL+=(pgm_read_word_near(HT16 + HT16CNT++)^32768)-32768;
  if (LT16CNT<LT16LEN) DRUMTOTAL+=(pgm_read_word_near(LT16 + LT16CNT++)^32768)-32768;
  if (MT16CNT<MT16LEN) DRUMTOTAL+=(pgm_read_word_near(MT16 + MT16CNT++)^32768)-32768;
  if (OH16CNT<OH16LEN) DRUMTOTAL+=(pgm_read_word_near(OH16 + OH16CNT++)^32768)-32768;
  if (RD16CNT<RD16LEN) DRUMTOTAL+=(pgm_read_word_near(RD16 + RD16CNT++)^32768)-32768;
  if (RS16CNT<RS16LEN) DRUMTOTAL+=(pgm_read_word_near(RS16 + RS16CNT++)^32768)-32768;
  if (SD16CNT<SD16LEN) DRUMTOTAL+=(pgm_read_word_near(SD16 + SD16CNT++)^32768)-32768;
  if  (DRUMTOTAL>32767) DRUMTOTAL=32767;
  if  (DRUMTOTAL<-32767) DRUMTOTAL=-32767;
  DRUMTOTAL+=32768;
  return DRUMTOTAL;
}
