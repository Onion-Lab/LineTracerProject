#include <SoftwareSerial.h>
#include <MsTimer2.h>

// 블루투스 시리얼 설정 
int BLUETOOTH_TX=2;
int BLUETOOTH_RX=3;

int LEFT_LINE = A5; // 왼쪽 라인트레이서 센서는 A5 핀에 연결
int CENTER_LINE = A4; // 가운데 라인트레이서 센서는 A4 핀에 연결
int RIGHT_LINE = A3; // 오른쪽 라인트레이서 센서는 A3 핀에 연결

int RIGHT_MOROR_ENABLE_PWM = 5;      // 오른쪽 모터의 Enable & PWM
int LEFT_MOROR_ENABLE_PWM = 6;       // 왼쪽 모터의 Enable & PWM
int RIGHT_MOTOR_DIR_1 = 8;      // 오른쪽 모터 제어선 IN1
int RIGHT_MOTOR_DIR_2 = 9;      // 오른쪽 모터 제어선 IN2
int LEFT_MOTOR_DIR_1 = 10;      // 왼쪽 모터 제어선 IN3
int LEFT_MOROR_DIR_2 = 11;      // 왼쪽 모터 제어선 IN4

// PWM값
int MOTOR_SPEED_PWM = 110;
int MOTOR_TURN_SPEED_PWM = 170;

// 회전하는 시간. 배터리 전압에 따른 수정 필요
int TURN_MS = 700;

// 차량의 상태를 저장
int CAR_STATUS = 0;

int L, R, C;

// 블루투스 통신을 위한 객체선언
SoftwareSerial bluetoothSerial(BLUETOOTH_TX, BLUETOOTH_RX); 


// 우회전 : R후진 L 전진
//R1 H
//R2 L
//L1 H 
//L2 L

// 직진 : R전진 L 전진
//R1 L
//R2 H
//L1 H 
//L2 L

// 좌회전 : R전진 L 후진
//R1 L
//R2 H
//L1 L
//L2 H


void drive(int delayMs)
{
    digitalWrite(RIGHT_MOTOR_DIR_1, LOW);
	digitalWrite(RIGHT_MOTOR_DIR_2, HIGH);
	digitalWrite(LEFT_MOTOR_DIR_1, HIGH);
	digitalWrite(LEFT_MOROR_DIR_2, LOW);

	analogWrite(RIGHT_MOROR_ENABLE_PWM, MOTOR_SPEED_PWM);  // 우측 모터 속도값
	analogWrite(LEFT_MOROR_ENABLE_PWM, MOTOR_SPEED_PWM);   // 좌측 모터 속도값
	delay(delayMs);
}

void reverse(int delayMs)
{
    digitalWrite(RIGHT_MOTOR_DIR_1, HIGH);
	digitalWrite(RIGHT_MOTOR_DIR_2, LOW);
	digitalWrite(LEFT_MOTOR_DIR_1, LOW);
	digitalWrite(LEFT_MOROR_DIR_2, HIGH);

    analogWrite(RIGHT_MOROR_ENABLE_PWM, MOTOR_SPEED_PWM);  // 우측 모터 속도값
	analogWrite(LEFT_MOROR_ENABLE_PWM, MOTOR_SPEED_PWM);   // 좌측 모터 속도값
	delay(delayMs);
}

void parking(int delayMs)
{
	analogWrite(RIGHT_MOROR_ENABLE_PWM, 0);  // 우측 모터 속도값
	analogWrite(LEFT_MOROR_ENABLE_PWM, 0);   // 좌측 모터 속도값
	delay(delayMs);
}

void rightTurn(int delayMs)
{
    digitalWrite(RIGHT_MOTOR_DIR_1, HIGH);
	digitalWrite(RIGHT_MOTOR_DIR_2, LOW);
	digitalWrite(LEFT_MOTOR_DIR_1, HIGH);
	digitalWrite(LEFT_MOROR_DIR_2, LOW);

    analogWrite(RIGHT_MOROR_ENABLE_PWM, MOTOR_TURN_SPEED_PWM);  // 우측 모터 속도값
	analogWrite(LEFT_MOROR_ENABLE_PWM, MOTOR_TURN_SPEED_PWM);   // 좌측 모터 속도값
	delay(delayMs);
}

void leftTurn(int delayMs)
{
    digitalWrite(RIGHT_MOTOR_DIR_1, LOW);
	digitalWrite(RIGHT_MOTOR_DIR_2, HIGH);
	digitalWrite(LEFT_MOTOR_DIR_1, LOW);
	digitalWrite(LEFT_MOROR_DIR_2, HIGH);

    analogWrite(RIGHT_MOROR_ENABLE_PWM, MOTOR_TURN_SPEED_PWM);  // 우측 모터 속도값
	analogWrite(LEFT_MOROR_ENABLE_PWM, MOTOR_TURN_SPEED_PWM);   // 좌측 모터 속도값   
	delay(delayMs); 
}



// 아두이노가 부팅되고 최초로 호출되는 함수
void setup() 
{
	Serial.begin(9600);   // 시리얼모니터
	bluetoothSerial.begin(9600); // 블루투스 시리얼
	Serial.println("Arduino Start!\n");
	pinMode(RIGHT_MOROR_ENABLE_PWM, OUTPUT);
	pinMode(LEFT_MOROR_ENABLE_PWM, OUTPUT);
	pinMode(RIGHT_MOTOR_DIR_1, OUTPUT);
	pinMode(RIGHT_MOTOR_DIR_2, OUTPUT);
	pinMode(LEFT_MOTOR_DIR_1, OUTPUT);
	pinMode(LEFT_MOROR_DIR_2, OUTPUT);
	pinMode(LEFT_LINE, INPUT);
	pinMode(CENTER_LINE, INPUT);
	pinMode(RIGHT_LINE, INPUT);
	Serial.println("GPIO Initialized!\n");

	MsTimer2::set(100, interruptFunction);
	MsTimer2::start();
}


