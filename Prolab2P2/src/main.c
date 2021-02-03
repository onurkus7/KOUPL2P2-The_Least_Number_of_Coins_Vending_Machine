#include <stdint.h>
#include <string.h>
#include "inc/tm4c123gh6pm.h"
#include <stdbool.h>
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#define RW 0x20
#define RS 0x40
#define E  0x80
#define BAUDRATE 600

volatile unsigned long delay;
int sayac_25=0;
int stok_25=0;
int sayac_50=0;
int stok_50=0;
int sayac_100=0;
int stok_100=0;
float toplam_para=0;
float para_ustu=0;
float kalan_para=0;
int but_1,but_2,but_3,but_4,but_5,but_6;
int button_sag, button_sol;


void portlariAktiflestir(void){
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;		// A portunu aktive et
	delay = SYSCTL_RCGC2_R;						// A portunun aktive edilmesini 1 tick bekle
	GPIO_PORTA_AMSEL_R &= ~0b11100000;			// A portunun analog modunu devre dışı bırak
	GPIO_PORTA_PCTL_R &= ~0xFFF00000;			// A portundaki pinlerin voltajını düzenle (PCTL=Power Control)
	GPIO_PORTA_DIR_R |= 0b11100000;				// A portunun giriş çıkışlarını belirle
	GPIO_PORTA_AFSEL_R &= ~0b11100000;			// A portundaki alternatif fonksiyonları seç
	GPIO_PORTA_DEN_R |= 0b11100000;				// A portunun pinlerini aktifleştir
	GPIO_PORTA_DR8R_R |= 0b11100000;			// A portundaki pinlerin 8mA çıkışını aktive et

	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;		// B portunu aktive et
	delay = SYSCTL_RCGC2_R;						// B portunun aktive edilmesini 1 tick bekle
	GPIO_PORTB_AMSEL_R &= ~0b11111111;			// B portunun analog modunu devre dışı bırak
	GPIO_PORTB_PCTL_R &= ~0xFFFFFFFF;			// B portundaki pinlerin voltajını düzenle (PCTL=Power Control)
	GPIO_PORTB_DIR_R |= 0b11111111;				// B portunun giriş çıkışlarını belirle
	GPIO_PORTB_AFSEL_R &= ~0b11111111;			// B portundaki alternatif fonksiyonları seç
	GPIO_PORTB_DEN_R |= 0b11111111;				// B portunun pinlerini aktifleştir
	GPIO_PORTB_DR8R_R |= 0b11111111;			// B portundaki pinlerin 8mA çıkışını aktive et

	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD; // Port D’yi aktiflestir
	delay = SYSCTL_RCGC2_R;  	// zaman gecirmek icin
	GPIO_PORTD_DIR_R |= 0x0F;	// PD 3,2,1,0 pinlerini cikis yap
	GPIO_PORTD_AFSEL_R &= ~0x0F; // PD 3,2,1,0 pinlerini alternatif fonksinunu 0 yap
	GPIO_PORTD_DEN_R |= 0x0F;	// PD 3,2,1,0 pinlerini aktiflestir

	volatile unsigned long tmp; // bu degisken gecikme yapmak icin gerekli
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;   // 1) E portunun osilatörünü etkinleştir
	tmp = SYSCTL_RCGCGPIO_R;    	// allow time for clock to start
	GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port E
	GPIO_PORTE_CR_R = 0x3F;         // allow changes to PE5-0 //PE5-0 değişikliklerine izin ver
	                                   // only PE0 needs to be unlocked, other bits can't be locked
	    			 // Sadece PE0 kilidinin açılması gerekir, diğer bitler kilitlenemez
	GPIO_PORTE_AMSEL_R = 0x00;    	// 3) disable analog on PE //PE'de analog devre dışı bırak
	GPIO_PORTE_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PE4-0
	GPIO_PORTE_DIR_R = 0x00;      	// 5) PE4,PE5 in, PE3-0 out
	GPIO_PORTE_AFSEL_R = 0x00;    	// 6) disable alt funct on PE7-0
	GPIO_PORTE_PUR_R = 0x3F;      	// enable pull-up on PE5 and PE4
	   	   	   	   	   	 //PE4 ve PE5'te pull up'ı etkinleştir ( BUTON İÇİN)
	GPIO_PORTE_DEN_R = 0x3F;      	// 7) enable digital I/O on PE5-0 // portE 5-0 giriş çıkış  etkinlerştir.

	SYSCTL_RCGCGPIO_R |= 0x00000020; // Port F’nin saatini aktifleştir
	tmp = SYSCTL_RCGCGPIO_R; // Saatin başlaması için gecikme
	GPIO_PORTF_LOCK_R = 0x4C4F434B; // Port F GPIO kilidini aç
	GPIO_PORTF_CR_R = 0x01; // PF4-0 kilidini aç (Sadece PF0 kilitlidir, diğer bitler kilitli değildir.)
	GPIO_PORTF_DIR_R = 0x0E; // PF4,PF0 giriş, PF3-1 çıkış
	GPIO_PORTF_PUR_R = 0x11; // PF0 ve PF4 üzerindeki pull-up direncini aktifleştir
	GPIO_PORTF_DEN_R = 0x1F; // PF4-0 digital I/O aktifleştir
}



