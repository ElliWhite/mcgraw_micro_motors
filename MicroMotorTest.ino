
int LED_BUILTIN = 2;
int encoder_a = 4;
int encoder_b = 16;
volatile int encoder_a_state = 0;
volatile int encoder_a_prev_state = 0;
volatile int encoder_b_state = 0;
volatile int encoder_b_prev_state = 0;
bool encoder_a_changed = false;
bool encoder_b_changed = false;
bool is_forwards = false;
bool go_forwards = true;

long previous_millis = 0;
long interval = 2000;  // interval at which to change motor direction (milliseconds)

int motor_control_a = 22;
int motor_control_b = 23;



const int PWM_CHANNEL_A = 0;    // ESP32 has 16 channels which can generate 16 independent waveforms
const int PWM_CHANNEL_B = 0;    // ESP32 has 16 channels which can generate 16 independent waveforms
const int PWM_FREQ = 500;     // Recall that Arduino Uno is ~490 Hz. Official ESP32 example uses 5,000Hz
const int PWM_RESOLUTION = 8; // We'll use same resolution as Uno (8 bits, 0-255) but ESP32 can go up to 16 bits

// The max duty cycle value based on PWM resolution (will be 255 if resolution is 8 bits)
const int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1); 

volatile long pulse_count_a = 0;
volatile long pulse_count_b = 0;
long previous_pulse_count_a = 0;
long previous_pulse_count_b = 0;
int revolutions = 0;
// 7 CPR
// 210 reduction ratio
// 2940 pulses-per-rev (pulse = STATE CHANGE) [7*210*2]


void IRAM_ATTR isrEncoderAChange(){
  encoder_a_state = digitalRead(encoder_a);
  pulse_count_a++;
}



void IRAM_ATTR isrEncoderBChange(){
  encoder_b_state = digitalRead(encoder_b);
  pulse_count_b++;
}



void motorChangeDirection(){
  if(go_forwards){
    go_forwards = false;
    motorBackwards(50);
  } else {
    go_forwards = true;
    motorForwards(50);
  }
}


void motorTest(){
  // go forwards
  ledcDetachPin(motor_control_b);
  ledcAttachPin(motor_control_a, PWM_CHANNEL_A);

  // fade up PWM on given channel
  for(int dutyCycle = 0; dutyCycle <= MAX_DUTY_CYCLE; dutyCycle++){   
    ledcWrite(PWM_CHANNEL_A, dutyCycle);
    Serial.print(" Duty cycle set to ");
    Serial.println(dutyCycle);
    delay(100);
  }

  // turn off channel A
  ledcWrite(PWM_CHANNEL_A, 0);

  // go backwards
  ledcDetachPin(motor_control_a);
  ledcAttachPin(motor_control_b, PWM_CHANNEL_B);
  
  // fade up PWM on given channel
  for(int dutyCycle = 0; dutyCycle <= MAX_DUTY_CYCLE; dutyCycle++){   
    ledcWrite(PWM_CHANNEL_B, dutyCycle);
    Serial.print(" Duty cycle set to ");
    Serial.println(dutyCycle);
    delay(100);
  }

  // turn off channel B
  ledcWrite(PWM_CHANNEL_B, 0);
}



void motorForwards(uint8_t dutyCycle){
  // go forwards
  ledcDetachPin(motor_control_b);
  ledcAttachPin(motor_control_a, PWM_CHANNEL_A);
  ledcWrite(PWM_CHANNEL_A, dutyCycle);
  
}



void motorBackwards(uint8_t dutyCycle){
  // go backwards
  ledcDetachPin(motor_control_a);
  ledcAttachPin(motor_control_b, PWM_CHANNEL_B);
  ledcWrite(PWM_CHANNEL_B, dutyCycle);
}



void motorStop(){
  if(go_forwards){
    ledcWrite(PWM_CHANNEL_A, 0);
  } else {
    ledcWrite(PWM_CHANNEL_B, 0);
  }
}



void setup() {

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(encoder_a, INPUT);
  pinMode(encoder_b, INPUT);
  attachInterrupt(digitalPinToInterrupt(encoder_a), isrEncoderAChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder_b), isrEncoderBChange, CHANGE);

  // Sets up a channel (0-15), a PWM duty cycle frequency, and a PWM resolution (1 - 16 bits) 
  // ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits);
  ledcSetup(PWM_CHANNEL_A, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_B, PWM_FREQ, PWM_RESOLUTION);

  // ledcAttachPin(uint8_t pin, uint8_t channel);
  ledcAttachPin(motor_control_a, PWM_CHANNEL_A);
  ledcAttachPin(motor_control_b, PWM_CHANNEL_B);
  
  //Set both channels to 0
  ledcWrite(PWM_CHANNEL_A, 0);
  ledcWrite(PWM_CHANNEL_B, 0);

  // set initial forwards to 50/255 duty cycle
  motorForwards(50);

}



void loop() {

  unsigned long current_millis = millis();

 /* if(current_millis - previous_millis > interval) {
    motorStop(); 
    noInterrupts();
    previous_millis = current_millis;    
    Serial.print("A Pulse Count ");
    Serial.print(pulse_count_a);
    Serial.print("     B Pulse Count ");
    Serial.println(pulse_count_b);
    previous_pulse_count_a = pulse_count_a;
    previous_pulse_count_b = pulse_count_b;
    pulse_count_a = 0;
    pulse_count_b = 0;
    delay(1000);
    interrupts();
    motorChangeDirection();
  }
*/

  if(pulse_count_a == 2940 || pulse_count_b == 2940){
    revolutions+=1;
    Serial.print("A Pulse Count ");
    Serial.print(pulse_count_a);
    Serial.print("     B Pulse Count ");
    Serial.print(pulse_count_b);
    Serial.print("     Revolutions ");
    Serial.println(revolutions);
    previous_pulse_count_a = pulse_count_a;
    previous_pulse_count_b = pulse_count_b;
    pulse_count_a = 0;
    pulse_count_b = 0;
  }
  
  
  if(encoder_a_prev_state != encoder_a_state){
    noInterrupts();
    if(encoder_a_state != encoder_b_state){
      digitalWrite(LED_BUILTIN, 1);
      is_forwards = true;
    } else {
      digitalWrite(LED_BUILTIN, 0);
      is_forwards = false;
    }

  }
 
  if(encoder_b_prev_state != encoder_b_state){
    noInterrupts();
    if(encoder_b_state == encoder_a_state){
      digitalWrite(LED_BUILTIN, 1);
      is_forwards = true;
    } else {
      digitalWrite(LED_BUILTIN, 0);
      is_forwards = false;
    }
  }
  
  encoder_a_prev_state = encoder_a_state;
  encoder_b_prev_state = encoder_b_state;
  interrupts();
 
}
