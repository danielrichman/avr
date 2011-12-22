/* Copyright (C) 2010  Daniel Richman; GNU GPL 3 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static const double centre_latitude = 51.8;
static const double centre_longitude = -0.6;
static const double radius = 5;
static const int steps = 120;   /* therefore 2 minutes for full circle */
static const char *date = "221211";
static const char *altitude = "250.1";
static const char *speed = "162.24";
static int hour = 18;
static int minute = 25;
static int second = 12;

static int current_step = 0;
static int send_position = -1;
static char data[200];

static void make_ddm_str(char *target, int target_size, double value)
{
    int degrees, minutes, milliminutes;
    const char *format;

    if (value < 0)
        value = 0 - value;

    degrees = (int) floor(value);
    value -= floor(value);
    value *= 60;

    minutes = (int) floor(value);
    value -= floor(value);
    value *= 1000;

    milliminutes = (int) floor(value);

    if (target_size == 11)
        format = "%.3d%.2d.%.4d";
    else
        format = "%.2d%.2d.%.4d";

    snprintf(target, target_size, format, degrees, minutes, milliminutes);
}

static void add_checksums()
{
    char *pos;
    unsigned char checksum;
    char buf[3];

    pos = data;
    checksum = 0;

    while (*pos && pos < (data + sizeof(data) - 3))
    {
        /* Start at $, end at * */
        if (*pos == '$')
        {
            checksum = 0;
        }
        else if (*pos == '*')
        {
            snprintf(buf, sizeof(buf), "%.2X", checksum);
            memcpy(pos + 1, buf, 2);
        }
        else
        {
            checksum ^= *pos;
        }

        pos++;
    }
}

static void new_strings()
{
    double latitude, longitude, angle, bearing;
    int bearing_degrees;
    char time_str[7], lat_str[10], lon_str[11];
    char lat_dir, lon_dir;

    angle = (2 * M_PI * current_step) / steps;
    latitude = centre_latitude + (radius * sin(angle));
    longitude = centre_longitude + (radius * cos(angle));

    bearing = angle + M_PI_2;
    bearing_degrees = floor((bearing * 180) / M_PI);

    snprintf(time_str, sizeof(time_str), "%.2d%.2d%.2d", hour, minute, second);
    make_ddm_str(lat_str, sizeof(lat_str), latitude);
    make_ddm_str(lon_str, sizeof(lon_str), longitude);

    lat_dir = (latitude >= 0 ? 'N' : 'S');
    lon_dir = (longitude >= 0 ? 'E' : 'W');

    snprintf(data, sizeof(data), 
        "$GPGGA,%s.00,%s,%c,%s,%c,1,08,1.0,%s,M,,,,*XX\n"
        "$GPRMC,%s.000,A,%s,%c,%s,%c,%s,%.3d,%s,,*XX\n",
        time_str, lat_str, lat_dir, lon_str, lon_dir, altitude,
        time_str, lat_str, lat_dir, lon_str, lon_dir,
        speed, bearing_degrees, date);

    add_checksums();
}

static void advance()
{
    current_step++;
    if (current_step == steps)
        current_step = 0;

    second++;
    if (second == 60)
    {
        second = 0;
        minute++;
        if (minute == 60)
        {
            minute = 0;
            hour++;
            if (hour == 24)
            {
                hour = 0;
            }
        }
    }
}

static int sending()
{
    return send_position != -1;
}

ISR (TIMER1_COMPA_vect)
{
    if (!sending())
    {
        new_strings();
        advance();

        /* Enable TX */
        UCSR0B |= (1 << UDRIE0);
        send_position = 0;
    }
}

ISR (USART_UDRE_vect)
{
    UDR0 = data[send_position];
    send_position++;

    if (send_position >= sizeof(data) || data[send_position] == '\0')
    {
        /* Disable TX */
        UCSR0B &= ~(1 << UDRIE0);

        send_position = -1;
    }
}

int main()
{
    UBRR0H = 0;
    UBRR0L = 103;               /* Setup the USART: 9600 baud */
    UCSR0B = (1 << TXEN0);      /* Enable transmission */

    TCCR1A = (1 << WGM12);      /* Clear timer on compare match (CTC) */
    TCCR1B = (1 << CS12);       /* Prescale 16Mhz/256 => 62500Hz */
    OCR1A = 62500;              /* Compare matches at 62500/62500 = 1Hz */
    TIMSK1 = (1 << OCIE1A);     /* Enable interrupt on compare match */

    sei();                      /* Enable interrupts */

    /* Interrupt driven */
    for (;;)
        sleep_mode();
}