void komutGonder(unsigned char LCD_Comment){
	GPIO_PORTA_DATA_R &= ~(RS+RW+E);
	GPIO_PORTB_DATA_R = LCD_Comment;
	GPIO_PORTA_DATA_R |= E;
	GPIO_PORTA_DATA_R &= ~(RS+RW);
	for (delay = 0 ; delay < 1; delay++);
	GPIO_PORTA_DATA_R &= ~(RS+RW+E);
	for (delay = 0 ; delay < 1000; delay++);
}

void veriGonder(unsigned char LCD_Data){
	GPIO_PORTB_DATA_R = LCD_Data;
	GPIO_PORTA_DATA_R |= RS+E;
	GPIO_PORTA_DATA_R &= ~RW;
	for (delay = 0 ; delay < 23 ; delay++);
	GPIO_PORTA_DATA_R &= ~(RS+RW+E);
	for (delay = 0 ; delay < 1000; delay++);
}

void ekraniAktiflestir(){
	portlariAktiflestir();
	for (delay = 0 ; delay < 15000; delay++);
	komutGonder(0x38);
	for (delay = 0 ; delay < 5000; delay++);
	komutGonder(0x38);
	for (delay = 0 ; delay < 150; delay++);
	komutGonder(0x0C);
	komutGonder(0x01);
	komutGonder(0x06);
	for (delay = 0 ; delay < 50000; delay++);
}

void ekranaYazdir(unsigned int line,unsigned int digit, unsigned char *str){
	unsigned int lineCode = line==1 ?0x80:0xC0;
	komutGonder(lineCode + digit);
	while(*str != 0){ veriGonder(*str++); }
}
void init_UARTstdio() {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(0x00000001);
	GPIOPinConfigure(0x00000401);
	GPIOPinTypeUART(0x40004000, 0x00000001 | 0x00000002);
	UARTConfigSetExpClk(0x40004000, SysCtlClockGet(), BAUDRATE,
                        	(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         	UART_CONFIG_PAR_NONE));
	UARTStdioConfig(0, BAUDRATE, SysCtlClockGet());
}



