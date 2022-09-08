/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   traceroute.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lumenthi <lumenthi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/15 13:05:31 by lumenthi          #+#    #+#             */
/*   Updated: 2022/09/08 17:13:10 by lumenthi         ###   ########.fr       */
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

# define ARGS_H args & 0x01

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

typedef struct	s_data {
	uint8_t				args;
	char				*path;
	char				ipv4[INET_ADDRSTRLEN];
	struct addrinfo		*host_info;
	uint16_t			port;

	struct sockaddr		*host_addr;
	struct sockaddr_in	servaddr;

	struct timeval		start_time;
	struct timeval		end_time;
	char				*address;
	int					sockfd;
	int					interval;
	unsigned int		sequence;
	unsigned int		hops;
}						t_data;

/* ip */
/*
struct iphdr
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
};
*/

/* icmp */
/*
struct icmphdr
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
};
*/

typedef struct icmp_packet {
	struct icmphdr		hdr;
	char				msg[60-sizeof(struct icmphdr)];
}						t_icmp_packet;

typedef struct	packet {
	struct iphdr		hdr;
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
-TCP (SOCK_STREAM)
-UDP (SOCK_DGRAM)
*/

#endif
