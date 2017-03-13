// Used Board 
// Arduino Mega ADK

// Lidar Information
// http://xv11hacking.wikispaces.com/LIDAR+Sensor

// Header File for RS232C Serial Communication
#include <SoftwareSerial.h>

const int MotorPWM = 4; // Lidar Motor Speed Control (PWM)
byte data = 0; // Incoming Serial Byte
byte error = -1;
short data0 = 0;
short data1 = 0;

short degree = 0;
short distance = 0;

void setup() {
  pinMode(MotorPWM, OUTPUT); // DDR
  analogWrite(MotorPWM, 255); // Motor Speed MAX

  Serial.begin(115200);  // USB Seriaㅣ
  Serial1.begin(115200);  // XV-11 LDS data
}

/*
 *  XV-11 Serial Data Format
 *
 *  Motor 구동과 함께 통신 시작.
 *  1 패킷 당 길이는 22 bytes.
 *  전체 1 회전으로 90 패킷이 산출되며 각각 4 회의 연속 판독을 포함(1패킷 당 4도의 연속 판독!!!!!)
 *  1980 바이트에 총 360 개의 수치 (1 도당 1)
 *
 *
 *  <start> <index> <speed_L> <speed_H> [Data 0] [Data 1] [Data 2] [Data 3] <checksum_L> <checksum_H>
 *
 *    <start> byte 는 시작 바이트로 항상 0xFA
 *    <index> 는 0xA0 (패킷 0, 판독 0-3)에서 0xF9 (패킷 89, 판독 값 356-359)까지가는 90 패킷의 색인 바이트.
 *    <speed_L> <speed_H> 는 속도값
 *
 *    [Data 0] [Data 1] [Data 2] [Data 3]는 [데이터 0] ~ [데이터 3]은 4 개의 판독 값
 *      바이트 0 : <거리 7 : 0>`
`*      바이트 1 : <invalid data "flag> <"strength warning "flag> <distance 13 : 8>`
 *`     바이트 2 : <신호 강도 7 : 0>`
`*      바이트 3 : <신호 강도 15 : 8>
 *      거리 정보는 mm 단위이며 14 비트로 코딩. 최소 거리는 약 15cm이고, 최대 거리는 약 6m (범위가 벗어나면 Error Code 발생)
 *      바이트 1의 비트 7이 설정되면 거리를 계산할 수 없음을 나타냅니다. 이 비트가 설정되면 바이트 0에 오류 코드가 포함
 *      오류 코드의 예는 0x02, 0x03, 0x21, 0x25, 0x35 또는 0x50
 *
 *    바이트 1의 비트 6은 레이저 강도가 이 거리에서 예상되는 것보다 크게 떨어지는 경우의 경고
 *    재료의 반사율이 낮거나 (검은 색 재료 ...) 도트가 예상되는 크기 나 모양(다공성 재료, 투명 패브릭, 격자, 대상의 가장자리 ...)을 가지지 않을 때 발생할 수 있다.
 *    바이트 2와 3은 강도 표시의 LSB와 MSB입니다. 이 값은 retroreflector를 마주 할 때 매우 높아질 수 있습니다.
 *
 *    checksum은 패킷의 2 바이트 체크섬입니다.
 */

void loop() {
  // if we get a valid byte from LDS, read it and send it to USB-serial
  if (Serial1.available() > 0) {
    data = Serial1.read(); // Get Incoming Serial byte:

    if(data == 0xFA) {
      Serial.print("\n");
      Serial.print(data, HEX);
      Serial.print(" ");

      for(int i=1; i <= 22; i++) {
        data = Serial1.read();

        // Index byte
        // 인덱스 당 각도 4도 연속 측정 후 평균거리 산출
        if(i == 1) {
          Serial.print(data, HEX);
          Serial.print(" Deg: ");
          degree = (short)(data - 0xA0) << 2; // MCU는 비트 이동 연산이 곱하기/나누기 연산보다 빠르다.
          Serial.print(degree );
          Serial.print(" ");
        }

        if(i == 4) {
          data0 = (short) data;
          delay(1);
        }

        if(i == 5) {
          // Error bit Masking
          if(data >= 0x80) {
            data1 = error;
          }
          else {
            if(data >= 0x40) {
              data = data - 0x40;
            }

            data1 = ((short) data) << 8;
          }

          if(data1 != error) {
            distance = data0 + data1;

            Serial.print(" Distance: ");
            Serial.print(distance);
            Serial.print(" mm");
          } else {
            Serial.print(" Distance: ");
            Serial.print("ERROR");
          }
        }
      }
    }
  }
}
