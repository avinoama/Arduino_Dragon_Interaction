#include <SoftwareSerial.h>
#include <Arduino.h>

//should connect to TX of the Serial MP3 Player module
#define ARDUINO_RX 5
//connect to RX of the module
#define ARDUINO_TX 6

#define CMD_SEL_DEV 0X09
  #define DEV_TF 0X02

#define CMD_SET_DAC 0X1A
  #define DAC_ON  0X00
  #define DAC_OFF 0X01

#define CMD_VOLUME_UP 0X04
#define CMD_VOLUME_DOWN 0X05
#define CMD_SET_VOLUME 0X06

#define CMD_PLAY 0X0D
#define CMD_PLAY_W_INDEX 0X08
#define CMD_PLAY_W_VOL 0X22
#define CMD_PLAY_FOLDER_FILE 0X0F
#define CMD_STOP_PLAY 0X16
#define CMD_PAUSE 0X0E
#define CMD_SINGLE_CYCLE_PLAY 0X08
#define CMD_SINGLE_CYCLE 0X19
  #define SINGLE_CYCLE_ON 0X00
  #define SINGLE_CYCLE_OFF 0X01

#define CMD_NEXT_SONG 0X01
#define CMD_PREV_SONG 0X02

#define CMD_SLEEP_MODE 0X0A
#define CMD_WAKE_UP 0X0B
#define CMD_RESET 0X0C

#define CMD_FOLDER_CYCLE 0X17
#define CMD_SHUFFLE_PLAY 0X18

#define CMD_GROUP_DISPLAY 0X21

SoftwareSerial mp3_serial(ARDUINO_RX, ARDUINO_TX);

static int8_t Send_buf[8] = {
  0} 
;

void serialmp3_sendCommand(int8_t command, int16_t dat) {
  delay(20);
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0xff; //version
  Send_buf[2] = 0x06; //the number of bytes of the command without starting byte and ending byte
  Send_buf[3] = command; //
  Send_buf[4] = 0x01; //0x00 = no feedback, 0x01 = feedback
  Send_buf[5] = (int8_t)(dat >> 8);//datah
  Send_buf[6] = (int8_t)(dat); //datal
  Send_buf[7] = 0xef; //ending byte
  for(uint8_t i=0; i<8; i++) {
    mp3_serial.write(Send_buf[i]) ;
  }
}

void serialmp3_init() {
  mp3_serial.begin(9600);
  //Wait chip initialization is complete
  delay(500);
  //wait for 200ms
  serialmp3_sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card  
  delay(200);
}

void serialmp3_set_vol(byte vol) {
  serialmp3_sendCommand(CMD_SET_VOLUME, vol);
}

void serialmp3_play(byte folder, byte track) {
  //play the first song with volume 15 class: 0X0F01
  // serialmp3_sendCommand(CMD_PLAY_W_VOL, int(volume<< 8) | track);
  serialmp3_sendCommand(CMD_PLAY_FOLDER_FILE, folder<<8 | track); //>>>
}

void serialmp3_play(byte track) {
  serialmp3_play(1, track);
}

void serialmp3_stop() {
  serialmp3_sendCommand(CMD_STOP_PLAY, 0);
}

void serialmp3_pause() {
  serialmp3_sendCommand(CMD_PAUSE, 0);
}

void serialmp3_resume() {
  serialmp3_sendCommand(CMD_PLAY, 0);
}

void serialmp3_next() {
  serialmp3_sendCommand(CMD_NEXT_SONG, 0);
}

void serialmp3_prev() {
  serialmp3_sendCommand(CMD_PREV_SONG, 0);
}


