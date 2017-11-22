#include "../include/simulator.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
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

#define TIMER 30
struct in_pkt{
	struct pkt packet;
	int rel_timeout;
	int abs_timeout;
	int start_time;
	int sent;
	};

std::vector<struct in_pkt> msg_buf;
std::vector<struct in_pktt>::iterator it;
std::vector<struct pkt> recv_buf;

struct in_pkt sndpkt[1001];
struct pkt sndpkt_ack[2];
struct pkt recv_buffer[1000];

int N;
int *base = new int(0), *next_seq_no = new int(0) , *expected_seq_no = new int(0),  *recv_base = new int(0), *counter = new int(0);
struct pkt data_ack;
struct in_pkt ack;
struct in_pkt make_pkt(struct msg message, int seqnum);
int checksum(struct pkt packet);
int iteration;
struct pkt make_pkt_ack(struct msg message, int seqnum);

	
void A_output(struct msg message)
{
		if(*next_seq_no < (*(base) + N))
		{
			sndpkt[*next_seq_no] = make_pkt(message,*next_seq_no);
			printf("From A: Next seq num is %d\n", *next_seq_no);
			tolayer3 (0,sndpkt[*next_seq_no].packet);
			sndpkt[*next_seq_no].sent = 1;
        		printf("A sent seqnum %d, acknum %d , checksum %d, data %s start time %d abs_timeout %d rel_timeout %d\n",sndpkt[*next_seq_no].packet.seqnum, sndpkt[*next_seq_no].packet.acknum, sndpkt[*next_seq_no].packet.checksum, sndpkt[*next_seq_no].packet.payload, sndpkt[*next_seq_no].start_time, sndpkt[*next_seq_no].abs_timeout, sndpkt[*next_seq_no].rel_timeout);
			msg_buf.push_back(sndpkt[*next_seq_no]);
			(*next_seq_no)++;
			
			if(iteration == 1)
			{
				starttimer(0, TIMER);
				iteration+=1;
			}
	
		}
		else
		{
			printf("A:Adding to buffer\n");
			sndpkt[*next_seq_no] = make_pkt(message,*next_seq_no);
			msg_buf.push_back(sndpkt[*next_seq_no]);
			(*next_seq_no)++;
		}
		printf("Base %d, seqnum %d\n",*base, *next_seq_no);
		if(iteration != 1 && *base == (*next_seq_no - 1))
		{ printf("Starting timer when base == seqnum\n");
		starttimer(0,TIMER);}
		
		
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	int arecv_checksum = checksum(packet);
	//if(packet.seqnum == 0)
	//stoptimer(1);
	if(arecv_checksum == packet.checksum)
	{
		if(packet.seqnum < (*base) + N)
		{
			printf("Packet.seqnum %d Packet acknum %d Base = %d\n",packet.seqnum,packet.acknum,*base);
			if(*base == packet.acknum)
			{
				sndpkt[*base].packet.acknum = packet.acknum;
				*base = packet.acknum + 1;	
				stoptimer(0);
				//if(sndpkt[*base].packet.acknum != -1)
				//starttimer(0,sndpkt[*base].rel_timeout);
				//if(msg_buf.at((*base)+N).empty() == false)
				printf("Updated base %d\n",*base);
				for(int i = (*base)+N, j= *base; j <=*next_seq_no ; i++, j++)
				{	if(sndpkt[(*base)+N].sent != 1 && (*base)+N < *next_seq_no)
					{
						tolayer3(0,sndpkt[i].packet);
						sndpkt[i].sent = 1;
						sndpkt[i].start_time = get_sim_time();
						sndpkt[i].rel_timeout = sndpkt[(*base)].start_time - sndpkt[i].start_time;
						printf("Sending data %s when base equals acknum\n",sndpkt[i].packet.payload);
					}
					if(sndpkt[j].packet.acknum == -1)
					{
						int sim_time = get_sim_time();
						int remain_time = get_sim_time() - sndpkt[j].start_time;
						int time_val = TIMER - remain_time;
						printf("Sim time %d, remain time %d , time val %d\n",sim_time,remain_time,time_val);
						printf("In A_input starting timer for seqnum %d value %d\n",sndpkt[j].packet.seqnum, time_val);
						starttimer(0,time_val);
						break;
					}
				}
			}
			
			/*else
			{
				
				if(sndpkt[(*base)+N].packet.acknum == -1)
				sndpkt[packet.acknum].rel_timeout = -1;
				for(int i = *(base), j = 0; i<*next_seq_no, j< N; i++, j++)
				{
					if(sndpkt[i].packet.acknum == -1 && sndpkt[i].sent != 1)
					{
						printf("From A_input Sending Packet %s seqnum %d\n",sndpkt[i].packet.payload, sndpkt[i].packet.seqnum);
						sndpkt[i].start_time = get_sim_time();
						tolayer3(0,sndpkt[i].packet);
						sndpkt[i].sent = 1;
						sndpkt[i].rel_timeout = sndpkt[i].start_time - sndpkt[(*base)].start_time;
					}
				}
				
			}*/

		}
			
	}
	
}

