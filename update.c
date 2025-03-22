#include <stdint.h>

#define RCC_APB2ENR  (*(volatile unsigned int*)0x40021018)
#define RCC_APB1ENR  (*(volatile unsigned int*)0x4002101C)
#define GPIOA_CRH    (*(volatile unsigned int*)0x40010804)
#define USART1_SR    (*(volatile unsigned int*)0x40013800)
#define USART1_DR    (*(volatile unsigned int*)0x40013804)
#define USART1_BRR   (*(volatile unsigned int*)0x40013808)
#define USART1_CR1   (*(volatile unsigned int*)0x4001380C)

#define GPIOB_CRL  (*(volatile unsigned int*)0x40010C00)
#define GPIOB_BSRR (*(volatile unsigned int*)0x40010C10)

#define TIM2_CR1  (*(volatile unsigned int*)0x40000000)
#define TIM2_CNT  (*(volatile unsigned int*)0x40000024)
#define TIM2_PSC  (*(volatile unsigned int*)0x40000028)
#define TIM2_ARR  (*(volatile unsigned int*)0x4000002C)
#define TIM2_SR   (*(volatile unsigned int*)0x40000010)

volatile int blink_count = 0;
volatile int busy = 0;
volatile int led_state = 0; // 0 = OFF, 1 = ON

char inputBuffer[10];  // User input buffer
volatile int inputIndex = 0;
volatile int newData = 0; // Flag for new data

char queuedData[10]; // Store queued input
volatile int hasQueuedData = 0; // Flag if queued data exists

void uart_init(void) {
    RCC_APB2ENR |= (1 << 2) | (1 << 14); // Enable GPIOA and USART1 clocks

    // PA9 (TX), PA10 (RX)
    GPIOA_CRH &= ~(0xF << (1 * 4));
    GPIOA_CRH |= (0xB << (1 * 4));
    GPIOA_CRH &= ~(0xF << (2 * 4));
    GPIOA_CRH |= (0x4 << (2 * 4));

    USART1_BRR = 0x341; // Baud rate
    USART1_CR1 |= (1 << 13) | (1 << 3) | (1 << 2); // Enable USART, TX, RX
}

void uart_send_char(char c) {
    while (!(USART1_SR & (1 << 7)));
    USART1_DR = c;
}

void uart_send_string(char *str) {
    while (*str) uart_send_char(*str++);
}

void gpio_init(void) {
    RCC_APB2ENR |= (1 << 3);
    GPIOB_CRL &= ~(0xF << (2 * 4));
    GPIOB_CRL |= (0x2 << (2 * 4));
}

void timer_init(void) {
    RCC_APB1ENR |= (1 << 0);
    TIM2_PSC = 8000 - 1;
    TIM2_ARR = 500;
    TIM2_CR1 |= (1 << 0);
}

int string_to_int(char *str) {
    int result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

void uart_check_input(void) {
    if (USART1_SR & (1 << 5)) {
        char ch = USART1_DR;
        uart_send_char(ch);
        if (ch >= '0' && ch <= '9') {
            if (inputIndex < sizeof(inputBuffer) - 1) {
                inputBuffer[inputIndex++] = ch;
                inputBuffer[inputIndex] = '\0';
            }
        } else if (ch == '\r' || ch == '\n' || ch == ' ') {
            if (busy) {
                for (int i = 0; i <= inputIndex; i++) {
                    queuedData[i] = inputBuffer[i];
                }
                hasQueuedData = 1;
                uart_send_string("\r\nThe Input is in queue. Blinking...\r\n");
            } else {
                newData = 1;
            }
            inputIndex = 0;
        }
    }
}

int main(void) {
    uart_init();
    gpio_init();
    timer_init();
    uart_send_string("\r\nReady to receive blink count (1-999):\r\n");

    while (1) {
        uart_check_input();

        if (newData && !busy) {
            newData = 0;
            int num = string_to_int(inputBuffer);

            if (num > 0 && num <= 999) {
                busy = 1;
                blink_count = num;
                uart_send_string("\r\nBlinking LED...\r\n");
            } else {
                uart_send_string("\r\nInvalid input! Enter a number (1-999):\r\n");
            }
        }

        if (blink_count > 0 && (TIM2_SR & (1 << 0))) {
            TIM2_SR &= ~(1 << 0);
            if (led_state == 0) {
                GPIOB_BSRR = (1 << 2);
                led_state = 1;
            } else {
                GPIOB_BSRR = (1 << (2 + 16));
                led_state = 0;
                blink_count--;
            }
        }

        if (blink_count == 0 && busy) {
            busy = 0;
            uart_send_string("\r\nBlinking finished. Ready for next input:\r\n");
            if (hasQueuedData) {
                hasQueuedData = 0;
                uart_send_string("\r\nQueued Data(Press enter to continue: ");
                uart_send_string(queuedData);
                uart_send_string("\r\n");
            }
        }
    }
}
