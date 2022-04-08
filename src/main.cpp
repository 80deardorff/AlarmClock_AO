#include <Arduino.h>
#include "qpn.h"
#include "ClockAlarm_SM.h"
#include "button_SM.h"
#include "lcd.h"
#include "main.h"

Q_DEFINE_THIS_FILE;

bool flag_report_button_press = true;

static void sys_tick_init(void);
static void display_init(void);
static void buzzer_init(void);
static void attach_button_interrupts();

static QEvt ClockAlarmQueue[8];
static QEvt ButtonQueue[8];

QActiveCB const QF_active[] = {
  {(QActive*) 0, (QEvt*) 0, 0},
  {(QActive*) AO_ClockAlarm, (QEvt*) ClockAlarmQueue, Q_DIM(ClockAlarmQueue)},
  {(QActive*) AO_Button, (QEvt*) ButtonQueue, Q_DIM(ButtonQueue)}
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  display_init();
  attach_button_interrupts();
  buzzer_init();
  Clock_Alarm_ctor();
  Button_ctor();
  QF_init(Q_DIM(QF_active));
}

void loop() {
  // put your main code here, to run repeatedly:
  QF_run();
}

void SET_handler() {
  QF_INT_DISABLE();
  if(flag_report_button_press) {
    flag_report_button_press = false;
    QActive_armX(AO_Button, 0, MS_TO_TICKS(50), 0);
  }
  QF_INT_ENABLE();
}

void OK_handler() {
  QF_INT_DISABLE();
  if(flag_report_button_press) {
    flag_report_button_press = false;
    QActive_armX(AO_Button, 0, MS_TO_TICKS(50), 0);
  }
  QF_INT_ENABLE();
}

static void attach_button_interrupts() {
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON1), SET_handler, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON2), OK_handler, RISING);
}

static void sys_tick_init(void) {
  TCCR1A = 0;           // CTC Mode
  TCCR1B = TCCR1B_PRESCALER_1;   // Prescaler = 1, CTC Mode
  TIMSK1 |= B00000010;  // Interrupt enable for OCR1A compare match
  OCR1A = TIMER1_OC_MATCH_VALUE; // OC match value for CONFIG_TICKS_PER_SECOND
}

static void display_init(void) {
  pinMode(PIN_LCD_BACKLIGHT, OUTPUT);
  lcd_backlight_on();
  lcd_begin(16,2);
  lcd_clear();
  lcd_move_cursor_L_to_R();
  lcd_set_cursor(0,0);
  lcd_no_auto_scroll();
  lcd_cursor_off();
}

static void buzzer_init(void) {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
}

void QF_onStartup(void) {
  sys_tick_init();
}

void QV_onIdle(void) {
  QV_CPU_SLEEP();
}

Q_NORETURN Q_onAssert	(	char_t const Q_ROM *const 	module,int_t const 	location ){

  Serial.println("Assertion failure!!");
  Serial.println((String)module);
  Serial.println(location);
  while(1);
}
