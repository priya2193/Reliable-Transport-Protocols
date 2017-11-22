#include "../include/simulator.h"
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<malloc.h>

#define BUFSIZE 9999

/* ******************************************************************
 *  *  *  *  ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
 *   *   *   *
 *    *    *    *     This code should be used for PA2, unidirectional data transfer
 *     *     *     *        protocols (from A to B). Network properties:
 *      *      *      *           - one way network delay averages five time units (longer if there
 *       *       *       *                are other messages in the channel for GBN), but can be larger
 *        *        *        *                   - packets can be corrupted (either the header or the data portion)
 *         *         *         *                        or lost, according to user-defined probabilities
 *          *          *          *                           - packets will be delivered in the order in which they were sent
 *           *           *           *                                (although some can be lost).
 *            *            *            *                                **********************************************************************/

int NextPosition;
int NextSeqNum=0;
int base;
int window_size;
float TIMEOUT;
int RequestedSeqNum;
int buffered=0;
int Seq_num=0;
int LastPacketSent=0;
float sample_rtt=0;
float estimated_rtt=0;
float difference=0;
float dev_rtt=0;
float time_sent[1500];
float Ack_received_at;
int No_of_Acks_received=0;


struct msg LastMessage;
struct pkt LastAck;

struct packetInfo
{
        struct pkt packet;
        bool isSent;
        bool isAcked;
};

struct packetInfo *PacketStored;

int checkSum(struct pkt packet)
{
        int i;
        int Sum = packet.acknum + packet.seqnum;
        for (i = 0; i < sizeof(packet.payload); i++)
        {
                Sum += packet.payload[i];
        }
        return Sum;
}
 
int check_checkSum(struct pkt packet)
{
        int i;
        int Sum = packet.acknum + packet.seqnum;
        for (i = 0; i < sizeof(packet.payload); i++)
        {
                Sum += packet.payload[i];
        }
        return Sum;

}
/*std::vector<packetInfo> BufferedPackets;*/


void timer_update(float x, float y)
{ float loss;
  float temp=TIMEOUT;
  loss=1-((float)No_of_Acks_received/(float)NextSeqNum);
  float a;
  if(loss>=0.8)
  {
    TIMEOUT=40;
    printf("TIMEOUT to 30\n");
	return;    
  }
  else if(loss>=0.6)
  {
    TIMEOUT=35;
    printf("TIMEOUT to 35\n");
   return; 
  }
  else if(loss>=0.4)
  {
    a=0.050;
  }
  else if(loss>=0.2)
  {
    a=0.040;
  }
  else 
  {
    a=0.030;
  }
  printf("Loss is %f", loss);
  sample_rtt=x-y;
  estimated_rtt=(a*estimated_rtt)+((1-a)*sample_rtt);  /** 2 zeros**/
  difference=abs(sample_rtt - estimated_rtt);
  dev_rtt=(0.75*dev_rtt)+(0.25*difference); /** 3  zeros **/
  if(loss<=0.6)
{
	TIMEOUT+=0.01*estimated_rtt+dev_rtt; /*0.004090*/
} 
 printf("TU: sample rtt %f, estimated_rtt %f, dev_rtt %f and old timeout is %f new timeout is %f\n", sample_rtt, estimated_rtt, dev_rtt,temp, TIMEOUT);
}

int corruptAck=0, InvalidPacket=0;

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
        /*printf("A: A_output called\n");*/
        struct pkt packet;
        packet.acknum = 0;
        packet.seqnum = Seq_num;
        strncpy(packet.payload, message.data, sizeof(packet.payload));
        packet.checksum = checkSum(packet);
        /*BufferedPackets.push_back(packet);*/
        PacketStored[NextPosition].packet = packet;
        PacketStored[NextPosition].isAcked = false;
        PacketStored[NextPosition].isSent = false;

        if (NextSeqNum < base + window_size)
        {
                /*int i;
 *  *  *  *              for(i=NextSeqNum;i<base+window_size-NextSeqNum && i<=NextPosition;i++)
 *   *   *   *                              {*/

                        if (NextSeqNum == base)
                        {
                                /*printf("AO: Base is equal to the sequence number\n");*/
                                starttimer(0, TIMEOUT);

                        }

                        if (PacketStored[NextSeqNum].isSent == false && PacketStored[NextSeqNum].isAcked == false)
                        {
                              /*  printf("AO: Sending packet with message %s, seqnum %d and checksum %d\n", PacketStored[NextSeqNum].packet.payload, PacketStored[NextSeqNum].packet.seqnum, PacketStored[NextSeqNum].packet.checksum);
 *   *   *                                  */
                                tolayer3(0, PacketStored[NextSeqNum].packet);
                                time_sent[NextSeqNum]=get_sim_time();
                                PacketStored[NextSeqNum].isSent = true;
                                LastPacketSent=NextSeqNum;
                        }
                /*}*/

        }

        else
        {
                /*printf("AO: Packet buffered with seqnum %d\n", PacketStored[NextPosition].packet.seqnum);*/

        }

        NextSeqNum++;
        NextPosition++;
        Seq_num++;/*=(Seq_num+1)%(window_size+1);*/
       /* printf("\n");*/
        return;

}

/* called from layer 3, when a packet arrives for layer 4 */

