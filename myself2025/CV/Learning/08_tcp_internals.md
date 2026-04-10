# TCP Internals — Deep Dive

**Study Time: ~2.5 hours**  
**Prerequisites:** Basic networking (IP addresses, ports, client-server model)

---

## 1. TCP in the Big Picture

```
Application Layer:  HTTP, gRPC, your custom protocol
                         ↓
Transport Layer:    TCP (reliable, ordered, byte stream)
                    UDP (unreliable, unordered, datagrams)
                         ↓
Network Layer:      IP (routing, addressing)
                         ↓
Link Layer:         Ethernet, WiFi (frames on the wire)
```

**TCP provides:**
- Reliable delivery (retransmits lost packets)
- Ordered delivery (reassembles out-of-order packets)
- Flow control (don't overwhelm the receiver)
- Congestion control (don't overwhelm the network)
- Byte-stream abstraction (no message boundaries)

---

## 2. TCP Header

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          Source Port          |       Destination Port        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Sequence Number                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Acknowledgment Number                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Data |           |U|A|P|R|S|F|                               |
| Offset| Reserved  |R|C|S|S|Y|I|            Window             |
|       |           |G|K|H|T|N|N|                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           Checksum            |         Urgent Pointer        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Options (variable)                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

Key fields:
  Sequence Number:  byte position of first data byte in this segment
  Ack Number:       next byte the receiver expects (cumulative ack)
  Window:           how many bytes the receiver can accept (flow control)
  Flags:            SYN (start), ACK (acknowledge), FIN (end), RST (reset),
                    PSH (push to app), URG (urgent data)
```

---

## 3. Connection Lifecycle

### Three-Way Handshake (Connection Establishment)

```
    Client                              Server
      |                                    |
      |── SYN (seq=100) ─────────────────→|  "I want to connect"
      |                                    |  seq=100 is client's ISN
      |                                    |
      |←── SYN-ACK (seq=300, ack=101) ───|  "OK, I accept"
      |                                    |  seq=300 is server's ISN
      |                                    |  ack=101: "I expect your next byte to be 101"
      |                                    |
      |── ACK (seq=101, ack=301) ────────→|  "Got it"
      |                                    |
      |      CONNECTION ESTABLISHED        |

Why three steps (not two)?
  Two-step: server doesn't know if client received SYN-ACK
  → Could waste resources on half-open connections
  Three-step: both sides confirm they can send AND receive
```

### Data Transfer

```
    Client                              Server
      |                                    |
      |── seq=101, len=500 ─────────────→|  "Here's 500 bytes starting at 101"
      |                                    |
      |←── ack=601 ──────────────────────|  "Got it, send byte 601 next"
      |                                    |
      |── seq=601, len=300 ─────────────→|  "300 more bytes"
      |                                    |
      |←── ack=901 ──────────────────────|  "Got it"

Cumulative ACK: ack=601 means "I have ALL bytes up to 600"
  Even if some earlier ack was lost, next ack covers everything
```

### Four-Way Termination

```
    Client                              Server
      |                                    |
      |── FIN (seq=901) ───────────────→|  "I'm done sending"
      |                                    |
      |←── ACK (ack=902) ──────────────|  "Noted"
      |                                    |
      |    (server may still send data)    |
      |                                    |
      |←── FIN (seq=700) ──────────────|  "I'm done too"
      |                                    |
      |── ACK (ack=701) ───────────────→|  "Got it"
      |                                    |
      | [TIME_WAIT: 2×MSL ≈ 60 sec]      |
      |                                    |

TIME_WAIT (client side):
  - Lasts 2 × Maximum Segment Lifetime (typically 60 seconds)
  - Purpose: ensure final ACK arrives; allow late packets to expire
  - Problem: high-traffic servers can exhaust ephemeral ports
  - Fix: net.ipv4.tcp_tw_reuse = 1 (allows reuse of TIME_WAIT sockets)
```

### TCP State Diagram (Simplified)

```
Client:
  CLOSED → SYN_SENT → ESTABLISHED → FIN_WAIT_1 → FIN_WAIT_2 → TIME_WAIT → CLOSED

Server:
  CLOSED → LISTEN → SYN_RECEIVED → ESTABLISHED → CLOSE_WAIT → LAST_ACK → CLOSED
```

---

## 4. Flow Control

**Problem:** Receiver might be slower than sender. Without control, receiver's buffer overflows → packets dropped.

**Solution:** Receiver advertises a **window size** — how many bytes it can accept.

```
Receiver Buffer (16 KB):
  [consumed by app][  queued data  ][    free space    ]
                                    |←── window = 8KB ──→|

Sender sees: "I can send up to 8 KB before I must stop"

As receiver's app reads data:
  [consumed + app read][  less queued  ][  MORE free space   ]
                                        |←── window = 12KB ──→|
  
  Receiver sends ACK with updated window → sender can send more
```

### Window Scaling
```
TCP header window field: 16 bits → max 65,535 bytes
Modern networks need much larger windows (high bandwidth × high latency)

Solution: Window Scale option (negotiated in SYN)
  3-way handshake: "I'll use scale factor 7" → window × 2^7 = window × 128
  
  Allows windows up to 65535 × 2^14 = 1 GB
  
  BDP (Bandwidth-Delay Product):
    To keep a 10 Gbps link with 10ms RTT fully utilized:
    BDP = 10 Gbps × 10ms = 12.5 MB → need at least 12.5 MB window
```

### Zero Window
```
Receiver buffer full:
  Receiver sends ACK with window=0 → sender MUST stop

  Sender enters persist state:
    Periodically sends 1-byte probe (window probe)
    Receiver responds with current window size
    When window > 0 → sender resumes

  Stall visible in tcpdump as:
    [TCP ZeroWindow]
    [TCP Window Update]
```

---

## 5. Congestion Control

**Problem:** Multiple senders on a network. If all send at full speed → switches overflow → packets dropped → everyone retransmits → worse congestion (congestion collapse).

**Solution:** Each sender maintains a **congestion window (cwnd)** that limits how much unacknowledged data it can have in flight.

```
Effective window = min(cwnd, receiver_window)
  cwnd:            sender's estimate of network capacity
  receiver_window: receiver's buffer availability
```

### Slow Start

```
cwnd starts at 1 MSS (Maximum Segment Size, typically 1460 bytes)
For each ACK received: cwnd += 1 MSS

Effect: cwnd doubles every RTT (exponential growth)

RTT 0: cwnd = 1 MSS  → send 1 segment
        gets 1 ACK    → cwnd = 2
RTT 1: cwnd = 2 MSS  → send 2 segments
        gets 2 ACKs   → cwnd = 4
RTT 2: cwnd = 4 MSS  → send 4 segments
        gets 4 ACKs   → cwnd = 8
RTT 3: cwnd = 8 MSS  → send 8 segments
...

Not actually "slow" — exponential! But starts from 1.
Stops when: cwnd reaches ssthresh (slow-start threshold)

Initial cwnd (IW):
  Old standard: 1 MSS
  RFC 6928 (2013): 10 MSS → ~14 KB 
  This is why first page load feels faster on modern systems
```

### Congestion Avoidance

```
When cwnd >= ssthresh: switch to linear growth
For each RTT: cwnd += 1 MSS (additive increase)

RTT 0: cwnd = 16 MSS
RTT 1: cwnd = 17 MSS
RTT 2: cwnd = 18 MSS
...

Slow, cautious probing for available bandwidth
```

### Packet Loss Detection & Response

```
Method 1: Triple Duplicate ACK (Fast Retransmit)
  Sender sends segments 1, 2, 3, 4, 5
  Segment 3 lost
  Receiver: ACK 1, ACK 2, ACK 2 (dup), ACK 2 (dup), ACK 2 (dup)
  
  3 duplicate ACKs → sender retransmits segment 3 immediately
  (Don't wait for timeout — that's too slow)
  
  Response (Reno):
    ssthresh = cwnd / 2
    cwnd = cwnd / 2    (multiplicative decrease)
    Resume congestion avoidance from new cwnd
  
  This is AIMD: Additive Increase, Multiplicative Decrease

Method 2: Retransmission Timeout (RTO)
  No ACK received within RTO → assume severe loss
  
  Response:
    ssthresh = cwnd / 2
    cwnd = 1 MSS         (back to slow start!)
    Much more aggressive response — network might be badly congested
  
  RTO calculation:
    Based on smoothed RTT (SRTT) and RTT variance
    RTO = SRTT + 4 × RTT_variance
    Minimum RTO: typically 200ms (Linux default: 200ms)
```

### Modern Congestion Control: CUBIC and BBR

```
CUBIC (Linux default since 2006):
  - Uses a cubic function for window growth (not linear)
  - Aggressive growth when far from last loss point
  - Gentle probing near last loss point
  - W = C × (t - K)³ + W_max
  - Good for high-BDP networks (fat pipes)

BBR (Bottleneck Bandwidth and RTT) — Google, 2016:
  - Model-based: estimates bottleneck bandwidth and min RTT
  - Doesn't use packet loss as congestion signal
  - Better for lossy networks (wireless, WAN)
  - Used by Google, YouTube, Cloudflare
  
  Phases:
    STARTUP: exponential probe for bandwidth (like slow start)
    DRAIN: drain excess queue
    PROBE_BW: steady state, periodically probe for more bandwidth
    PROBE_RTT: occasionally drain queue to measure true min RTT

  Key insight: Loss-based CC (Reno, CUBIC) fill buffers (bufferbloat)
               BBR tries to operate at the point of max throughput
               with minimum delay
```

---

## 6. Nagle's Algorithm & TCP_NODELAY

```
Nagle's Algorithm (RFC 896):
  Problem: small writes (1-byte telnet keystrokes) generate 
           41-byte packets (20 IP + 20 TCP + 1 data) → 97.6% overhead
  
  Solution: buffer small writes until one of:
    a) All previously sent data has been acknowledged
    b) Buffer has accumulated a full MSS (1460 bytes)
    
  Effect: consolidates small writes into larger segments
  
