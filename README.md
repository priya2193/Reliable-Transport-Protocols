# Reliable-Transport-Protocols

### 1. Objectives

In a given simulator, implement three reliable data transport protocols: Alternating-Bit (ABT), Go-Back-N (GBN), and Selective-Repeat (SR).

### 2. Implementation

**2.1 Programming environment**

You will write C (or C++) code that compiles under the GCC (GNU Compiler Collection) environment. Furthermore, you should ensure that your code compiles and operates correctly on the ONE CSE server assigned to you (see section 3.2).
You should NOT use any CSE server other than the one assigned to you.
Your code should successfully compile using the version of gcc (for C code) or g++ (for C++ code) found on the CSE servers and should function correctly when executed.

**2.2 CSE Servers**

You will receive your server assignment for PA2 in an e-mail from the course staff. If you do not receive this e-mail within the first week of PA2 release, contact us.
For the purpose of this assignment, you should only use (for development and/or testing) the directory created for you on the assigned server. Change the access permission to this directory so that only you are allowed to access the contents of the directory. This is to prevent others from getting access to your code.

**2.3 Overview**

In this programming assignment, you will be writing the sending and receiving transport-layer code for implementing a simple reliable data transfer protocol. There are 3 versions of this assignment, the Alternating-Bit Protocol version, the Go-Back-N version, and the Selective-Repeat version.
Since we don't have standalone machines (with an OS that you can modify), your code will have to execute in a simulated hardware/software environment. However, the programming interface provided to your routines, i.e., the code that would call your entities from above and from below is very close to what is done in an actual UNIX environment. Stopping/starting of timers is also simulated, and timer interrupts will cause your timer handling routine to be activated.

**2.4 The routines you will write**