/* called when A's timer goes off */
void A_timerinterrupt()
{

	printf("Timeout occurred for %d\n",*base);
	
	int loc = *base;
	int min = sndpkt[(*base)].rel_timeout;
	printf("Min %d\n",min);
	//if(min == 0)
	//starttimer(0,TIMER);
	//else{  && sndpkt[j].rel_timeout < min
	printf("Resending packet seqnum %d\n",sndpkt[*base].packet.seqnum);
	tolayer3(0,sndpkt[*base].packet);
	sndpkt[*base].start_time = get_sim_time();
	for(int i = 0, j = (*base); i< N, j< *next_seq_no; i++, j++)
	{
		if(sndpkt[j].rel_timeout != -1 && sndpkt[j].rel_timeout > 0)
		{
			min = sndpkt[j].rel_timeout;
			loc = j;
			//printf("Resending packet seqnum %d\n",sndpkt[j].packet.seqnum);
			//tolayer3(0,sndpkt[j].packet);
			//sndpkt[j].start_time = get_sim_time();
		}
	}
	printf("Starting timer for: seqnum %d\n",loc);
	int sim_time = get_sim_time();
	int remain_time = get_sim_time() - sndpkt[loc].start_time;
	int time_val = TIMER - remain_time;
	printf("Sim time %d, remain time %d , time val %d\n",sim_time,remain_time,time_val);
	if(time_val> 0)
	starttimer(0,time_val);
	else
	starttimer(0,TIMER);
	//}
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	*next_seq_no = 0;
	*base = 0;
	N = getwinsize();
	iteration = 1;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	char data[20];
    	int recv_checksum;
	
    	strncpy(data,packet.payload,20);
    //printf("payload %s data %s data length %d  packet seqnum %d \n",packet.payload, data, int(strlen(data)), packet.seqnum);
    	recv_checksum = checksum(packet);
    	printf("Expected_seq_no %d, recv_base %d packet seqnum %d\n",*expected_seq_no,*recv_base,packet.seqnum);
	if(*expected_seq_no < *recv_base + N)
	{
		if(recv_checksum == packet.checksum)		
		{
			printf("Adding to recv buffer at %d, message %s, packet acknum %d\n",packet.seqnum,packet.payload,packet.acknum);   
			struct msg message;
	    		strncpy(message.data, packet.payload,20);
			recv_buffer[packet.seqnum] = make_pkt_ack(message,packet.seqnum);
				//recv_buf.push_back(recv_packet);
			struct msg ack;
	    		strcpy(ack.data, "ACK");
	    		sndpkt_ack[1] = make_pkt_ack(ack,packet.seqnum);
	    		printf("Ack details : ack.seqnum %d , ack.acknum %d , ack.payload %s, ack.checksum %d\n", sndpkt_ack[1].seqnum, sndpkt_ack[1].acknum, sndpkt_ack[1].payload, sndpkt_ack[1].checksum);	
			tolayer3 (1, sndpkt_ack[1]);
			if(packet.seqnum == *expected_seq_no)
			{
				tolayer5 (1, packet.payload);
	   			printf("From B sent packet to app %s:\n", packet.payload);
	    			*recv_base += 1;
				(*expected_seq_no)++;
				recv_buffer[packet.seqnum].seqnum = *expected_seq_no - 1;
				//int count = 0;
				//int i = *recv_base;
				/*while(i != *recv_base + N)
				{	
					printf("Emptying recv_buf\n");
					//for(std::vector<struct pkt>::iterator it = recv_buf.begin(); it != recv_buf.end(); ++it) {
					//printf("Vector buf loop\n");
					if(recv_buf.at(i) != NULL){
					if(recv_buf.at(i).seqnum == (*expected_seq_no)){
					printf("Sending %s from recv_buf to app layer:\n",recv_buf.at(i).payload);
					tolayer5(1, recv_buf.at(i).payload);
					printf("Deleting first from recv_buffer%s\n", recv_buf.at(i).payload); 
					recv_buf.erase(recv_buf.at(i));
					(*expected_seq_no)++;
					//count +=1;
					}
					}
					//}
					//if(count == 0)
					//break;
					i++;
	 			}*/
				int j = 0;
				//while(i != (*recv_base) + N - 1 && j != *counter)
				for(j = *recv_base; j < (*recv_base) + N; j++)
				{
					
					printf("Emptying recv_buf\n");
					if(recv_buffer[j].acknum == (*expected_seq_no))
					{
						printf("Sending %s from recv_buf to app layer:\n",recv_buffer[j].payload);
						if(recv_buffer[j].seqnum == -1)
						tolayer5(1, recv_buffer[j].payload);
						recv_buffer[j].seqnum = *expected_seq_no;
						printf("Deleting first from recv_buffer%s\n", recv_buffer[j].payload);	
						(*expected_seq_no)++;
						(*recv_base) += 1;
					}
					//else
					//break;
					//i++; j++;
				}
	

	
			}
			/*else
			{
				//struct pkt recv_packet;
				printf("Adding to recv buffer at %d, message %s, packet acknum %d\n",packet.seqnum,packet.payload,packet.acknum);   
				struct msg message;
	    			strncpy(message.data, packet.payload,20);
				recv_buffer[packet.seqnum] = make_pkt_ack(message,packet.seqnum);
				//recv_buf.push_back(recv_packet);
				
				//(*expected_seq_no)++;
				
			}*/
		}
		
	}
		

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	*recv_base = 0;
	*expected_seq_no = 0;
	for(int k =0; k<1000; k++)
	recv_buffer[k].acknum = -1;
	*counter = 0;
}








