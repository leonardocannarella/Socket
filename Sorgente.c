#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> //For socket, recvfrom
#include <sys/types.h> //For recvfrom
#include <netinet/if_ether.h>  //For ETH_P_ALL
#include <arpa/inet.h>	//For htons

#define MAX_PACKET_SIZE  65536

struct my_machdr {
	unsigned char dest[6];
	unsigned char source[6];
	union {
		unsigned char type_or_len[2];
		unsigned short type_or_len_value;
	};
};

unsigned char *cast(unsigned char *mac, unsigned char *result);
void printPacket(unsigned char *s, int size);

unsigned char *cast(unsigned char *mac, unsigned char *result){
	//Is broadcast?
	if (mac[0] & 255 == 255 && 
			mac[1] & 255 == 255 &&
			mac[2] & 255 == 255 &&
			mac[3] & 255 == 255 &&
			mac[4] & 255 == 255 &&
			mac[5] & 255 == 255){ 
		strcpy(result,"BROADCAST");

	}else if ( (mac[0] & 1) == 0) {
		strcpy(result,"SINGLE");
	}else if ( (mac[0] & 3)  == 3) { // MULTICAST 
		strcpy(result,"MULTICAST GRUPPO LOCALE");
	}else
		strcpy(result,"MULTICAST GRUPPO UNVERSALE");
	return result;	
}

void printPacket(unsigned char *s, int size){
	int p=0,i=0,pp=0, conta=1;
	for (i=0;i<size; i++) {
		if (conta==1) 
			printf("\t");
		printf("%02X ", s[i]);
		if (++conta==16){
			printf("\t\t");
			while (p < i) {
				if (s[p] >=32 && s[p] < 128)
					printf("%c ", s[p]);
				else
					printf(".");
				p++;
			}
			printf("\n");
			p = i;
			conta=1;
		}
	}
}

int main(int argc, char argv[]) {
	int sock_r;
	int i=0;
	char tmpbuf[80];
	sock_r=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	//setsockopt(sock_r , SOL_SOCKET , SO_BINDTODEVICE , "enp1s0" , strlen("enp1s0")+ 1 );
	if(sock_r<0)
	{
		printf("error in socket\n");
		return -1;
	}

	unsigned char *buffer = (unsigned char *) malloc(MAX_PACKET_SIZE); //to receive data
	
	int buflen;

	char tmp[80];
	
	while( i>=0){	
		//Receive a network packet and copy in to buffer
		memset(buffer,0,MAX_PACKET_SIZE);
		buflen=recv(sock_r,buffer,MAX_PACKET_SIZE,0);
		if(buflen<0)
		{
			printf("error in reading recvfrom function\n");
			return -1;
		}
		
		struct my_machdr *mac = (struct my_machdr *)(buffer);
		                
		sprintf(tmpbuf, "%02X-%02X-%02X-%02X-%02X-%02X", mac->dest[0],mac->dest[1],mac->dest[2],mac->dest[3],mac->dest[4],mac->dest[5]);
		if ( (strcmp("FF-FF-FF-FF-FF-FF",tmpbuf)) == 0) 
		{		// IF di controllo dell'indirizzo MAC; aggiornare o commentare il comando
			printf("\n\n**************** Pacchetto N. %d\n\n", i);
			printf("\n***Ethernet Header\n");
			printf("\t|-Destination Address :\t%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \t%s\n", mac->dest[0],mac->dest[1],mac->dest[2],mac->dest[3],mac->dest[4],mac->dest[5], cast(mac->dest,tmp) );
			printf("\t|-Source Address :\t%.2X-%.2X-%.2X-%.2X-%.2X-%.2X\t%s\n", mac->source[0],mac->source[1],mac->source[2],mac->source[3],mac->source[4],mac->source[5], cast(mac->source,tmp) );
			printf("\t|-Protocol/Type :\t%.2X%.2Xh ---> valore decimale  %d\n",mac->type_or_len[0], mac->type_or_len[1], htons(mac->type_or_len_value));
			if (htons(mac->type_or_len_value) > 1500) {
				printf("\t|-Type : Ethernet 2\n");
				printf("\t|-Protocol : %.2Xh\n", htons(mac->type_or_len_value));
			}else{
				printf("\t|-Type : 802.3\n");
			}
			printf("\nData\n");
			printPacket(buffer, buflen);
			printf("\n\n\n");
			i++;
		}
	}

	printf ("Success\n");
	return 0;
}
