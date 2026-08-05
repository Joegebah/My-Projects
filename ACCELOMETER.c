#include <stdint.h>
#include <math.h>
#include <stdio.h>
void printu(uint8_t value1);
void print_string(char *str);
uint8_t i2c_read_register(uint8_t device_addr, uint8_t reg);
void i2c_write_register(uint8_t device_addr, uint8_t reg, uint8_t data);
int16_t combine(uint8_t device_addr, uint8_t high, uint8_t low);
typedef struct {
    volatile uint32_t CR1;   // (Control Register 1 - on/off, start, stop)
    volatile uint32_t CR2;   // Tells the peripheral what frequency its input clock is running at (in MHz)
    volatile uint32_t OAR1;  // (Own Address Register 1 - slave address)
    volatile uint32_t OAR2;  // (Own Address Register 2 - general call address)
    volatile uint32_t DR;    // The mailbox. Write a byte here → it gets sent on the bus. Read from it → you get the byte the sensor sent back.
    volatile uint32_t SR1;   // The flags register — your window into what's happening. You poll bits here to know when each step of the conversation is done
    volatile uint32_t SR2;   // (Status Register 2)
    volatile uint32_t CCR;   // Sets the bus speed — how fast SCL ticks. You put a divisor here that determines whether you run at 100 kHz (standard) or 400 kHz (fast). Similar idea to UART's BRR.
    volatile uint32_t TRISE; // A fine-tuning register for signal timing (the maximum "rise time" of the bus signals).
} I2C_Msg;

typedef struct {
    volatile uint32_t MODER;   // (GPIO port mode register)
    volatile uint32_t OTYPER;  // (GPIO port output type register)
    volatile uint32_t OSPEEDR; // (GPIO port output speed register)
    volatile uint32_t PUPDR;   // (GPIO port pull-up/pull-down register)
    volatile uint32_t IDR;     // (GPIO port input data register)
    volatile uint32_t ODR;     // (GPIO port output data register)
    volatile uint32_t BSRR;    // (GPIO port bit set/reset register)
    volatile uint32_t LCKR;    // (GPIO port configuration lock register)
    volatile uint32_t AFRL;    // (GPIO alternate function low register)
    volatile uint32_t AFRH;    // (GPIO alternate function high register)

} GPIO_Msg;
typedef struct {
    volatile uint32_t CR;           // (Clock control register)
    volatile uint32_t PLLCFGR;      // (PLL configuration register)
    volatile uint32_t CFGR;         // (Clock configuration register)
    volatile uint32_t CIR;          // (Clock interrupt register)
    volatile uint32_t AHB1RSTR;     // (AHB1 peripheral reset register)
    volatile uint32_t AHB2RSTR;     // (AHB2 peripheral reset register)
    volatile uint32_t AHB3RSTR;     // (AHB3 peripheral reset register)
    volatile uint32_t RESERVED0;    // Reserved
    volatile uint32_t APB1RSTR;     // (APB1 peripheral reset register)
    volatile uint32_t APB2RSTR;     // (APB2 peripheral reset register)
    volatile uint32_t RESERVED1[2]; // Reserved
    volatile uint32_t AHB1ENR;      // (AHB1 peripheral clock enable register)
    volatile uint32_t AHB2ENR;      // (AHB2 peripheral clock enable register)
    volatile uint32_t AHB3ENR;      // (AHB3 peripheral clock enable register)
    volatile uint32_t RESERVED2;    // Reserved
    volatile uint32_t APB1ENR;      // (APB1 peripheral clock enable register)
    volatile uint32_t APB2ENR;      // (APB2 peripheral clock enable register)
} RCC_Msg;
typedef struct{
    volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR;
} USART_Msg;

#define I2C1 ((volatile I2C_Msg *) 0x40005400) // Base address of I2C peripheral)
#define GPIOB ((volatile GPIO_Msg *) 0x40020400) // Base address of GPIOB peripheral)
#define RCC ((volatile RCC_Msg *) 0x40023800) // Base address of RCC peripheral)
#define GPIOA ((volatile GPIO_Msg *) 0x40020000) // Base adress for GPIOA
#define USART ((volatile USART_Msg *) 0x40004400) // Base adress for USART
#define CPACR (*((volatile uint32_t*) 0xE000ED88))

