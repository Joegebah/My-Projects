#include <stdint.h>

typedef struct {
    volatile uint32_t RESERVED[12];
    volatile uint32_t AHB1ENR;
    volatile uint32_t RESERVED1[3];
    volatile uint32_t APB1ENR;
} RCC_TYPE;
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TYPE;
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TYPE;
#define RCC ((RCC_TYPE *)0x40023800)
#define GPIOA ((GPIO_TYPE *)0x40020000)
#define USART2 ((USART_TYPE *)0x40004400)

int main(void){
    RCC->APB1ENR |= (1<<17); // Enable USART2 clock
    RCC->AHB1ENR |= (1<<0);  // Enable GPIOA
    GPIOA->MODER &= ~(3<<4); // Clear mode for PA2
    GPIOA->MODER |= (2<<4);  // Set PA2 to alternate function mode
    GPIOA->AFR[0] &= ~(0xF<<8); // Clear alternate function for PA2
    GPIOA->AFR[0] |= (7<<8);  // Set alternate function 7 (USART2) for PA2
    USART2->BRR = 139; // Set baud rate to 115200 (16 MHz clock)
    USART2->CR1 |= (1<<3) | (1<<13); // enable transmitter and USART
    while (1) {
        while (!(USART2->SR & (1 << 7))); // wait until TXE (bit 7) is set
        USART2->DR = 'A'; 
        for (volatile int i = 0; i < 100000; i++); // Delay loop
    }
}





