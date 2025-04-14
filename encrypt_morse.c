#include <lpc213x.h>
#include <string.h>

/* ——— I²C Slave Addresses ——— */
#define SLAVE_ADDR0 0x70  // LCD0 (raw input) on I2C0
#define SLAVE_ADDR1 0x70  // LCD1 (Encrypted + Morse) on I2C1

/* ——— VIGENÈRE KEY (digits only) ——— */
const char *VIG_KEY = "SECURE";  
#define VIG_KEY_LEN (sizeof(VIG_KEY)-1)

/* ——— Keypad & Morse Tables (unchanged) ——— */
#define MAX 12
#define AA 2
#define SI 3
#define STO 4
#define STA 5
#define I2EN 6

unsigned char write[] = {0xF7,0xFD,0xFB,0xFE};
unsigned char comb[] = {
 0x77,0xB7,0xD7,0xE7, 0x7B,0xBB,0xDB,0xEB,
 0x7D,0xBD,0xDD,0xED, 0x7E,0xBE,0xDE,0xEE
};
char num[] = "0000032106540987";

/* Unused in this flow, but kept for full original code integrity */
unsigned char seg_values[] = {
 0x3F,0x3F,0x3F,0x3F,0x3F,0x4F,0x5B,0x06,
 0x3F,0x7D,0x6D,0x66,0x3F,0x6F,0x7F,0x07
};
const char *digitMorseCodeTable[10] = {
    "-----", ".----", "..---", "...--", "....-",
    ".....", "-....", "--...", "---..", "----."
};

/* ——— Prototypes ——— */
void wait(int count);
void delay_ms(int count);
void delay_ms_1(int j);
void I2C_Init(void);
int  I2C1_Start(void);
void senddata0(char data);
void senddata1(char data);
void sendchar0(char data);
void sendchar1(char data);
void LCD_init(void);
void init(void);
char GetKey(void);
const char *getMorseCode(char digit);
void led(char input);
char vigenere_encrypt(char digit, int pos);

/* ——— Simple busy-wait ——— */
void wait(int count) {
    while (count--) ;
}

/* ——— Delays ——— */
void delay_ms(int count) {
    int j, i;
    for (j = 0; j < count; j++)
        for (i = 0; i < 35; i++) ; // ~10 µs @60 MHz
}
void delay_ms_1(int j) {
    int x, i;
    for (i = 0; i < j; i++)
        for (x = 0; x < 6000; x++) ; // ~1 ms
}

/* ——— I²C Initialization ——— */
void I2C_Init(void) {
    VPBDIV = 0x02;        // PCLK = CCLK/2 = 30 MHz
    PINSEL0 = 0x30C00050; // P0.2/P0.3 = I2C0, P0.10/P0.11 = I2C1
    // I2C1
    I2C1SCLH = 150; I2C1SCLL = 150;
    I2C1CONSET = (1 << I2EN);
    // I2C0
    I2C0SCLH = 150; I2C0SCLL = 150;
    I2C0CONSET = (1 << I2EN);
}

/* ——— Start both I2C buses ——— */
int I2C1_Start() {
    I2C1CONCLR = 1 << STO;
    I2C1CONCLR = 1 << SI;
    I2C1CONSET = 1 << STA;
    I2C0CONCLR = 1 << STO;
    I2C0CONCLR = 1 << SI;
    I2C0CONSET = 1 << STA;
    return 0;
}

/* ——— Low-level byte writes ——— */
void senddata1(char data) {
    while (!(I2C1CONSET & (1 << SI))) ;
    I2C1DAT = data;
    I2C1CONCLR = 1 << SI;
    delay_ms(200);
}
void senddata0(char data) {
    while (!(I2C0CONSET & (1 << SI))) ;
    I2C0DAT = data;
    I2C0CONCLR = 1 << SI;
    delay_ms(200);
}

/* ——— Nibble + EN toggles ——— */
void sendchar1(char data) {
    senddata1(0x50 | (data >> 4));
    delay_ms(50);
    senddata1(0x40 | (data >> 4));
    delay_ms(50);
    senddata1(0x50 | (data & 0x0F));
    delay_ms(50);
    senddata1(0x40 | (data & 0x0F));
    delay_ms(50);
    delay_ms(50);
}
void sendchar0(char data) {
    senddata0(0x50 | (data >> 4));
    delay_ms(50);
    senddata0(0x40 | (data >> 4));
    delay_ms(50);
    senddata0(0x50 | (data & 0x0F));
    delay_ms(50);
    senddata0(0x40 | (data & 0x0F));
    delay_ms(50);
    delay_ms(50);
}