When Nagle hurts:
  - RPC/request-response protocols: write header, write body
    Nagle holds the body until header is ACK'd → adds 1 RTT latency
  
  - Real-time applications: games, trading systems
    Every millisecond matters → don't buffer

TCP_NODELAY:
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
  
  Disables Nagle → every write() sends immediately
  Essential for: storage data paths, RPC frameworks, low-latency systems
```

### Delayed ACK & Nagle Interaction

```
Delayed ACK: receiver waits up to 40ms before sending ACK
  Hoping to piggyback ACK on outgoing data → fewer packets

Problem: Nagle + Delayed ACK = 40ms latency

  Client: write(100 bytes) → sent immediately (first write)
  Client: write(50 bytes)  → Nagle buffers it (waiting for ACK)
  Server: received 100 bytes → delays ACK 40ms
  Server: ACK finally sent after 40ms → 
  Client: receives ACK → Nagle sends buffered 50 bytes
  
  Total: 40ms added latency for no good reason!

Fix: TCP_NODELAY on the client (disables Nagle)
  Or: TCP_QUICKACK on the server (disables delayed ACK)
  Or: Use writev() to send header+body in one syscall
```

---

## 7. Socket Tuning for Storage Systems

```c
// 1. Disable Nagle (essential for RPC/request-response)
int flag = 1;
setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

