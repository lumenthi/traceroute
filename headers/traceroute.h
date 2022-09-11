#ifndef TRACEROUTE_H
# define TRACEROUTE_H

#include "libft.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <limits.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

# define ARGS_H args & 0x01

/* struct sockaddr {
	ushort	sa_family;
	char	sa_data[14];
}; */

/* struct sockaddr_in {
	short			sin_family;
	u_short			sin_port;
	struct in_addr	sin_addr;
	char			sin_zero[8];
}; */

/* struct addrinfo {
	int				ai_flags;
	int				ai_family;
	int				ai_socktype;
	int				ai_protocol;
	size_t			ai_addrlen;
	struct sockaddr	*ai_addr;
	char			*ai_canonname;
	struct addrinfo	*ai_next;
}; */

#define SENT 0
#define RECEIVED 1
#define NOT_DISPLAYED 2
#define DISPLAYED 3

typedef struct			s_query {
	char				ipv4[INET_ADDRSTRLEN];
	unsigned int		port;
	uint8_t				status;
}						t_query;

typedef struct	s_data {
	uint8_t				args;
	char				*path;
	char				ipv4[INET_ADDRSTRLEN];
	struct addrinfo		*host_info;

	struct sockaddr		*host_addr;
	struct sockaddr_in	servaddr;

	struct timeval		start_time;
	struct timeval		end_time;
	char				*address;

	int					interval;
	unsigned int		hops; /* Max hops */
	unsigned int		size; /* Packet size */
	unsigned int		squeries; /* Simultaneous queries */
	unsigned int		ttl;
	unsigned int		sttl;
	unsigned int		port;
	unsigned int		sport;

	/* Select related */
	fd_set				udpfds;
	fd_set				icmpfd;
	int					maxfd;
	/* Dynamically allocated */
	int					*udp_sockets;
	int					icmp_socket;

	/* Display related */
	/* Dynamically allocated */
	unsigned int		tqueries;
	t_query				*queries;
}						t_data;


/* struct iphdr
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int	ihl:4;
	unsigned int	version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	unsigned int	version:4;
	unsigned int	ihl:4;
#else
# error        "Please fix <bits/endian.h>"
#endif
	u_int8_t	tos;
	u_int16_t	tot_len;
	u_int16_t	id;
	u_int16_t	frag_off;
	u_int8_t	ttl;
	u_int8_t	protocol;
	u_int16_t	check;
	u_int32_t	saddr;
	u_int32_t	daddr;
}; */

/* struct udphdr
{
	u_int16_t	source;
	u_int16_t	dest;
	u_int16_t	len;
	u_int16_t	check;
}; */

/* struct icmphdr
{
	u_int8_t	type;
	u_int8_t	code;
	u_int16_t	checksum;
	union
	{
		struct
		{
			u_int16_t	id;
			u_int16_t	sequence;
		}	echo;
		u_int32_t	gateway;
		struct
		{
			u_int16_t	__unused;
			u_int16_t	mtu;
		}	frag;
	}	un;
}; */

typedef struct icmp_packet {
	struct icmphdr		hdr;
	char				msg[60-sizeof(struct icmphdr)];
}						t_icmp_packet;

typedef struct udp_packet {
	struct udphdr		hdr;
	char				msg[60-sizeof(struct udphdr)];
}						t_udp_packet;

typedef struct packet {
	struct iphdr		ip_hdr;
	struct icmp_packet	content;
}						t_packet;

/* traceroute.c */
int		ft_traceroute(char *destination, uint8_t args, char *path);

/* packet.c */
void	traceroute_loop(t_data *g_data);

/* ====NOTES====

- Default packet size: 60 ?
- www.baidu.com
- Entering the traceroute host command without options sends three 40-byte ICMP datagrams with an initial TTL of 1, a maximum TTL of 30, a timeout period of 5 seconds, and a TOS specification of 0 to destination UDP port number 33434. For each host in the processed path, the initial TTL for each host and the destination UDP port number for each packet sent are incremented by one.
- 16 Packets simultaneously
- Port incrementation
- TCP (SOCK_STREAM)
- UDP (SOCK_DGRAM)
- sudo tcpdump udp -X
*/

#endif
