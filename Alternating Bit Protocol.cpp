#include "../include/simulator.h"
#include<stdio.h>
#include <string.h>
#include<stdlib.h>
#include<vector>
//#include "../include/simulator.h"
using namespace std;

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
 
 This code should be used for PA2, unidirectional data transfer
 protocols (from A to B). Network properties:
 - one way network delay averages five time units (longer if there
 are other messages in the channel for GBN), but can be larger
 - packets can be corrupted (either the header or the data portion)
 or lost, according to user-defined probabilities
 - packets will be delivered in the order in which they were sent
 (although some can be lost).
 **********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */

#define BUFFER 50
#define TIMER 20
bool *a_state = new bool(false);
bool *b_state = new bool(false);
bool *wait_for_ack = new bool(false); 
struct pkt data_ack;
struct pkt data;
void change_state(bool *state);
struct pkt make_pkt(struct msg message,int seqnum);
struct pkt make_pkt_ack(struct pkt packet, int acknum);
struct pkt sndpkt;
int checksum(struct pkt packet);
int checksumData(char* data, const int size);
int n;
struct pkt message_buffer[BUFFER];
int *no_of_incoming_msgs = 0;
int last_sent_packet = 0;

struct msg_counter
{
	char data[20];
	int msg_to_app_layer;
};



std::vector<struct pkt> msg_buf;
std::vector<struct pkt>::iterator it;
std::vector<struct pkt> recv_buf;
  


void A_output(struct msg message)
{
  
    printf("Add to buffer\n"); 
    sndpkt = make_pkt(message,0);
    printf("Incoming message added to buffer %s\n", (sndpkt).payload);
    msg_buf.push_back(sndpkt);


    if(*wait_for_ack == true )
	printf("\n Waiting for ack");
    else if(*wait_for_ack == false)
    {
	printf("A Sending message: %s from state %d\n", msg_buf.at(0).payload, *a_state);
	msg_buf.at(0).seqnum = *a_state;
	msg_buf.at(0).checksum = checksum(msg_buf.at(0));
	starttimer (0, TIMER);
        tolayer3 (0, msg_buf.front());     
	*wait_for_ack = true;
	printf("A:Here\n");
	
	   
    }
  
	
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    char data[20];
    strncpy(data,packet.payload,20);
    int a_checksum = checksum(packet);
    printf("A_input, a_chechksum %d, packet_checksum %d\n", a_checksum, packet.checksum);
       
        
        if(packet.acknum == *a_state && (a_checksum == packet.checksum))
        {
	    printf("A received ack %d \n", packet.acknum);
            stoptimer(0);
	    printf(" A stopped timer \n");
	    *wait_for_ack = false;
	    change_state(a_state);
	    
	   if(msg_buf.empty() == false)
	  	{  printf("Deleting first from buffer %s\n", msg_buf.at(0).payload); 
			msg_buf.erase(msg_buf.begin());
		}
		
		if(msg_buf.empty() == false)
		{
			printf("A Sending message from A_input: %s,state %d\n", msg_buf.at(0).payload, *a_state);
			msg_buf.at(0).seqnum = *a_state;
			msg_buf.at(0).checksum = checksum(msg_buf.at(0));
        		tolayer3 (0, msg_buf.front());
        		starttimer (0, TIMER);
			*wait_for_ack = true;
			
		}	
		
	}  	
	else
	{
		printf("A Sending message again: %s , state %d \n", msg_buf.at(0).payload, *a_state);
		msg_buf.at(0).seqnum = *a_state;
		msg_buf.at(0).checksum = checksum(msg_buf.at(0));
        	tolayer3 (0, msg_buf.front());
		stoptimer(0);
        	starttimer (0, TIMER);
		*wait_for_ack = true;
		
	}

	
		
}


/* called when A's timer goes off */
void A_timerinterrupt()
{
    printf("Retransmission \n");
     if(msg_buf.empty() == false)
{
	printf("Sending msg %s , *a_state %d\n", msg_buf.at(0).payload, *a_state);
	msg_buf.at(0).seqnum = *a_state;
	msg_buf.at(0).checksum = checksum(msg_buf.at(0));
    tolayer3(0, msg_buf.front());
    starttimer(0, TIMER);
	*wait_for_ack = true;
}

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it tseo do any initialization */
void A_init()
{
 	*a_state = false;   
	
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    char data[20];
    int data_checksum;
	
	
    strncpy(data,packet.payload,20);
    
    data_checksum = checksum(packet);
    struct pkt ack;
    *wait_for_ack = true;
	printf("B:Here\n");
   
   	 
	printf("checksum data %d, packet checksun %d \n ",data_checksum, packet.checksum);
	
        printf("Packet seqnum %d, *b_state %d\n", packet.seqnum , *b_state);
	
        if(data_checksum == packet.checksum && packet.seqnum == *b_state)
        {	
	    printf("B In state b %d, ack = %d \n",*b_state, packet.acknum);
            ack = make_pkt_ack(packet, *b_state);  
            tolayer5 (1, packet.payload);
	    printf("Ack details : ack.seqnum %d , ack.acknum %d , ack.payload %s, ack.checksum %d\n", (ack).seqnum, (ack).acknum, (ack).payload, (ack).checksum);
	    change_state(b_state);
            tolayer3 (1, ack);
    
        }
        else
        {   printf("B Here In state b = %d, ack = opposite \n",*b_state);
	   // int n;
		if(*b_state == 0) n = 1;
		else
		n = 0;
            ack = make_pkt_ack(packet, n);
            tolayer3 (1, ack);
        }
        
        
    
    
        
        
    
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
   *b_state = false;
	n= 0;
}




void change_state(bool *state)
{
        if(*state == false)
            *state = true;
        else
            *state = false;
}


struct pkt make_pkt(struct msg message, int seqnum)
{
    //struct pkt *data = (struct pkt *)malloc(sizeof(struct pkt));
    printf("Making packet");
    (data).acknum = 2;
    (data).seqnum = seqnum;
    //printf("IN make packet, packet lenght %d packet data %s" ,int(strlen(message.data)), message.data);
     strncpy((data).payload,message.data,20);
	printf("Data is %s\n",data.payload);
    (data).checksum = checksum(data);
//printf("data.cheksum %d  \n", (*data).checksum);
   
    
    return data;
}

struct pkt make_pkt_ack(struct pkt packet, int acknum)
{
     
    // printf("Making packet");
    (data_ack).acknum = acknum;
    (data_ack).seqnum = packet.seqnum;
   // printf("IN make packet ack, packet lenght %d packet data %s" ,int(strlen(packet.payload)), packet.payload);
     strncpy((data_ack).payload,packet.payload,20);
	printf("Checksum calculation\n");
    (data_ack).checksum = checksum(data_ack);
	//printf("data.cheksum %d  \n", (*data).checksum);
   
    
    return data_ack;
}


int checksum(struct pkt packet)
{
    int sum = 0;
    char *s = new char[sizeof(packet.payload)];
    strncpy(s,packet.payload,20);
//while (*s != 0)

for(int i = 0; i<20 && *s != 0 ; i++)    
    {
        sum += *s;
        s++;
    }
   // printf ("%d", atoi(sum));
   sum += packet.acknum;
   sum += packet.seqnum;
   return sum;
}


int checksumData(char* data, const int size)
{
    int sum = 0;
    int l = size;
    while (l != 0)
    {
        sum ^= *data;
        data++;
        l--;
    }
	// printf("checksum %d",sum);
	//int a = sum - 48;
	//printf("int %d", a);
    return sum;
}