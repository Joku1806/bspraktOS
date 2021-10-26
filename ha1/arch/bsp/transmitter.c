#include <stdint.h>


#define UART_BASE 0x7E201000 //offset nicht einberechnet

//registers

enum registers{//wir brauchen vermutlich nicht alle-- nutzlose noch entfernen
    DR = 0x0,
    FR = 0x18,
    IBRD = 0x24,
    FBRD = 0x28,
    LCRH = 0x2c,
    CR = 0x30,
    IFLS = 0x34,
    IMSC = 0x38,
    RIS = 0x3c,
    MIS = 0x40,
    ICR = 0x44,
    DMACR = 0x48,
    ITCR = 0x80,
    ITIP = 0x84,
    ITOP = 0x88,
    TDR = 0x8c
}; //ggf statt einem enum einfach als const values ODER #define deklarieren

volatile uint32_t *target(uint32_t offset) //offset ist jeweils aus dem enum "registers" 
{

    const uint64_t adress = UART_BASE + offset;

    return (volatile uint32_t*) ((void*) adress);//typecast (void*) ggf unnötig
}

void wait()
{   
    //Soll warten bis task completed ist
    //TODO
}

void send(char * data, size_t size)
{
    wait();

    for(int i = 0; i < size; i++)
    {
        if(data[i] == NULL)//ausnahmefall TODO
        {
            wait()
        }

        *target(registers.DR) = data[i]; //tatsächliches senden von chars
        wait()
    }

}