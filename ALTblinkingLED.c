#include <stdint.h>

// Define the Registers

#define rcc_ahb1enr (*((volatile uint32_t*) 0x40023830))
#define gpioa_moder (*((volatile uint32_t*) 0x40020000))
#define gpioa_odr (*((volatile uint32_t*) 0x40020014))

int main(void)

	rcc_ahb1enr |= 0x1;

	gpioa_moder &= ~0xC00;

	gpioa_moder |= 0x400;

	while(1)
	{
		int j=0;
		while(j<6)
		{
				gpioa_odr ^= 0x20;
				j++;
				for (volatile int i=0; i<1000000; i++);

	}

		int x=0;
		while(x<6)
		{
				gpioa_odr ^= 0x20;
				x++;

				for (volatile int y=0; y<500000; y++);
	}


	}


}