The procedures you will write are for the sending entity (A) and the receiving entity (B). Only unidirectional transfer of data (from A to B) is required. Of course, the B side will have to send packets to A to acknowledge receipt of data. Your routines are to be implemented in the form of the procedures described below. These procedures will be called by (and will call) procedures which simulate a network environment. The overall structure of the environment is shown below:


   ![Alt Text](https://lh3.googleusercontent.com/NAuASh68hDcVbTZmIDNBq1tqTq2lGKc28Apr8vThOY_Zj4xJOyASoF3weORrN1wOImuKDlOCeUL-hBZasECseIkoSLA3rW5XDEVLe0KIAaYOqH9Ujr9jeUEFEU5keh-IhAvlrfcr)


The unit of data passed between the upper layers and your protocols is a message, which is declared as:

struct msg {
char data[20];
};

This declaration, and all other data structures and simulator routines, as well as stub routines (i.e., those you are to complete) are inside the template files, described later. Your sending entity will thus receive data in 20-byte chunks from Layer 5; your receiving entity should deliver 20-byte chunks of correctly received data to Layer 5 at the receiving side.
The unit of data passed between your routines and the network layer is the packet, which is declared as:

struct pkt {
int seqnum;
int acknum;
int checksum;
char payload[20];
};

Your routines will fill in the payload field from the message data passed down from Layer 5. The other packet fields will be used by your protocols to insure reliable delivery, as we've seen in class.

The routines you will write are detailed below. As noted above, such procedures in real-life would be part of the operating system, and would be called by other procedures in the operating system.

**A_output (message)**

where message is a structure of type msg, containing data to be sent to the B-side. This routine will be called whenever the upper layer at the sending side (A) has a message to send. It is the job of your protocol to insure that the data in such a message is delivered in-order, and correctly, to the receiving side upper layer.

**A_input(packet)**

where packet is a structure of type pkt. This routine will be called whenever a packet sent from the B-side (as a result of a tolayer3() (see section 3.5) being called by a B-side procedure) arrives at the A-side. packet is the (possibly corrupted) packet sent from the B-side.

**A_timerinterrupt()**

This routine will be called when A's timer expires (thus generating a timer interrupt). You'll probably want to use this routine to control the retransmission of packets. See starttimer() and stoptimer() below for how the timer is started and stopped.

**A_init()**

This routine will be called once, before any of your other A-side routines are called. It can be used to do any required initialization.

**B_input(packet)**

where packet is a structure of type pkt. This routine will be called whenever a packet sent from the A-side (as a result of a tolayer3() (see section 3.5) being called by a A-side procedure) arrives at the B-side. packet is the (possibly corrupted) packet sent from the A-side.

**B_init()**

This routine will be called once, before any of your other B-side routines are called. It can be used to do any required initialization.
These six routines are where you can implement your protocols.
You are not allowed to modify any other routines.

**2.5 Software Interfaces**

The procedures described above are the ones that you will write. We have written the following routines which can be called by your routines:

**starttimer (calling_entity, increment)**

where calling_entity is either 0 (for starting the A-side timer) or 1 (for starting the B side timer), and increment is a float value indicating the amount of time that will pass before the timer interrupts. A's timer should only be started (or stopped) by A-side routines, and similarly for the B-side timer. To give you an idea of the appropriate increment value to use: a packet sent into the network takes an average of 5 time units to arrive at the other side when there are no other messages in the medium.

**stoptimer (calling_entity)**

where calling_entity is either 0 (for stopping the A-side timer) or 1 (for stopping the B side timer).
tolayer3 (calling_entity, packet)
where calling_entity is either 0 (for the A-side send) or 1 (for the B side send), and packet is a structure of type pkt. Calling this routine will cause the packet to be sent into the network, destined for the other entity.
tolayer5 (calling_entity, data)
where calling_entity is either 0 (for A-side delivery to layer 5) or 1 (for B-side delivery to layer 5), and data is a char array of size 20. With unidirectional data transfer, you would only be calling this with calling_entity equal to 1 (delivery to the B-side). Calling this routine will cause data to be passed up to layer 5.

**getwinsize()**

returns the window size value passed as parameter to -w (see section 3.6).

**get_sim_time()**

returns the current simulation time.

**2.6 The simulated network environment**

A call to procedure tolayer3() sends packets into the medium (i.e., into the network layer). Your procedures A_input() and B_input() are called when a packet is to be delivered from the medium to your transport protocol layer.

The medium is capable of corrupting and losing packets. However, it will not reorder packets. When you compile your procedures and our procedures together and run the resulting program, you will be asked to specify certain values regarding the simulated network environment as command-line arguments. We describe them below:

**Seed (-s)**

The simulator uses some random numbers to reproduce random behavior that a real network usually exhibits. The seed value (a non-zero positive integer) initializes the random number generator. Different seed values will make the simulator behave slightly differently and result in different output values.

**Window size (-w)**

This only applies to Go-back-N and Selective-Repeat binaries. Both these protocols use a finite-sized window to function. You need to tell the simulator before hand, what window size you want to use. Infact, your code will internally use this value for 
implementing the protocols.

**Number of messages to simulate (-m)**

The simulator (and your routines) will stop as soon as this number of messages has been passed down from Layer 5, regardless of whether or not all of the messages have been correctly delivered. Thus, you need not worry about undelivered or unACK'ed messages still in your sender when the simulator stops. This value should always be greater than 1. If you set this value to 1, your program will terminate immediately, before the message is delivered to the other side.

**Loss (-l)**

Specify a packet loss probability [0.0,1.0]. A value of 0.1, for example, would mean that one in ten packets (on average) are lost.

**Corruption (-c)**

You are asked to specify a packet corruption probability [0.0,1.0]. A value of 0.2, for example, would mean that one in five packets (on average) are corrupted. Note that the contents of payload, sequence, ack, or checksum fields can be corrupted. Your checksum should thus include the data, sequence, and ack fields.

**Average time between messages from sender's layer5 (-t)**

You can set this value to any non-zero, positive value. Note that the smaller the value you choose, the faster packets will be arriving to your sender.

**Tracing (-v)**

Setting a tracing value of 1 or 2 will print out useful information about what is going on inside the simulation (e.g., what's happening to packets and timers). A tracing value of 0 will turn this off. A tracing value greater than 2 will display all sorts of odd messages that are for our own simulator-debugging purposes. A tracing value of 2 may be helpful to you in debugging your code. You should keep in mind that, in reality, you would not have underlying networks that provide such nice information about what is going to happen to your packets!

### 3. Protocols

**3.1 Alternating-Bit-Protocol (ABT)**

You are to write the procedures which together will implement a stop-and-wait (i.e., the alternating bit protocol, which is referred to as rdt3.0) unidirectional transfer of data from the A-side to the B-side. Your protocol should use only ACK messages.
You should perform a check in your sender to make sure that when A_output() is called, there is no message currently in transit. If there is, you should buffer the data being passed to the A_output() routine.

**3.2 Go-Back-N (GBN)**

You are to write the procedures which together will implement a Go-Back-N unidirectional transfer of data from the A-side to the B-side, with a certain window size.
It is recommended that you first implement the easier protocol (the Alternating-Bit version) and then extend your code to implement the more difficult protocol (the Go-Back-N version). Some new considerations for your GBN code (which do not apply to ABT) are:
A_output()
will now sometimes be called when there are outstanding, unacknowledged messages in the medium, implying that you will have again to buffer multiple messages in your sender. Also, you'll need buffering in your sender because of the nature of Go-Back-N: sometimes your sender will be called but it won't be able to send the new message because the new message falls outside of the window.
Rather than have you worry about buffering an arbitrary number of messages, it will be OK for you to have some finite, maximum number of buffers available at your sender (say for 1000 messages) and have your sender simply abort (give up and exit) should all 1000 buffers be in use at one point (Note: If the buffer size is not enough in your experiments, set it to a larger value). In the “real-world”, of course, one would have to come up with a more elegant solution to the finite buffer problem!

**3.3 Selective-Repeat (SR)**

You are to write the procedures which together will implement a Selective-Repeat unidirectional transfer of data from the A-side to the B-side, with a certain window size.
It is recommended that you implement the GBN protocol before you extend your code to implement SR. Some new considerations for your SR code are:
B_input(packet)
will have to buffer multiple messages in your receiver because of the nature of Selective-Repeat. The receiver should reply with ACKs to all packets falling inside the receiving window.
A_timerinterrupt()
will be called when A's timer expires (thus generating a timer interrupt).
Even though the protocol uses multiple logical timers, remember that you've only got one hardware timer, and may have many outstanding, unacknowledged packets in the medium. You will have to think about how to use this single timer to implement multiple logical timers. Note that an implementation that simply sets a timer every T time units and retransmits all the packets that should have expired within those time units is NOT acceptable. Your implementation has to ensure that each packet is retransmitted at the exact time at which it would be retransmitted if you had multiple timers.
