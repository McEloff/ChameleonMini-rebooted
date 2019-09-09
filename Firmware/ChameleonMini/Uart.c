#include "Uart.h"
//����FIFO
fifo_buffer_t fifo_buf=
{
	.len=0,
	.wr_ptr=0,
	.rd_ptr=0,
	.buf={0},
};
//FIFO��������
void fifo_buffer_input(uint8_t data)
{
	fifo_buf.buf[fifo_buf.wr_ptr] = data;
	fifo_buf.wr_ptr = (fifo_buf.wr_ptr + 1) % CFG_UART_BUFSIZE;
	fifo_buf.len++;
}
//FIFO���ݶ�ȡ
uint8_t fifo_buffer_read(void)
{
	uint8_t data;
	data = fifo_buf.buf[fifo_buf.rd_ptr];
	fifo_buf.rd_ptr = (fifo_buf.rd_ptr + 1) % CFG_UART_BUFSIZE;
	fifo_buf.len--;
	return data;
}
//FIFO�������
void fifo_buffer_clear(void)
{
	fifo_buf.rd_ptr = 0;
	fifo_buf.wr_ptr = 0;
	fifo_buf.len = 0;
}

void delay_us(unsigned int t)
{
	volatile  int i;
	for( i=0;i<t;i++)
	{
		volatile  int a=4;
		while(a--);
	}
}

uint16_t user_read(uint8_t *rx)
{
	uint8_t *ptr = rx;
	//��ֵ
	uint32_t len=fifo_buf.len;
	uint32_t lenadd=0;
	if(len)
	{
		while(fifo_buf.len)
		{
			if(fifo_buf.len > 0)
			{
				(*ptr++) = fifo_buffer_read();
				lenadd++;
			}
 
			delay_us(60);//20����
		}
		return lenadd;
	}
	else
	return 0;	
}
/*
+------------------------------------------------------------------------------
| Function    : uart_putc
+------------------------------------------------------------------------------
| Description : send the data
|
| Parameters  :  DATA to send  (HEX)
| Returns     :
|
+------------------------------------------------------------------------------
*/
void uart_putc(unsigned char c)
{
	if(c == '\n')
	uart_putc('\r');

	/* wait until transmit buffer is empty */
	while(!(USART.STATUS & USART_DREIF_bm));

	/* send next byte */
	USART.DATA = c;
}

/*
+------------------------------------------------------------------------------
| Function    : uart_putc_hex
+------------------------------------------------------------------------------
| Description : convert the data to ASCII(2 Bytes) and send it
|				0xf1 -->> 'F'+'1'
| Parameters  :  DATA to send  (8 bits HEX)
| Returns     :
|
+------------------------------------------------------------------------------
*/
void uart_putc_hex(unsigned char b)
{
	/* upper nibble */
	if((b >> 4) < 0x0a)
	uart_putc((b >> 4) + '0');
	else
	uart_putc((b >> 4) - 0x0a + 'a');

	/* lower nibble */
	if((b & 0x0f) < 0x0a)
	uart_putc((b & 0x0f) + '0');
	else
	uart_putc((b & 0x0f) - 0x0a + 'a');
}

/*
+------------------------------------------------------------------------------
| Function    : uart_putw_hex
+------------------------------------------------------------------------------
| Description : convert the data to ASCII(4 Bytes) and send it
|				0xf1e0 -->> 'f'+'1'+'e'+'0'
| Parameters  :  DATA to send  (16 bits HEX)
| Returns     :
|
+------------------------------------------------------------------------------
*/
void uart_putw_hex(unsigned int w)
{
	uart_putc_hex((unsigned char) (w >> 8));
	uart_putc_hex((unsigned char) (w & 0xff));
}

/*
+------------------------------------------------------------------------------
| Function    : uart_putdw_hex
+------------------------------------------------------------------------------
| Description : convert the data to ASCII(2 Bytes) and send it
|				0xf1e01234 -->> 'f'+'1'+'e'+'0'+'1'+'2'+'3'+'4'
| Parameters  :  DATA to send  (32 bits HEX)
| Returns     :
|
+------------------------------------------------------------------------------
*/
void uart_putdw_hex(unsigned long dw)
{
	uart_putw_hex((unsigned int) (dw >> 16));
	uart_putw_hex((unsigned int) (dw & 0xffff));
}

