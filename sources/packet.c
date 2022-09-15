/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lumenthi <lumenthi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/15 10:00:10 by lumenthi          #+#    #+#             */
/*   Updated: 2022/09/15 10:00:15 by lumenthi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "traceroute.h"

static int send_packet(t_data *g_data, int rsocket)
{
	char			buf[g_data->size];
	unsigned int	i = 0;
	int				frag = IP_PMTUDISC_DONT;

	/* Packet content */
	while (i < g_data->size)
	{
		buf[i] = 0x40+i;
		i++;
	}

	/* Package informations */
	g_data->servaddr.sin_family = AF_INET;
	g_data->servaddr.sin_port = htons(g_data->port);
	g_data->host_addr = (struct sockaddr *)&g_data->servaddr;

	/* Check port */
	if (g_data->port > USHRT_MAX) {
		printf("\nStarting port is too high, can't go further\n");
		return -2;
	}

	/* Set packet's TTL */
	if (setsockopt(rsocket, SOL_IP, IP_TTL, &g_data->ttl,
		sizeof(g_data->ttl)) != 0) {
		fprintf(stderr, "Failed to set sender's TTL\n");
		return -2;
	}
	/* Real traceroute sets IP_PMTUDISC_DONT to allow fragmentation */
	if (setsockopt(rsocket, IPPROTO_IP, IP_MTU_DISCOVER, &frag, sizeof(frag)) != 0) {
		fprintf(stderr, "Failed to set sender's fragmentation\n");
		return -2;
	}

	if (sendto(rsocket, buf, sizeof(buf), 0,
		g_data->host_addr, sizeof(*(g_data->host_addr))) <= 0)
	{
		fprintf(stderr, "Failed to send packet\n");
		return -2;
	}

	g_data->queries[CURRENT_QUERY].port = g_data->port;
	g_data->queries[CURRENT_QUERY].ttl = g_data->ttl;
	g_data->queries[CURRENT_QUERY].status = SENT;

	/* Filling start time for the query */
	if ((gettimeofday(&g_data->queries[CURRENT_QUERY].start_time, NULL)) < 0) {
		fprintf(stderr, "Failed to set packet sending time\n");
		return -1;
	}


	return rsocket;
}

static unsigned int get_packet_index(t_query *queries, unsigned int port,
	unsigned int limit)
{
	unsigned int i = 0;

	while (i < limit) {
		if (queries[i].port == port)
			break;
		i++;
	}
	return i;
}

static int queries_informations(t_data *g_data, struct packet full_packet,
	struct sockaddr_in *receiver)
{
	unsigned int port;
	uint8_t type;
	struct iphdr *ip_hdr;
	struct udphdr *udp_hdr;
	struct icmphdr *icmp_hdr;
	unsigned int index;
	struct timeval end_time;

	/* First thing to do, do not waste usec */
	if ((gettimeofday(&end_time, NULL)) < 0) {
		fprintf(stderr, "Failed to set packet receiving time\n");
		return -1;
	}

	icmp_hdr = (struct icmphdr *)(&full_packet.content.hdr);
	ip_hdr = (struct iphdr *)(&full_packet.content.msg);
	udp_hdr = (struct udphdr *)((void *)ip_hdr+sizeof(struct iphdr));

	port = ntohs(udp_hdr->dest);
	type = icmp_hdr->type;

	index = get_packet_index(g_data->queries, port, g_data->tqueries);
	/* Invalid packet */
	if (index >= g_data->tqueries || g_data->queries[index].status != SENT) {
		/* Dropping packet */
		return -1;
	}

	ft_strncpy(
		g_data->queries[index].ipv4,
		inet_ntoa(receiver->sin_addr),
		INET_ADDRSTRLEN
	);

	g_data->queries[index].end_time.tv_sec = end_time.tv_sec;
	g_data->queries[index].end_time.tv_usec = end_time.tv_usec;

	if (type == ICMP_TIME_EXCEEDED)
		g_data->queries[index].status = RECEIVED;
	else if (type == ICMP_DEST_UNREACH) {
		g_data->queries[index].status = RECEIVED_END;
		g_data->reached = 1;
	}

	return 1;
}

static int receive_packet(t_data *g_data, int rsocket)
{
	struct packet	rec_packet;
	struct sockaddr receiver;
	socklen_t		receiver_len;
	int				flags = g_data->drop ? MSG_DONTWAIT : 0;

	/* Set receive timeout */
	if (setsockopt(rsocket, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)&g_data->timeout, sizeof(g_data->timeout)) != 0) {
		fprintf(stderr, "Failed to set receiver's timeout\n");
		return -1;
	}

	/* Formatting receivers */
	ft_memset(&rec_packet, 0, sizeof(rec_packet));
	receiver_len = sizeof(receiver);
	ft_memset(&receiver, 0, receiver_len);

	if (recvfrom(rsocket,
				&rec_packet,
				sizeof(rec_packet),
				flags,
				&receiver,
				&receiver_len) <= 0)
	{
		/* Packet timed out, drop next incoming packets */
		g_data->drop = 1;
		return 1;
	}

	return queries_informations(g_data, rec_packet, (struct sockaddr_in *)&receiver);
}

static int	udp_iterate(t_data *g_data)
{
	unsigned int i = 0;
	while (i < g_data->squeries) {
		/* Sent all packets */
		if (CURRENT_QUERY < g_data->tqueries &&
			FD_ISSET(g_data->udp_sockets[i], &g_data->udpfds)) {
			if (send_packet(g_data, g_data->udp_sockets[i]) == -2)
				return -1;
			g_data->port++;
			g_data->ttl = g_data->sttl + ((CURRENT_QUERY) / g_data->probe);
			g_data->sent++;
		}
		i++;
	}
	return 0;
}

static int	icmp_receive(t_data *g_data)
{
	unsigned int i = 0;
	unsigned int rec = 0;

	if (FD_ISSET(g_data->icmp_socket, &g_data->icmpfd)) {
		while (rec < g_data->sent) {
			if (receive_packet(g_data, g_data->icmp_socket) > 0)
				rec++;
			i++;
		}
	}
	return 0;
}

int monitor_packet(t_data *g_data)
{
	g_data->drop = 0;
	g_data->sent = 0;
	if (CURRENT_QUERY < g_data->tqueries && !g_data->reached &&
		select(g_data->maxfd+1, NULL, &g_data->udpfds, NULL, &g_data->timeout))
	{
		if (udp_iterate(g_data) < 0)
			return 1;
	}
	icmp_receive(g_data);

	return print_everything(g_data);
}
