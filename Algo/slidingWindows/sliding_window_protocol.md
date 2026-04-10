# Sliding Window Protocol

## What is Sliding Window Protocol?

The **Sliding Window Protocol** is a method used in computer networks to control the flow of data between a sender and receiver. Think of it like a smart delivery system that ensures packages are sent efficiently and reliably.

## Simple Analogy

Imagine you're sending boxes through a conveyor belt to your friend:
- You can't send ALL boxes at once (would overflow)
- You can't send ONE box and wait for confirmation before sending the next (too slow)
- **Solution**: Send a few boxes at a time and slide the "window" forward as confirmations come back

## How It Works

### The "Window"

The window is like a **moving frame** that shows which packets can be sent/received at any moment.

```
Sender's perspective:
[1][2][3][4][5][6][7][8][9][10]
 ^-----^
 Window (can send packets 1-3)
```

### Step-by-Step Process

1. **Initial State**: Sender has window of size 3
   ```
   Packets: [1][2][3][4][5][6][7][8][9][10]
   Window:   |===|
             Send these
   ```

2. **Sending**: Sender sends packets 1, 2, and 3
   ```
   Sent: 1, 2, 3 → [waiting for ACK]
   ```

3. **Acknowledgment**: Receiver sends ACK for packet 1
   ```
   Packets: [1][2][3][4][5][6][7][8][9][10]
   Window:      |===|
                Slide forward! Now can send 4
   ```

4. **Sliding**: Window "slides" forward, can now send packet 4
   ```
   Packets: [✓][2][3][4][5][6][7][8][9][10]
   Window:      |===|
   ```

## Key Concepts

### 1. Window Size
- **Small window**: Slower but uses less memory
- **Large window**: Faster but uses more memory
- Typical size: 1 to thousands of packets

### 2. Sequence Numbers
Each packet gets a number (like a tracking ID):
```
Packet #1 → Data: "Hello"
Packet #2 → Data: "World"
Packet #3 → Data: "!"
```

### 3. Acknowledgments (ACK)
Receiver says "Got it!" for each packet received:
```
Sender: Sends packet #5
Receiver: Sends ACK #5 (received successfully)
```

## Types of Sliding Window Protocols

### 1. **Go-Back-N**
- If packet 5 is lost, resend packets 5, 6, 7, 8... (everything after the lost one)
- Like rewinding and replaying from the error point

```
Sent: [1][2][3][4][5][6][7]
Lost:             ❌
Action: Resend → [5][6][7]
```

### 2. **Selective Repeat**
- Only resend the lost packet (more efficient)
- Like only replacing the broken box

```
Sent: [1][2][3][4][5][6][7]
Lost:             ❌
Action: Resend → [5] only
```

## Real-World Example

### Sending a Video File

Without Sliding Window:
```
Send frame 1 → Wait for ACK → Send frame 2 → Wait for ACK...
(Very slow! ⏰)
```

With Sliding Window (size 5):
```
Send frames 1,2,3,4,5 → Get ACKs → Slide → Send 6,7,8,9,10...
(Much faster! ⚡)
```

## Why Use Sliding Window?

### Advantages:
1. ✅ **Efficient**: Multiple packets in transit at once
2. ✅ **Reliable**: Detects and recovers from lost packets
3. ✅ **Flow Control**: Prevents overwhelming the receiver
4. ✅ **Better Network Utilization**: Keeps the pipe full

### Without Sliding Window:
```
Sender: Send packet → WAIT → WAIT → Get ACK → Send next
Network: |packet|.............|packet|.............
         (Wasted bandwidth!)
```

### With Sliding Window:
```
Sender: Send multiple packets → Process ACKs → Send more
Network: |p1|p2|p3|p4|p5|p6|p7|p8|p9|p10|
         (Fully utilized!)
```

## Common Use Cases

1. **TCP Protocol**: Uses sliding window for reliable data transfer
2. **File Transfers**: Sending large files efficiently
3. **Video Streaming**: Continuous data flow
4. **Network Protocols**: HDLC, PPP, etc.

## Simple Implementation Concept

```cpp
// Pseudocode for Sender
int window_size = 5;
int base = 0;  // First unacknowledged packet
int next_seq = 0;  // Next packet to send

while (data_to_send) {
    // Send packets within window
    while (next_seq < base + window_size) {
        send_packet(next_seq);
        next_seq++;
    }
    
    // Wait for ACK
    if (received_ack(ack_num)) {
        base = ack_num + 1;  // Slide window
    }
}
```

## Key Takeaways

1. **Window = Group of packets** that can be sent without waiting for ACK
2. **Sliding = Moving forward** as packets are acknowledged
3. **Efficiency = Speed + Reliability** combined
4. **Used in TCP** and many network protocols

## Visual Summary

```
Time →
        Window Size = 3
        
T1:     [1][2][3]|............  → Send 1,2,3
T2:     [1][2][3]|............  → Wait for ACKs
T3:     [✓][2][3][4]|.........  → Got ACK(1), send 4
T4:     [✓][✓][3][4][5]|......  → Got ACK(2), send 5
T5:     [✓][✓][✓][4][5][6]|...  → Got ACK(3), send 6
        And so on...
```

## Practice Questions

1. What happens if the window size is 1?
2. What's the difference between Go-Back-N and Selective Repeat?
3. Why can't we have an infinite window size?
4. How does the sliding window help with network congestion?

---

**Remember**: Sliding Window = Smart batching + Progressive movement forward! 🪟➡️
