#include <stdlib.h>  // Required for atoi()

#define RCC_APB2ENR  (*(volatile unsigned int*)0x40021018)
#define GPIOA_CRH    (*(volatile unsigned int*)0x40010804)
#define USART1_SR    (*(volatile unsigned int*)0x40013800)
#define USART1_DR    (*(volatile unsigned int*)0x40013804)
#define USART1_BRR   (*(volatile unsigned int*)0x40013808)
#define USART1_CR1   (*(volatile unsigned int*)0x4001380C)

void uart_init(void) {
    RCC_APB2ENR |= (1 << 2);   // Enable GPIOA clock
    RCC_APB2ENR |= (1 << 14);  // Enable USART1 clock

    // PA9 (TX)
    GPIOA_CRH &= ~(0xF << (1 * 4));
    GPIOA_CRH |= (0xB << (1 * 4));

    // PA10 (RX)
    GPIOA_CRH &= ~(0xF << (2 * 4));
    GPIOA_CRH |= (0x4 << (2 * 4));

    USART1_BRR = 0x341; // Baud rate
    USART1_CR1 |= (1 << 13) | (1 << 3) | (1 << 2); // Enable USART, TX, RX
}

void uart_send_char(char c) {
    while (!(USART1_SR & (1 << 7))); // Wait for TX buffer empty
    USART1_DR = c;
}

char uart_receive_char(void) {
    while (!(USART1_SR & (1 << 5))); // Wait for data
    return (char)USART1_DR;
}

void uart_send_string(char *str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

#define GPIOB_CRL  (*(volatile unsigned int*)0x40010C00)
#define GPIOB_BSRR (*(volatile unsigned int*)0x40010C10)

void gpio_init(void) {
    RCC_APB2ENR |= (1 << 3);  // Enable GPIOB clock
    GPIOB_CRL &= ~(0xF << (2 * 4));
    GPIOB_CRL |= (0x2 << (2 * 4)); // PB2 as output
}

void blink_led(int times) {
    for (int i = 0; i < times; i++) {
        GPIOB_BSRR = (1 << 2);  // Turn on LED
        for (volatile int j = 0; j < 500000; j++);
        GPIOB_BSRR = (1 << (2 + 16)); // Turn off LED
        for (volatile int j = 0; j < 500000; j++);
    }
}

void uart_receive_string(char *buffer, int maxLength) {
    int i = 0;
    char ch;
    while (i < maxLength - 1) {
        ch = uart_receive_char();
        if (ch == '\n' || ch == ' ') {
            break;  
        }
        uart_send_char(ch);  
        buffer[i++] = ch;
    }
    buffer[i] = '\0'; 
}

int main(void) {
    uart_init();
    gpio_init();
    uart_send_string("\r\nReady to receive blink count (1-999):");

    int busy = 0; 
    char inputBuffer[5];  

    while (1) {
        if (!busy) {
            uart_receive_string(inputBuffer, sizeof(inputBuffer));

            // Convert string to integer
            int num = atoi(inputBuffer);

          
            if (num > 0 && num <= 999) {
                busy = 1; 
                uart_send_string("\r\nBlinking LED...");
                blink_led(num);
                busy = 0; 
                uart_send_string("\r\nReady for next input:");
            } else {
                uart_send_string("\r\nInvalid input! Enter a number (1-999):");
            }
        }
    }
}