void A_input(struct pkt packet)
{       /*set isSet to true*/
        /*printf("A_input called \n");*/
        int y=check_checkSum(packet);
        int i;
        /*No_of_Acks_received++;*/
        /*printf("Received ack with %d and checksum %d\n", packet.acknum, y);*/
        if(packet.acknum>=base && y==packet.checksum)
        {       Ack_received_at=get_sim_time();
                No_of_Acks_received++;
                for(i=base;i<=packet.acknum;i++)
                {
                        PacketStored[i].isSent=true;
                        PacketStored[i].isAcked=false;
                }
                /*printf("AI: Packet received correctly\n");*/
                base=packet.acknum+1;
                /*printf("Base is now %d\n", base);*/
                if(base==NextSeqNum)
                {
                        /*printf("AI: Base == NextSeqNum. Stopping timer\n");*/
                        stoptimer(0);
                }
                else
                {
                        /*printf("AI: Restarting timer\n");*/
                        stoptimer(0);
                        starttimer(0,TIMEOUT);
                }


  /*printf("AI: NextSeqNum is %d and NextPosition is %d and LastPacketSent is %d. Sending buffered packets \n", NextSeqNum, NextPosition, LastPacketSent);*/
                for(int i=LastPacketSent;i<base+window_size && i<NextPosition;++i)
                {
                        if(PacketStored[i].isSent==false && PacketStored[i].isAcked==false)
                        {
                               /* printf("AI: Sending packet with seqnum %d, message %s and checksum %d\n",PacketStored[i].packet.seqnum, PacketStored[i].packet.payload, PacketStored[i].packet.checksum);
 *  *  *                                 */
tolayer3(0,PacketStored[i].packet);
                                LastPacketSent=i;
                                PacketStored[i].isSent=true;
                                time_sent[PacketStored[i].packet.seqnum]=get_sim_time();
                                if(i==base)
                                {
                                        starttimer(0,TIMEOUT);
                                }
                                NextSeqNum++;
                                buffered--;
                        }

                }
                if(No_of_Acks_received%10==0)
                {
                    timer_update(Ack_received_at, time_sent[packet.acknum]);
                }
        }

        else
        {
                      /*  printf("AI: Corrupted Ack \n");
 *   *   *                        */
                        corruptAck++;
        }

       /* printf("\n \n \n");*/
        return;
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
        int i;
        /*printf("Timer interrupted\n");*/
        for(i=base;i<=NextSeqNum && PacketStored[i].isSent==true && PacketStored[i].isAcked==false;i++)
        {
               /* printf("AT: Retransmitting packets with seqnum %d, message %s and checksum %d\n", PacketStored[i].packet.seqnum, PacketStored[i].packet.payload, PacketStored[i].packet.checksum);
 *  *  *                 */
                tolayer3(0,PacketStored[i].packet);
                time_sent[PacketStored[i].packet.seqnum]=get_sim_time(); /*-----------------------THIS LINE -------------------*/
                LastPacketSent=i;
        }
      /*  printf("\n \n \n");*/
        starttimer(0,TIMEOUT);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
        NextSeqNum=0;
        NextPosition=0;
        base=0;
        window_size=getwinsize();
        if(window_size<=50)
       {TIMEOUT=11.5; /* was 30, 25*/
        }
    else if(window_size<=100)
        {TIMEOUT=15;
        }
    else if(window_size<=150)
        {TIMEOUT=18;
        }
    else if(window_size<=200)
        {TIMEOUT=20;
        }
    else if(window_size<=500)
        {TIMEOUT=23;
        }
    else
        {TIMEOUT=25;
        }

        PacketStored=(struct packetInfo*) malloc(BUFSIZE*sizeof(struct packetInfo));
        /*TIMEOUT=5.0;*/
}
/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/


void B_input(struct pkt packet)
{
        int y=check_checkSum(packet);
        /*printf("BI: Packet seqnum is %d and RequestedSeqNum is %d\n", packet.seqnum, RequestedSeqNum);*/
        if(packet.seqnum==RequestedSeqNum && y==packet.checksum)
        {
                /*printf("BI: Packet received correctly with seqnum %d, message %s and checksum %d\n",packet.seqnum, packet.payload, y);*/
                tolayer5(1,packet.payload);
                struct pkt Ack;
                Ack.seqnum=RequestedSeqNum;
                Ack.acknum=RequestedSeqNum;
                strncpy(Ack.payload,packet.payload,sizeof(packet.payload));
                Ack.checksum=checkSum(Ack);
                LastAck=Ack;
                /*printf("BI: Sending ack with acknum %d and checksum %d\n",Ack.seqnum, Ack.checksum);*/
                tolayer3(1,Ack);
                RequestedSeqNum++;
        }

        else
        {

                if(LastAck.acknum!=-1)
                {
                /*printf("BI: Invalid Packet with checksum %d. Sending last saved Ack with checksum %d\n", y, LastAck.checksum);*/
                tolayer3(1,LastAck);
                }
                if(LastAck.acknum==-1)
                {
                   /* printf("BI: Invalid Packet with checksum %d. Sending last saved Ack with checksum %d\n", y, LastAck.checksum);*/
                }
                InvalidPacket++;
        }
      /*  printf("\n \n \n");*/
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
        RequestedSeqNum=0;
        LastAck.acknum=-1;
        /*LastAck.seqnum=-1;*/

}