/* ——— Initialize both LCDs ——— */
void LCD_init() {
    char code0 = SLAVE_ADDR0;
    char code1 = SLAVE_ADDR1;
    
    I2C_Init();
    I2C1_Start();
    wait(4000);

    // —— LCD0 on I2C0 @0x70 ——
    while (!(I2C0CONSET & (1 << SI))) ;
    IO0SET = (1 << 21);
    I2C0CONCLR = (1 << STO)|(1 << STA);
    I2C0CONSET = (1 << AA);
    I2C0DAT   = code0;
    I2C0CONCLR = 1 << SI;

    // Initialize LCD0 (raw input) on I2C0
    senddata0(0x10); senddata0(0x00);
    senddata0(0x12); senddata0(0x02);
    senddata0(0x12); senddata0(0x02);
    senddata0(0x18); senddata0(0x08);
    senddata0(0x10); senddata0(0x00);
    senddata0(0x1E); senddata0(0x0E);
    senddata0(0x10); senddata0(0x00);
    senddata0(0x16); senddata0(0x06);
    senddata0(0x10); senddata0(0x00);
    senddata0(0x11); senddata0(0x01);
    senddata0(0x18); senddata0(0x08);
    senddata0(0x10); senddata0(0x00);
    
    // Clear LCD0
    sendchar0(0x01);
    delay_ms(2000);

    // —— LCD1 on I2C1 @0x70 ——
    while (!(I2C1CONSET & (1 << SI))) ;
    IO1SET = (1 << 21);
    I2C1CONCLR = (1 << STO)|(1 << STA);
    I2C1CONSET = (1 << AA);
    I2C1DAT   = code1;
    I2C1CONCLR = 1 << SI;
    
    // Initialize LCD1 (Encrypted + Morse) on I2C1
    senddata1(0x10); senddata1(0x00);
    senddata1(0x12); senddata1(0x02);
    senddata1(0x12); senddata1(0x02);
    senddata1(0x18); senddata1(0x08);
    senddata1(0x10); senddata1(0x00);
    senddata1(0x1E); senddata1(0x0E);
    senddata1(0x10); senddata1(0x00);
    senddata1(0x16); senddata1(0x06);
    senddata1(0x10); senddata1(0x00);
    senddata1(0x11); senddata1(0x01);
    senddata1(0x18); senddata1(0x08);
    senddata1(0x10); senddata1(0x00);
    
    // Clear LCD1
    sendchar1(0x01);
    delay_ms(2000);
}

/* ——— Port & Keypad init ——— */
void init() {
    VPBDIV = 0x02;
    PINSEL1 = 0x0;
    PINSEL2 = 0x0;
    IODIR0 = 0xFF;
    IODIR1 = 0x000F0000; // P1.16–19 = rows
}

/* ——— Keypad scan (unchanged) ——— */
char GetKey() {
    int row = 0, ind = 0, i, temp;
    unsigned char w, w_final;
    while (1) {
        IO1CLR = 0xFFFFFFFF;
        w = write[row];
        IO1SET |= (w << 16);
        delay_ms(1000);
        temp     = IO1PIN;
        w_final  = (temp >> 16) & 0xFF;
        if (w_final != w) break;
        if (++row >= 4) row = 0;
    }
    for (i = 0; i < 16; i++)
        if (comb[i] == w_final) ind = i;
    return num[ind];
}

/* ——— Morse lookup (unchanged) ——— */
const char *getMorseCode(char digit) {
    switch (digit) {
    case '0': return "-----";
    case '1': return ".----";
    case '2': return "..---";
    case '3': return "...--";
    case '4': return "....-";
    case '5': return ".....";
    case '6': return "-....";
    case '7': return "--...";
    case '8': return "---..";
    case '9': return "----.";
    default:  return "";
    }
}

/* ——— Vigenère-style shift on digit ——— */
char vigenere_encrypt(char digit, int pos) {
    int shift = VIG_KEY[pos % VIG_KEY_LEN] - 'A';
    if (digit >= '0' && digit <= '9')
        return '0' + ((digit - '0' + shift) % 10);
    return digit;
}

/* ——— LED blink (unchanged) ——— */
void led(char input) {
    int dot = 100, dash = 250;
    switch (input) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': {
        const char *m = getMorseCode(input);
        while (*m) {
            IOSET0 = 1<<0;
            if (*m == '.') delay_ms_1(dot);
            else           delay_ms_1(dash);
            IOCLR0 = 1<<0;
            delay_ms_1(dot);
            m++;
        }
        delay_ms_1(500);
        break;
    }
    default: break;
    }
}

/* ——— MAIN ——— */
int main() {
    char x, e;
    int count = 100, keypos = 0;
    const char *morseCode;
    int digit_count = 0;   // Count digits entered
    int i;
    
    LCD_init();
    init();

    // Initialize both LCDs
    sendchar0(0x01);  // Clear LCD0
    sendchar1(0x01);  // Clear LCD1
    delay_ms(2000);

    while (count-- > 0) {
        /* Check digit count - CLEAR DISPLAYS IF WE'VE REACHED 4 DIGITS */
        if (digit_count >= 4) {
            digit_count = 0;  // Reset digit counter
            
            // Clear both displays immediately
            sendchar0(0x01);  // Clear LCD0
            sendchar1(0x01);  // Clear LCD1
            delay_ms(500);    // Short delay after clearing
        }
        
        /* Get key from keypad */
        x = GetKey();
        
        /* Encrypt the digit */
        e = vigenere_encrypt(x, keypos++);
        
        /* Update LCD0 with the current digit */
        if (digit_count == 0) {
            // For first digit, ensure the display is clear
            sendchar0(0x01);  // Clear display
            sendchar0(0x80);  // Position cursor at start
        } else {
            // For subsequent digits, just move the cursor
            sendchar0(0x80 + digit_count);  // Move cursor to appropriate position
        }
        
        // Display the digit
        sendchar0(x);
        
        /* Update LCD1 with encrypted value and Morse code */
        sendchar1(0x01);  // Clear display
        sendchar1(0x80);  // Position cursor at start
        
        // Show the encrypted digit
        sendchar1(e);
        
        // Display Morse code
        morseCode = getMorseCode(e);
        for (i = 0; morseCode[i]; i++) {
            sendchar1(morseCode[i]);
        }
        
        /* Blink LED for the morse code */
        led(e);
        
        /* Increment digit count AFTER processing the current digit */
        digit_count++;
        
        /* Wait before next input */
        delay_ms(1000);
    }

    /* Turn all LCDs off */
    senddata0(0x10); senddata0(0x08);  // Display off for LCD0
    senddata1(0x10); senddata1(0x08);  // Display off for LCD1

    return 0;
}