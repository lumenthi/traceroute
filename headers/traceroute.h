/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   traceroute.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lumenthi <lumenthi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/15 13:05:31 by lumenthi          #+#    #+#             */
/*   Updated: 2022/09/07 19:30:25 by lumenthi         ###   ########.fr       */
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

typedef struct	s_data {
	uint8_t args;
	char *path;
	char ipv4[INET_ADDRSTRLEN];
	struct addrinfo *host_info;
	struct sockaddr *host_addr;
	struct timeval start_time;
	struct timeval end_time;
	char *address;
	int sockfd;
	int interval;
}	t_data;

typedef struct s_packet {
	struct icmphdr hdr;
	char msg[64-sizeof(struct icmphdr)];
}	t_packet;

/* traceroute.c */
int		ft_traceroute(char *destination, uint8_t args, char *path);

/* packet.c */
void	traceroute_loop(t_data g_data);

#endif