/*
+------------------------------------------------------------------------------
| Function    : uart_putw_dec
+------------------------------------------------------------------------------
| Description : convert the data to decimal numbers and send it
|				0x123 -->> '2'+'9'+'1'
| Parameters  :  DATA to send  (16 bits HEX)
| Returns     :
|
+------------------------------------------------------------------------------
*/
void uart_putw_dec(unsigned int w)
{
	unsigned int num = 10000;
	unsigned char started = 0;

	while(num > 0)
	{
		unsigned char b = w / num;
		if(b > 0 || started || num == 1)
		{
			uart_putc('0' + b);
			started = 1;
		}
		w -= b * num;

		num /= 10;
	}
}

/*
+------------------------------------------------------------------------------
| Function    : uart_putdw_dec
+------------------------------------------------------------------------------
| Description : convert the data to decimal numbers and send it
|				0x123 -->> '2'+'9'+'1'
| Parameters  :  DATA to send  (32 bits HEX)
| Returns     :
|
+------------------------------------------------------------------------------
*/
void uart_putdw_dec(unsigned long dw)
{
	unsigned long num = 1000000000;
	unsigned char started = 0;

	while(num > 0)
	{
		unsigned char b = dw / num;
		if(b > 0 || started || num == 1)
		{
			uart_putc('0' + b);
			started = 1;
		}
		dw -= b * num;

		num /= 10;
	}
}

/*
+------------------------------------------------------------------------------
| Function    : uart_puts
+------------------------------------------------------------------------------
| Description : send string
|
| Parameters  : string to send
| Returns     :
|
+------------------------------------------------------------------------------
*/
void uart_puts(const char* str)
{
	while(*str)
	uart_putc(*str++);
}

/*
+------------------------------------------------------------------------------
| Function    : uart_getc
+------------------------------------------------------------------------------
| Description : receive one byte
|
| Parameters  :
| Returns     : data
|
+------------------------------------------------------------------------------
*/
unsigned char uart_getc(void)
{
	/* wait until receive buffer is full */
	while(!(USART.STATUS & USART_RXCIF_bm));

	unsigned char b = USART.DATA;
	if(b == '\r')
	b = '\n';
	
	return b;
}
//+------------------------------------------------------------------------------


void uart_init(void)
{
	/* USART ���ŷ�������*/
	///* PC3 (TXD0) ��� */
	//PORTC.DIRSET   = PIN3_bm;
	///* PC2 (RXD0) ���� */
	//PORTC.DIRCLR   = PIN2_bm;
	/* PE3 (TXD0) ��� */
	PORTE.DIRSET   = PIN3_bm;
	/* PE2 (RXD0) ���� */
	PORTE.DIRCLR   = PIN2_bm;

	/* USART ģʽ - �첽*/
	USART_SetMode(&USART,USART_CMODE_ASYNCHRONOUS_gc);
	/* USARTE0֡�ṹ, 8 λ����λ, ��У��, 1ֹͣλ */
	USART_Format_Set(&USART, USART_CHSIZE_8BIT_gc,USART_PMODE_DISABLED_gc, 0);
	/* ���ò����� 115200* 32M*/ 
	USART_Baudrate_Set(&USART, 16 , 0);
	///* ���ò����� 9600* 32M*/
	//USART_Baudrate_Set(&USART, 207 , 0);
	/* USART ʹ�ܷ���*/
	USART_Tx_Enable(&USART);
	/* USART ʹ�ܽ���*/
	USART_Rx_Enable(&USART);

	///* USART �����жϼ���*/
	//USART_RxdInterruptLevel_Set(&USART,USART_RXCINTLVL_LO_gc);
	USART_RxdInterruptLevel_Set(&USART,USART_RXCINTLVL_HI_gc);
	
}

/*
+------------------------------------------------------------------------------
| Function    : ISR(USARTE1_RXC_vect)
+------------------------------------------------------------------------------
| Description : USART�����жϺ��� �յ������ݷ��ͻ�ȥUSART
+------------------------------------------------------------------------------
*/
ISR(USARTE0_RXC_vect)  
{
    fifo_buffer_input(USART_GetChar(&USART));	
}