int main(void){
    CPACR |= (15<<20);
    RCC->AHB1ENR |= (1<<1); // Enable GPIOB clock
    RCC->APB1ENR |= (1<<21); // Enable I2C1 clock
    GPIOB->MODER &= ~((3<<16) | (3<<18)); // Clear mode bits for PB8&PB9
    GPIOB->MODER |= (2<<16) | (2<<18); // Set mode bits for PB8&PB9 as alternate function
    GPIOB->AFRH &= ~((0xF<<0) | (0xF<<4)); // Clear alternate function bits for PB8&PB9
    GPIOB->AFRH |= (4<<0) | (4<<4); // Set alternate function bits for PB8&PB9 as AF4 (I2C1)
    GPIOB->OTYPER |= (1<<8) | (1<<9); // shared bus, pins can only pull low so devices don't fight
    I2C1->CR2 = 16; // Set clock freq. to the clock of the MCU (16 MHz)
    // Formula for CCR: CCR = APB1_clock / (2 × desired_SCL_speed)
    I2C1->CCR = 80; // Set clock control register for 100 kHz SCL speed (STandard mode)
    // Formula for TRISE: TRISE = (APB1_clock_in_MHz) + 1
    I2C1->TRISE = 17; // Set maximum rise time for standard mode
    /* Because I²C pins are open-drain,
     the line doesn't snap HIGH instantly
      — the pull-up resistor drags it up,
       which takes a little time (the "rise time").
        The peripheral needs to know the maximum rise time so its timing stays within the I²C spec. 
        TRISE tells it that limit.*/
    I2C1->CR1 |= (1<<0); // Enable I2C
    // now printing this value with the USART protocol
    RCC->APB1ENR |= (1<<17);
    RCC->AHB1ENR |= (1<<0);
    GPIOA->MODER &= ~(3<<4);
    GPIOA->MODER |= (2<<4);
    GPIOA->AFRL &= ~(15<<8);
    GPIOA->AFRL |= (7<<8);
    USART->BRR = 139;
    USART->CR1 |= (1<<3) | (1<<13); // enable transmitter and USART
    i2c_write_register(0x68, 0x6B, 0);
    while(1){
        int16_t X = combine(0x68, 0x3B, 0x3C);
        int16_t Y = combine(0x68, 0x3D, 0x3E);
        int16_t Z = combine(0x68, 0x3F, 0x40);
        float pitch = atan2(X, sqrt(Y*Y + Z*Z)) * 180.0 / M_PI;
        float roll = atan2(Y, sqrt(X*X + Z*Z)) * 180.0 /M_PI;
        char buf[40];
        sprintf(buf, "Pitch: %d  Roll: %d\r\n", (int)pitch, (int)roll);
        print_string(buf);
        for (volatile int i; i<300000000; i++);
    }

}
void print_string(char *str){
    int i = 0;
    while(str[i] !='\0'){
        while (!(USART->SR & (1<<7)));   // wait TXE
        USART->DR = str[i];
        i++;
    }
}

int16_t combine(uint8_t device_addr, uint8_t high, uint8_t low){
    uint8_t vh = i2c_read_register(device_addr, high);
    uint8_t vl = i2c_read_register(device_addr, low);
    int16_t v = (vh<<8) | vl;
    return v;
}
uint8_t i2c_read_register(uint8_t device_addr, uint8_t reg) {
    /*1. START
    2. Send device address + WRITE bit → (tell it "I want to talk, and I'm going to write")
    3. Send the register number we want (0x75)
    4. Repeated START  → (turn the bus around)
    5. Send device address + READ bit → ("now I want to read")
    6. Read the byte the sensor sends
    7. STOP*/
    I2C1->CR1 |= (1<<8); // Generate START condition
    while(!(I2C1->SR1 & (1<<0))); // Wait for SB (Start Bit) flag
    I2C1->DR = (device_addr<<1) | 0; // 1st byte: the device ADDRESS ("talk to chip 0x68, writing")
    while(!(I2C1->SR1 & (1<<1))); // Wait for ADDR (Address acknowledged) flag
    (void)I2C1->SR1; // Read SR1 to clear ADDR flag
    (void)I2C1->SR2; // Read SR2 to clear ADDR flag
    I2C1->DR = reg; // 2nd byte: the REGISTER number ("I want register 0x75")
    while (!(I2C1->SR1 & (1<<7))); // Wait for TXE (Data register empty) flag
    I2C1->CR1 |= (1<<8); // Regenerate the Start
    while (!(I2C1->SR1 & (1<<0)));
    I2C1->DR = (device_addr<<1) | 1; // choosing the same slave again (IMU) + Intent of Reading (Read bit 1)
    while(!(I2C1->SR1 & (1<<1)));
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    I2C1->CR1 &= ~(1<<10);
    I2C1->CR1 |= (1<<9);
    /*SR1 isn't one status — it's a register full of separate flags, each reporting a different event.*/
    while(!(I2C1->SR1 & (1<<6)));
    uint8_t value = I2C1->DR;
    return value;

}
void printu(uint8_t value1){
    char digits[] = "0123456789ABCDEF";
    while (!(USART->SR & (1<<7))); // wait for TXE
    USART->DR = '0';
    while (!(USART->SR & (1<<7))); // wait for TXE
    USART->DR = 'x';
    int hx1 = (value1>>4);
    while (!(USART->SR & (1<<7))); // wait for TXE
    USART->DR = digits[hx1];
    int hx2 = value1 & 0x0F;
    while (!(USART->SR & (1<<7))); // wait for TXE
    USART->DR = digits[hx2];
    while (!(USART->SR & (1<<7))); // wait for TXE
    USART->DR = '\n';
}
void i2c_write_register(uint8_t device_addr, uint8_t reg, uint8_t data){
    I2C1->CR1 |= (1<<8);
    while(!(I2C1->SR1 & (1<<0)));
    I2C1->DR = (device_addr<<1) | 0;
    while (!(I2C1->SR1 & (1<<1)));
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    I2C1->DR = reg;
    while(!(I2C1->SR1 & (1<<7))); // TXE
    I2C1->DR = data;
    while(!(I2C1->SR1 & (1<<7)));
    while(!(I2C1->SR1 & (1<<2)));
    I2C1->CR1 |= (1<<9);
}