int main(){
	init_UARTstdio();
	ekraniAktiflestir();
	int para_sks;
	stok_25=20;
	stok_50=20;
	stok_100=10;
	toplam_para=((float)stok_25*0.25)+((float)stok_50*0.5)+((float)stok_100*1);
	int secim=0;
	int urun_stok[5];
	urun_stok[0]=30;
	urun_stok[1]=20;
	urun_stok[2]=15;
	urun_stok[3]=50;
	urun_stok[4]=100;
	float urun_fiyat[5];
	urun_fiyat[0]=0.5;
	urun_fiyat[1]=1;
	urun_fiyat[2]=1.5;
	urun_fiyat[3]=1.75;
	urun_fiyat[4]=2;
	int toplam;
	int i=0;
		for(i=0;i<5;i++){
			UARTprintf("test: %d\n", i);
		}
	while(1){
		while(1){
				but_1=GPIO_PORTE_DATA_R &= 0x01;
				but_2=GPIO_PORTE_DATA_R &= 0x02;
				but_3=GPIO_PORTE_DATA_R &= 0x04;
				but_4=GPIO_PORTE_DATA_R &= 0x08;
				but_5=GPIO_PORTE_DATA_R &= 0x10;
				but_6=GPIO_PORTE_DATA_R &= 0x20;
				button_sag = GPIO_PORTF_DATA_R & 0b00001;
				button_sol = GPIO_PORTF_DATA_R & 0b10000;
			if(secim==0){
			if(but_1==0){
				ekranaYazdir(1,0,"               ");
				ekranaYazdir(1,0,"0.25 kurus");
				toplam++;
				kalan_para+=0.25;
				sayac_25++;
				stok_25++;

			}else if(but_2==0){
				ekranaYazdir(1,0,"               ");
				ekranaYazdir(1,0,"0.50 kurus");
				toplam++;
				kalan_para+=0.5;
				sayac_50++;
				stok_50++;

			}else if(but_3==0){
				ekranaYazdir(1,0,"               ");
				ekranaYazdir(1,0,"1 lira");
				toplam++;
				kalan_para+=1;
				sayac_100++;
				stok_100++;

			}
			else if(button_sag==0){
				toplam_para=((float)sayac_25*0.25)+((float)sayac_50*0.5)+((float)sayac_100*1);
				ekranaYazdir(1,0,"               ");
				ekranaYazdir(1,0,"toplam para: ");
				secim++;
				toplam++;

			}else if(button_sol==0){
				toplam++;
				break;


			}
			}else if(secim==1){
				if(but_1==0){
					ekranaYazdir(1,0,"               ");
					ekranaYazdir(1,0,"Su : 0.5 Tl");
					urun_stok[0]--;
					toplam_para-=urun_fiyat[0];
					toplam++;

				}else if(but_2==0){
					ekranaYazdir(1,0,"               ");
					ekranaYazdir(1,0,"Cay : 1 Tl");
					urun_stok[1]--;
					toplam_para-=urun_fiyat[1];
					toplam++;

				}else if(but_3==0){
					ekranaYazdir(1,0,"               ");
					ekranaYazdir(1,0,"Kahve : 1.5 Tl");
					urun_stok[2]--;
					toplam_para-=urun_fiyat[2];
					toplam++;

				}else if(but_4==0){
					ekranaYazdir(1,0,"               ");
					ekranaYazdir(1,0,"Cikolata : 1.75 Tl");
					urun_stok[3]--;
					toplam_para-=urun_fiyat[3];
					toplam++;

				}else if(but_6==0){
					ekranaYazdir(1,0,"               ");
					ekranaYazdir(1,0,"Biskuvi : 2 Tl");
					toplam_para-=urun_fiyat[4];
					urun_stok[4]--;
					toplam++;
				}
				else if(button_sag==0){
					secim++;
					ekranaYazdir(1,0,"               ");
					ekranaYazdir(1,0,"Islem bitti");
					toplam++;


				}else if(button_sol==0){
					toplam++;
					break;

				}
			}
			para_sks=toplam%4;
			if(para_sks==2){
				ekranaYazdir(1,0,"               ");
				ekranaYazdir(1,0,"Para sikisti.");

				GPIO_PORTD_DATA_R |= 0b0010;
				GPIO_PORTD_DATA_R &= ~0b0001;

				for (delay = 0 ; delay < 400000 ; delay++);

				stok_25=20;
				stok_50=20;
				stok_100=10;
				urun_stok[0]=30;
				urun_stok[1]=20;
				urun_stok[2]=15;
				urun_stok[3]=50;
				urun_stok[4]=100;
				break;
			}else{
				GPIO_PORTD_DATA_R |= 0b0001;
				GPIO_PORTD_DATA_R &= ~0b0010;

				for (delay = 0 ; delay <400000 ; delay++);

			}
			UARTprintf("Toplam buton : %d\n", toplam);
			//UARTprintf("Girilen para : %f\n", kalan_para);
			//UARTprintf("Toplam para : %f\n", toplam_para);


			UARTprintf("sayac 0.25 : %d\n", sayac_25);
			UARTprintf("sayac 0.50 : %d\n", sayac_50);
			UARTprintf("sayac 1 : %d\n", sayac_100);

			UARTprintf("stok 0.25 : %d\n", stok_25);
			UARTprintf("stok 0.50 : %d\n", stok_50);
			UARTprintf("stok 1 : %d\n", stok_100);

			UARTprintf("Su : %d\n", urun_stok[0]);
			UARTprintf("Cay : %d\n", urun_stok[1]);
			UARTprintf("Kahve : %d\n", urun_stok[2]);
			UARTprintf("Cikolata : %d\n", urun_stok[3]);
			UARTprintf("Biskuvi : %d\n", urun_stok[4]);

		}
		}
	}


