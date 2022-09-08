/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lumenthi <lumenthi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/20 11:05:20 by lumenthi          #+#    #+#             */
/*   Updated: 2022/09/08 11:28:35 by lumenthi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "traceroute.h"

unsigned short checksum(void *b, int len)
{
	unsigned short	*buf = b;
	unsigned int	sum = 0;
	unsigned short	result;

	/* Sum up 2-byte values until none or only one byte left. */
	while (len > 1) {
		sum += *buf++;
		len -= 2;
	}

	/* Add left-over byte, if any. */
	if (len == 1)
		sum += *(unsigned char*)buf;

	/* Fold 32-bit sum into 16 bits; we lose information by doing this,
		increasing the chances of a collision.
		sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits) */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	/* Checksum is one's compliment of sum. */
	result = ~sum;
	return result;
}

static void debug_packet()
{

}

static int monitor_packet(t_data *g_data, int ttl)
{
	struct icmp_packet		icmp_packet;
	struct sockaddr			receiver;
	socklen_t				receiver_len;

	/* Setting TTL option */
	if (setsockopt(g_data->sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
		fprintf(stderr, "%s: %s: Failed to set TTL\n", g_data->path, g_data->address);
		freeaddrinfo(g_data->host_info);
		return 1;
	}

	printf("traceroute to %s (%s), %d hops max, %ld bytes packets\n",
		g_data->address, g_data->ipv4, g_data->hops, sizeof(icmp_packet));

	/* Formatting packet header */
	ft_memset(&icmp_packet, 0, sizeof(icmp_packet));
	icmp_packet.hdr.type = ICMP_ECHO;
	icmp_packet.hdr.un.echo.id = getpid();
	icmp_packet.hdr.un.echo.sequence = g_data->sequence;
	icmp_packet.hdr.checksum = checksum(&icmp_packet, sizeof(icmp_packet));

	/* Preparing receiver */
	receiver_len = sizeof(receiver);

	/* debug_packet(packet); */
	if (sendto(g_data->sockfd,
				&icmp_packet,
				sizeof(icmp_packet),
				0,
				g_data->host_addr,
				sizeof(*(g_data->host_addr))) <= 0)
	{
		fprintf(stderr, "Failed to send packet\n");
		return -1;
	}
	g_data->sequence++;
	if (recvfrom(g_data->sockfd,
				&icmp_packet,
				sizeof(icmp_packet),
				0,
				&receiver,
				&receiver_len) <= 0 && g_data->sequence > 0)
	{
		fprintf(stderr, "Failed to receive packet\n");
		return -1;
	}
	debug_packet(icmp_packet);
	return 0;
}

void traceroute_loop(t_data *g_data)
{

	int ttl = 64; /* Set to 1 */

	monitor_packet(g_data, ttl);
}
