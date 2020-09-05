#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

// Debounce time in us
#define DEBOUNCE_TIME 1000
// Number of LEDs/buttons
#define LED_COUNT 4

// Check if buttonis pressed, with debouncing
// port - the port the button is connected to
// pin - the pin the button is connected to
uint8_t is_pressed(uint8_t port, uint8_t pin)
{
  // 0 = pressed, 1 = not pressed
  if ((port & (1 << pin)) == 0)
  {
    _delay_us(DEBOUNCE_TIME);
    if ((port & (1 << pin)) == 0)
    {
      return 1;
    }
  }
  return 0;
}

// set an output pin high
void enable_output(volatile uint8_t * port, uint8_t pin)
{
  *port |= (1 << pin);
}

// set an output pin low
void disable_output(volatile uint8_t * port, uint8_t pin)
{
  *port &= ~(1 << pin);
}

void init()
{
  // Enable PB0 for output
  DDRB |= (1 << PB0);
  // Enable PB1 for output
  DDRB |= (1 << PB1);
  // Enable PB2 for output
  DDRB |= (1 << PB2);
  // Enable PB3 for output
  DDRB |= (1 << PB3);
  // Enable PD6 for sound output
  DDRD |= (1 << PD6);
  // Enable the pull-up resistor on PD2
  PORTD |= (1 << PD2);
  // Enable the pull-up resistor on PD3
  PORTD |= (1 << PD3);
  // Enable the pull-up resistor on PD4
  PORTD |= (1 << PD4);
  // Enable the pull-up resistor on PD5
  PORTD |= (1 << PD5);
}

void disable_led(uint8_t currentLed)
{
  switch (currentLed)
  {
  case 0:
    disable_output(&PORTB, PB0);
    break;
  case 1:
    disable_output(&PORTB, PB1);
    break;
  case 2:
    disable_output(&PORTB, PB2);
    break;
  case 3:
    disable_output(&PORTB, PB3);
    break;
  } 
}

// From "Make: AVR Programming" by Elliot Williams
void play_sound(uint16_t period, uint16_t duration)
{
  for (uint16_t elapsed = 0; elapsed < duration; elapsed += period)
  {
    for (uint16_t i = 0; i < period; i++)
    {
      _delay_us(1);
    }
    PORTD ^= (1 << PD6);
  }
}

void play_sound_for_led(uint8_t currentLed)
{
  switch (currentLed)
  {
  case 0:
    play_sound(929, 0xF000 / 2);
    break;
  case 1:
    play_sound(827, 0xF000 / 2);
    break;
  case 2:
    play_sound(781, 0xF000 / 2);
    break;
  case 3:
    play_sound(696, 0xF000 / 2);
    break;
  };
}

void enable_led(uint8_t currentLed)
{
  switch (currentLed)
  {
  case 0:
    enable_output(&PORTB, PB0);
    break;
  case 1:
    enable_output(&PORTB, PB1);
    break;
  case 2:
    enable_output(&PORTB, PB2);
    break;
  case 3:
    enable_output(&PORTB, PB3);
    break;
  };
}

int8_t is_button_pressed(uint8_t currentLed)
{
  if (is_pressed(PIND, PD2))
  {
    if (currentLed == 0)
      return 1;
     else
      return -1;
  }
  else if (is_pressed(PIND, PD3))
  {
     if (currentLed == 1)
      return 1;
     else
      return -1;
  }
  else if (is_pressed(PIND, PD4))
  {
    if (currentLed == 2)
      return 1;
     else
      return -1;
  }
  else if (is_pressed(PIND, PD5))
  {
    if (currentLed == 3)
      return 1;
     else
      return -1;
  }

  return 0;
}

uint8_t NewRandom(uint8_t randMax, uint8_t oldRandom, uint8_t attempts)
{
  uint8_t newRandom = 0;
  for (uint8_t i = 0; i < attempts; ++i)
  {
    newRandom = rand() % randMax;
    if (newRandom != oldRandom)
      break;
  }

  return newRandom;
}

void blink_sequence(uint8_t currentLed, uint16_t blinkDelay)
{
  disable_led(currentLed);
  _delay_ms(blinkDelay);
  enable_led(currentLed);
}

enum State
{
  Init,
  NewLed,
  HandlePress,
  WrongLed
};

int main(void)
{
  init();

  enum State state = Init;
  // Light up the first LED
  enable_output(&PORTB, PB0);
  // Used as random seed
  uint16_t cycleCount = 0;
  // Current active LED
  uint8_t currentLed = 0;

  while (1)
  { 
    switch (state)
    {
      case Init:
      {
        ++cycleCount;
        // Check if the first button is pressed
        if (is_button_pressed(currentLed))
        {
          srand(cycleCount);
          state = NewLed;
        }
      }
      break;
      case NewLed:
      {
        disable_led(currentLed);
        _delay_us(1000);
        currentLed = NewRandom(LED_COUNT, currentLed, 10);
        enable_led(currentLed);
        play_sound_for_led(currentLed);
        state = HandlePress;
      }
      break;
      case HandlePress:
      {
        int8_t buttonPressState = is_button_pressed(currentLed);
        if (buttonPressState == 1)
        {
          state = NewLed;
        }
        else if (buttonPressState == -1)
        {
          state = WrongLed;
        }
      }
      break;
      case WrongLed:
        play_sound(1043, 0xF000);
        
        blink_sequence(currentLed, 100);
        _delay_ms(100);
        blink_sequence(currentLed, 100);

        state = HandlePress;
      break;
    };
  }
  
  return 0;
}