void leftTurnCommandFunction()
{
	int timeStack = 0;
	drive(1);
	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}

	// 정지선에서 3초 대기 후 좌회전
	parking(2000);
	leftTurn(TURN_MS);
	while(!(L == HIGH && C == HIGH && R == HIGH))
	{
		delay(10);
	}
	parking(100);

	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}
	parking(2000);

	leftTurn(TURN_MS*2);
	while(!(L == HIGH && C == HIGH && R == HIGH))
	{
		delay(10);
	}
	parking(2000);

	drive(1);
	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}
	parking(2000);
	drive(500);
	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}

	parking(2000);
	rightTurn(TURN_MS);
	while(!(L == HIGH && C == HIGH && R == HIGH))
	{
		delay(10);
	}
	parking(100);

	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}
	parking(1000);
	leftTurn(TURN_MS*2);
	while(!(L == HIGH && C == HIGH && R == HIGH))
	{
		delay(10);
	}
	parking(2000);
}

void driveCommandFunction()
{
	int timeStack = 0;
	drive(1);
	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}

	// 정지선에서 3초 대기 후 직진
	parking(2000);
	drive(500);


	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}
	// 두번째 정지선을 만나면 2초 후에 360도 회전
	parking(2000);

	leftTurn(TURN_MS*2);
	while(!(L == HIGH && C == HIGH && R == HIGH))
	{
		delay(10);
	}
	parking(2000);

	drive(1);
	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}
	parking(2000);
	drive(500);

	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}
	parking(1000);
	leftTurn(TURN_MS*2);
	while(!(L == HIGH && C == HIGH && R == HIGH))
	{
		delay(10);
	}
	parking(2000);
}

void rightTurnCommandFunction()
{
	int timeStack = 0;
	drive(1);
	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}

	// 정지선에서 3초 대기 후 좌회전
	parking(2000);
	rightTurn(TURN_MS);
	while(!(L == HIGH && C == HIGH && R == HIGH))
	{
		delay(10);
	}
	parking(100);

	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}
	parking(2000);

	leftTurn(TURN_MS*2);
	while(!(L == HIGH && C == HIGH && R == HIGH))
	{
		delay(10);
	}
	parking(2000);

	drive(1);
	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}
	parking(2000);
	drive(500);
	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}

	parking(2000);
	leftTurn(TURN_MS);
	while(!(L == HIGH && C == HIGH && R == HIGH))
	{
		delay(10);
	}
	parking(100);

	while(1) 
	{
		L = digitalRead(LEFT_LINE);
		C = digitalRead(CENTER_LINE);
		R = digitalRead(RIGHT_LINE);
		if(L == HIGH && C == LOW && R == LOW) // 왼쪽으로 치우쳐진 경우
		{
			Serial.println("Right Turn!!");
			rightTurn(0); // 우회전
		}
		else if(L == LOW && C == LOW && R == HIGH) // 오른쪽으로 치우쳐진 경우
		{
			Serial.println("Left Turn!!");
			leftTurn(0); // 좌회전
		}
		else if(L == LOW && C == LOW && R == LOW) // 아무것도 검출되지 않는 경우
		{
			Serial.println("GOGO!!");
			drive(0); // 전진
		}
		else if(L == HIGH && C == HIGH && R == HIGH) // 정지선까지 전진 후 빠져나옴
		{
			timeStack++;
			if(timeStack > 2)
			{
				break;
			}
		}
	}
	parking(1000);
	leftTurn(TURN_MS*2);
	while(!(L == HIGH && C == HIGH && R == HIGH))
	{
		delay(10);
	}
	parking(2000);
}


// 메인루프
void loop()
{
	int L, C, R;
	int timeStack = 0;
	switch(CAR_STATUS)
	{
		case 1:
			leftTurnCommandFunction();
			CAR_STATUS = 0;
			break;
		case 2:
			driveCommandFunction();
			CAR_STATUS = 0;
			break;
		case 3:
			rightTurnCommandFunction();
			CAR_STATUS = 0;
			break;
		case 4:
			break;
		default:
			break;
	}
}

void interruptFunction()
{
	byte data;
	if (bluetoothSerial.available())
	{
		data = bluetoothSerial.read();
		if(data >0)
		{
			Serial.write(data);
		}
		
		switch(data)
		{
			case '0': // Stop
			CAR_STATUS = 0;
			Serial.println("Get Stop Command....");
			break;
	
			case '1': // Sequence 1
			CAR_STATUS = 1;
			Serial.println("Get Sequence 1 Command....");
			break;
	
			case '2': // Sequence 2
			CAR_STATUS = 2;
			Serial.println("Get Sequence 2 Command....");
			break;
	
			case '3': // Sequence 3
			CAR_STATUS = 3;
			Serial.println("Get Sequence 3 Command....");
			break;

			case '4': // Sequence 4
			CAR_STATUS = 4;
			Serial.println("Get Sequence 4 Command....");
			break;

			default:
			break;
		}
	}
}
