#include "traceroute.h"

static void print_packet(struct packet full_packet, struct sockaddr_in *receiver,
	unsigned int ttl)
{
	(void)full_packet;
	// printf("ttl: %d\n", full_packet.ip_hdr.ttl);
	// printf("type: %d\n", full_packet.content.hdr.type);
	// printf("id: %d\n", full_packet.content.hdr.un.echo.id);

	printf(" %d  %s (%s)\n",
		ttl, inet_ntoa(receiver->sin_addr), inet_ntoa(receiver->sin_addr));
}

static int send_packet(t_data *g_data, unsigned int ttl, unsigned int port)
{
	int				rsocket;
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
	g_data->servaddr.sin_port = htons(port);
	g_data->host_addr = (struct sockaddr *)&g_data->servaddr;

	if ((rsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr, "Failed to create sender's socket\n");
		return -1;
	}

	/* Set packet's TTL */
	if (setsockopt(rsocket, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
		fprintf(stderr, "Failed to set sender's TTL\n");
		return -1;
	}
	/* Real traceroute sets no flags in header set IP_PMTUDISC_DONT to allow fragmentation */
	if (setsockopt(rsocket, IPPROTO_IP, IP_MTU_DISCOVER, &frag, sizeof(frag)) != 0) {
		fprintf(stderr, "Failed to set sender's fragmentation\n");
		return -1;
	}

	if (sendto(rsocket, buf, sizeof(buf), 0,
		g_data->host_addr, sizeof(*(g_data->host_addr))) <= 0)
	{
		fprintf(stderr, "Failed to send packet\n");
		close(rsocket);
		return -1;
	}

	return rsocket;
}

static int receive_packet(struct packet *rec_packet, struct sockaddr *receiver)
{
	int				rsocket;
	socklen_t		receiver_len;
	struct timeval	timeout;

	/* default timeout (seconds) */
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	if ((rsocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
		fprintf(stderr, "Failed to create receiver's socket\n");
		return -1;
	}

	/* Set receive timeout */
	if (setsockopt(rsocket, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)&timeout, sizeof(timeout)) != 0) {
		fprintf(stderr, "Failed to set receiver's timeout\n");
		return -1;
	}

	/* Formatting receivers */
	ft_memset(rec_packet, 0, sizeof(*rec_packet));
	receiver_len = sizeof(*receiver);
	ft_memset(receiver, 0, receiver_len);

	if (recvfrom(rsocket,
				rec_packet,
				sizeof(*rec_packet),
				0,
				receiver,
				&receiver_len) <= 0)
	{
		fprintf(stderr, "Failed to receive packet\n");
		return -1;
	}
	close(rsocket);
	return 0;
}

static int	 create_sockets(t_data *g_data)
{
	unsigned int i = 0;

	while (i < g_data->squeries) {
		/* UDP socket */
		if ((g_data->udp_sockets[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		{
			fprintf(stderr, "Failed to create sender's socket\n");
			return -1;
		}
		FD_SET(g_data->udp_sockets[i], &g_data->udpfds);

		/* ICMP socket */
		if ((g_data->icmp_sockets[i] = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		{
			fprintf(stderr, "Failed to create receiver's socket\n");
			return -1;
		}
		FD_SET(g_data->icmp_sockets[i], &g_data->icmpfds);
		i++;
	}
	return 0;
}

static void	clear_sockets(t_data *g_data)
{
	unsigned int i = 0;

	while (i < g_data->squeries) {
		/* UDP socket */
		FD_CLR(g_data->udp_sockets[i], &g_data->udpfds);
		close(g_data->udp_sockets[i]);

		/* ICMP socket */
		FD_CLR(g_data->icmp_sockets[i], &g_data->icmpfds);
		close(g_data->icmp_sockets[i]);
		i++;
	}
}

static int monitor_packet(t_data *g_data)
{
	printf("Select return: %d\n", select(g_data->squeries, &g_data->icmpfds,
		&g_data->udpfds, NULL, NULL));

	/* while (1) {
		if (select(max_sockets, &icmpfds, &udpfds, NULL, NULL)) {
		}
		if (FD_ISSET)
	} */

	(void)send_packet;
	(void)receive_packet;
	(void)print_packet;
	// print_packet(rec_packet, (struct sockaddr_in *)&receiver, ttl);

	return 0;
}

void traceroute_loop(t_data *g_data)
{
	int ret = 0;
	unsigned int hop = 0;

	printf("traceroute to %s (%s), %d hops max, %ld bytes packets\n",
		g_data->address, g_data->ipv4, g_data->hops,
		sizeof(struct iphdr)+sizeof(struct udphdr)+g_data->size);

	create_sockets(g_data); /* TODO: Error check */

	while (hop < g_data->hops) {
		ret = monitor_packet(g_data);
		if (ret < 0)
			break;
		hop++;
	}
	clear_sockets(g_data);
}