int checksum(struct pkt packet)
{
    int sum = 0;
    char *s = new char[sizeof(packet.payload)];
   // printf("Payload size %d Payload %s, packet seqnum %d, packet acknum %d\n",sizeof(packet.payload), packet.payload, packet.seqnum, packet.acknum); 
    strncpy(s,packet.payload,20);
//for(int i = 0; i<20 && *s != 0 ; i++)  

    //while (*s != 0) 
    for(int i = 0; i<20 && *s != 0 ; i++) 	 
    {
        sum += *s;
        s++;
    }
   
   sum += packet.acknum;
   sum += packet.seqnum;
   return sum;
}


struct in_pkt make_pkt(struct msg message, int seqnum)
{
    
    // printf("Making packet");
  //  if(strcmp(message.data, "ACK") == 0){
	printf("I'm here\n");
    	//sndpkt[seqnum].packet.acknum = seqnum;}
    //else
	sndpkt[seqnum].packet.acknum = -1;
    sndpkt[seqnum].packet.seqnum = seqnum;
    strncpy(sndpkt[seqnum].packet.payload,message.data,20);

    sndpkt[seqnum].packet.checksum = checksum(sndpkt[seqnum].packet);
   // printf("data.cheksum %d  \n", sndpkt[seqnum].checksum);

    //printf("In make packet, packet length %d packet data %s, packet seqnum %d, packet acknum %d, packet checksum %d\n" ,int(strlen(sndpkt[seqnum].payload)), sndpkt[seqnum].payload, sndpkt[seqnum].seqnum, sndpkt[seqnum].acknum, sndpkt[seqnum].checksum);	
   
    sndpkt[seqnum].start_time = get_sim_time();
    sndpkt[seqnum].abs_timeout = TIMER;
    sndpkt[seqnum].rel_timeout = sndpkt[seqnum].start_time - sndpkt[0].start_time;

    return sndpkt[seqnum];
}



struct pkt make_pkt_ack(struct msg message, int seqnum)
{
     
    // printf("Making packet");
    (data_ack).acknum = seqnum;
    (data_ack).seqnum = -1;
   // printf("IN make packet ack, packet lenght %d packet data %s" ,int(strlen(packet.payload)), packet.payload);
     strncpy((data_ack).payload,message.data,20);
	printf("Checksum calculation\n");
    (data_ack).checksum = checksum(data_ack);
	//printf("data.cheksum %d  \n", (*data).checksum);
   
    
    return data_ack;
}