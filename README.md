# GNU Linux traceroute implementation
Prints a trace of the route IP packets are travelling to a remote host.
## Description
### General
- Traceroute (by default) sends UDP packets to a target host and wait for the ICMP replies
- Traceroute sends three datagrams for each value for the TTL field, printing a diagnostic line of output for these
- The TTL field is then steadily increased until the intended host responds
- In case there is no response within a time period of five seconds, an asterisque ‘*’ is printed

## Options
Implemented options:  
```
-f <first_ttl>     Start from the first_ttl hop (default is 1) [1-250]
-h                 print help and exit
-m <max_ttl>       Set the max number of hops (default is 30) [1-255]
-N <squeries>      Set the number of probes to be tried simultaneously (default is 16) [1-100]
-n                 Do not resolve IP addresses to their domain names
-p <port>          Set the destination port to use (default is 33434) [1-65535]
-q <nqueries>      Set the number of probes per each hop (default is 3) [1-10]
```
## Make
Given Makefile contains all standard rules
- all
- clean
- fclean
- re

## Example output
```bash
$ make
 ~ | Compiled libft
 ~ | Compiled ft_traceroute
$ sudo ./ft_traceroute www.google.com
traceroute to www.google.com (216.58.213.68), 30 hops max, 60 bytes packets
 1  10.0.2.2 (10.0.2.2)  0.335 ms  0.327 ms  0.377 ms
 2  10.11.254.254 (10.11.254.254)  0.584 ms  0.790 ms  0.784 ms
 3  10.60.1.11 (10.60.1.11)  0.463 ms  0.458 ms  0.451 ms
 4  dc3.42.fr (62.210.35.1)  3.200 ms  3.196 ms  3.189 ms
 5  195.154.1.174 (195.154.1.174)  1.388 ms  1.437 ms  1.780 ms
 6  a9k1-45x-s44-2.dc3.poneytelecom.eu (195.154.1.104)  1.272 ms  1.002 ms  1.155 ms
 7  51.158.1.34 (51.158.1.34)  1.117 ms 51.158.1.36 (51.158.1.36)  1.075 ms  0.970 ms
 8  62.210.0.155 (62.210.0.155)  1.350 ms 62.210.0.159 (62.210.0.159)  1.395 ms 62.210.0.147 (62.210.0.147)  1.304 ms
 9  195.154.3.214 (195.154.3.214)  1.422 ms 209.85.149.12 (209.85.149.12)  1.385 ms 195.154.3.214 (195.154.3.214)  1.371 ms
10  108.170.245.1 (108.170.245.1)  2.588 ms  2.571 ms  2.537 ms
11  142.250.224.199 (142.250.224.199)  2.081 ms  2.222 ms 142.250.224.197 (142.250.224.197)  2.171 ms
12  par21s18-in-f4.1e100.net (216.58.213.68)  1.374 ms  1.546 ms  1.369 ms
```