// 2. Increase socket buffers (for high throughput)
int bufsize = 4 * 1024 * 1024;  // 4 MB
setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
// Kernel doubles this internally (for bookkeeping overhead)
// Also set system-wide: /proc/sys/net/core/rmem_max, wmem_max

// 3. Enable keepalives (detect dead peers)
int keepalive = 1;
int idle = 60;      // seconds before first probe
int interval = 10;  // seconds between probes
int count = 5;      // probes before declaring dead
setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));

// 4. Address reuse (avoid TIME_WAIT issues on server restart)
int reuse = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)); // load balancing

// 5. TCP_CORK (batch small writes into one segment)
int cork = 1;
setsockopt(fd, IPPROTO_TCP, TCP_CORK, &cork, sizeof(cork));
// ... multiple write() calls ...  (all buffered)
cork = 0;
setsockopt(fd, IPPROTO_TCP, TCP_CORK, &cork, sizeof(cork));
// Now sends everything as one segment
```

### System-Wide Tuning (/proc/sys/net)

```bash
# Increase max socket buffer sizes
echo 16777216 > /proc/sys/net/core/rmem_max      # 16 MB
echo 16777216 > /proc/sys/net/core/wmem_max

# TCP buffer auto-tuning range (min, default, max)
echo "4096 1048576 16777216" > /proc/sys/net/ipv4/tcp_rmem
echo "4096 1048576 16777216" > /proc/sys/net/ipv4/tcp_wmem

# Reuse TIME_WAIT connections
echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse

# Increase connection backlog
echo 65535 > /proc/sys/net/core/somaxconn
echo 65535 > /proc/sys/net/ipv4/tcp_max_syn_backlog

# Switch congestion control to BBR
echo bbr > /proc/sys/net/ipv4/tcp_congestion_control

# Increase ephemeral port range
echo "1024 65535" > /proc/sys/net/ipv4/ip_local_port_range
```

---

## 8. Debugging TCP with tcpdump

```bash
# Capture all traffic on port 8080
sudo tcpdump -i any port 8080 -nn

# Show TCP flags and sequence numbers
sudo tcpdump -i any port 8080 -nn -S

# Output:
# 10:00:00.000 IP 10.1.1.1.54321 > 10.1.1.2.8080: Flags [S], seq 100, win 65535, length 0
# 10:00:00.001 IP 10.1.1.2.8080 > 10.1.1.1.54321: Flags [S.], seq 300, ack 101, win 65535
# 10:00:00.001 IP 10.1.1.1.54321 > 10.1.1.2.8080: Flags [.], ack 301, win 65535

# Flag legend: S=SYN, .=ACK, F=FIN, R=RST, P=PSH

# Save to file for Wireshark analysis
sudo tcpdump -i any port 8080 -w capture.pcap

# Show packet contents (hex + ASCII)
sudo tcpdump -i any port 8080 -X

