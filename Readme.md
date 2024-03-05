#### Load Balancer setup

- g++ --std=c++14 -o load_balancer load_balancer.cpp
- ./load_balancer


#### Load Balancer modes supported

- Round Robin
- Weighted Round Robin
- IP Hashing
- Random


1. Round Robin Mode : Requests are routed in a round-robin (circular) rotation basis

- ./load_balancer RR
- ./test

<pre>
MacBook-Air:load-balancer anish$ ./load_balancer RR
Load balancer server running...
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8082
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8082
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8082
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8081
</pre>

2. Weighted Round Robin Mode : Requests are routed based on the assigned weights of backend servers

- ./load_balancer WRR
- ./test

Assigned weights

    - Server 1: 127.0.0.1:8081 -> 3
    - Server 2: 127.0.0.1:8082 -> 1
    - Server 3: 127.0.0.1:8083 -> 2

<pre>
MacBook-Air:load-balancer anish$ ./load_balancer WRR
Load balancer server running...
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8082
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8082
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8082
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8081
</pre>


3. IP Hashing Mode : Requests are routed based on the client's IP address. Subsequent requests originating
from the same client will be routed to previously mapped backend server

- ./load_balancer IPH

- Connect from different machines with `nc <load_balancer_server_ip> 8080`

<pre>
MacBook-Air:load-balancer anish$ ./load_balancer IPH
Load balancer server running...
Client IP: 192.168.0.3
Routing request to backend server: 127.0.0.1:8083
Client IP: 192.168.0.10
Routing request to backend server: 127.0.0.1:8082
Client IP: 192.168.0.10
Routing request to backend server: 127.0.0.1:8082
Client IP: 192.168.0.3
Routing request to backend server: 127.0.0.1:8083
Client IP: 192.168.0.3
Routing request to backend server: 127.0.0.1:8083
Client IP: 192.168.0.10
Routing request to backend server: 127.0.0.1:8082
</pre>


4. Random Mode : Requests are routed to backend servers randomly.

- ./load_balancer RA
- ./test

<pre>
MacBook-Air:load-balancer anish$ ./load_balancer RA
Load balancer server running...
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8082
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8083
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8082
Routing request to backend server: 127.0.0.1:8081
Routing request to backend server: 127.0.0.1:8082
</pre>
