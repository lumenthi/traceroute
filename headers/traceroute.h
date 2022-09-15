/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   traceroute.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lumenthi <lumenthi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/15 09:59:41 by lumenthi          #+#    #+#             */
/*   Updated: 2022/09/15 09:59:44 by lumenthi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

# define ARGS_h args & 0x01
# define ARGS_f args & 0x02
# define ARGS_m args & 0x04
# define ARGS_N args & 0x08
# define ARGS_n args & 0x10
# define ARGS_p args & 0x20
# define ARGS_q args & 0x40
# define ARGS_INVALID 0xFF

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

#define NOT_USED 0
#define SENT 1
#define RECEIVED 2
#define RECEIVED_END 3
#define DISPLAYED 4

/* Index of the current querry when sending */
#define CURRENT_QUERY g_data->port-g_data->sport

typedef struct			s_query {
	char				ipv4[INET_ADDRSTRLEN];
	unsigned int		ttl;
	unsigned int		port;
	struct timeval		start_time;
	struct timeval		end_time;
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
	unsigned int		tqueries; /* Total queries */
	unsigned int		ttl;
	unsigned int		sttl;
	unsigned int		probe;
	unsigned int		port;
	unsigned int		sport;
	unsigned int		sent;
	struct timeval		timeout;
	uint8_t				reached;
	uint8_t				drop;

	/* Select related */
	fd_set				udpfds;
	fd_set				icmpfd;
	int					maxfd;
	/* Dynamically allocated */
	int					*udp_sockets;
	int					icmp_socket;

	/* Display related */
	/* Dynamically allocated */
	t_query				*queries; /* Informations about queries */
	char				aprobe[INET_ADDRSTRLEN]; /* Address of current probe */
	unsigned int		tprobe; /* TTL of current probe */
	unsigned int		cprobe; /* Probe counter */
	uint8_t				pend; /* Already reached end ? */
	char				host[NI_MAXHOST]; /* Hostname of current probe */

	unsigned int		cttl; /* Packet count for a ttl */
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
int		ft_traceroute(char *destination, uint8_t args,
	char *path, t_data g_data);

/* print.c */
int		print_help();
int		print_everything(t_data *g_data);

/* packet.c */
int		monitor_packet(t_data *g_data);

#endif