# Filter retransmissions (TCP analysis in tshark)
tshark -i any -f "port 8080" -Y "tcp.analysis.retransmission"
```

### Common TCP Issues Visible in tcpdump

```
1. [TCP Retransmission]:
   Same segment sent again → packet loss or severe congestion
   
2. [TCP Zero Window]:
   Receiver buffer full → sender must stop
   Usually means: receiver app is slow reading from socket
   
3. [TCP RST]:
   Connection forcefully terminated
   Causes: closed port, firewall, application crash, half-open connection
   
4. [TCP Dup ACK]:
   Same ACK sent again → segment was received out of order
   3 dup ACKs → fast retransmit triggered
   
5. High RTT between SYN and SYN-ACK:
   Server overloaded or network latency
   
6. Many connections in TIME_WAIT (ss -s):
   Server creating/closing many short connections
   Fix: connection pooling, SO_REUSEADDR, tcp_tw_reuse
```

---

## 9. TCP vs UDP — When to Use What

| Aspect | TCP | UDP |
|--------|-----|-----|
| Reliability | Guaranteed delivery | Best effort |
| Ordering | Ordered byte stream | No ordering |
| Connection | Connection-oriented (handshake) | Connectionless |
| Head-of-line blocking | Yes (one lost packet blocks all) | No |
| Use cases | HTTP, databases, RPC, file transfer | DNS, video, gaming, QUIC |

**For storage systems:** Almost always TCP. Reliability and ordering are non-negotiable for data integrity. The exception is RDMA (covered in 09_rdma.md) which provides its own reliability.

**QUIC (HTTP/3):** UDP-based protocol with TCP-like reliability + TLS built in + no head-of-line blocking. Growing in adoption.

---

## 10. Interview Questions

**Q: "How would you tune TCP for a storage replication data path?"**  
A: "Disable Nagle (TCP_NODELAY) for low-latency request-response. Increase SO_RCVBUF/SO_SNDBUF to match the BDP (bandwidth × RTT) — for a 10 Gbps link with 1ms RTT that's ~1.25 MB per direction. Enable TCP keepalives for detecting dead peers. Consider BBR congestion control for better throughput utilization. Use writev() to send headers and data in a single syscall to avoid the Nagle-delayed-ACK interaction."

**Q: "Explain TCP congestion control to me."**  
A: "TCP uses AIMD — Additive Increase, Multiplicative Decrease. Starts with slow-start (exponential growth) until ssthresh, then congestion avoidance (linear growth). On packet loss detected by triple dup-ACK, cwnd halves (fast recovery). On timeout, cwnd resets to 1 (slow start). Modern algorithms like CUBIC use cubic growth curves, and BBR takes a model-based approach — estimating actual bandwidth and RTT rather than reacting to loss."

**Q: "What's head-of-line blocking in TCP?"**  
A: "TCP guarantees ordered delivery. If packets 1, 2, 3, 4, 5 are sent and packet 3 is lost, the receiver has 1, 2, 4, 5 but can't deliver 4, 5 to the application until 3 is retransmitted and received. Packets 4 and 5 are 'blocked' by the lost packet at the 'head of the line.' This is why QUIC (over UDP) uses multiple independent streams — loss in one stream doesn't block others."

**Q: "A TCP connection between two data centers has high latency. Throughput is low despite a fast link. What's wrong?"**  
A: "Likely a BDP (Bandwidth-Delay Product) problem. The TCP window might be too small for the bandwidth × latency product. For a 1 Gbps link with 50ms RTT: BDP = 1 Gbps × 50ms = 6.25 MB. If the window is only 64 KB (default without scaling), you can only have 64 KB in flight → effective throughput = 64 KB / 50ms = 10 Mbps. Fix: enable TCP window scaling, increase socket buffers to at least BDP."

---

## Quick Reference Card

```
TCP = reliable, ordered, byte-stream transport

Connection: SYN → SYN-ACK → ACK (three-way handshake)
Teardown:   FIN → ACK → FIN → ACK (four-way) + TIME_WAIT

Flow Control: receiver window (how much receiver can accept)
Congestion Control: sender cwnd (estimated network capacity)
  Slow Start → Congestion Avoidance → Loss → Recovery

Nagle: buffers small writes (disable with TCP_NODELAY for latency)
Delayed ACK: waits 40ms to piggyback (can interact badly with Nagle)

Key socket options:
  TCP_NODELAY      → disable Nagle (essential for RPC)
  SO_RCVBUF/SNDBUF → increase buffer for BDP
  SO_KEEPALIVE     → detect dead peers
  TCP_CORK         → batch multiple writes
  SO_REUSEADDR     → avoid TIME_WAIT issues

BDP = Bandwidth × Delay → minimum window needed for full throughput
Modern CC: CUBIC (default), BBR (model-based, better for lossy networks)

Debugging: tcpdump -nn port X, ss -s, /proc/net/tcp
```
