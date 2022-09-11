#include "traceroute.h"

static void print_packet(t_data *g_data, struct packet full_packet,
	struct sockaddr_in *receiver)
{
	(void)full_packet;
	// printf("ttl: %d\n", full_packet.ip_hdr.ttl);
	// printf("type: %d\n", full_packet.content.hdr.type);
	// printf("id: %d\n", full_packet.content.hdr.un.echo.id);

	printf(" %d  %s (%s)\n",
		g_data->ttl, inet_ntoa(receiver->sin_addr), inet_ntoa(receiver->sin_addr));
}

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

	/* Set packet's TTL */
	if (setsockopt(rsocket, SOL_IP, IP_TTL, &g_data->ttl,
		sizeof(g_data->ttl)) != 0) {
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
		return -1;
	}

	g_data->queries[CURRENT_QUERY].port = g_data->port;
	g_data->queries[CURRENT_QUERY].status = SENT;

	return rsocket;
}

static int queries_informations(t_data *g_data, struct packet full_packet,
	struct sockaddr_in *receiver)
{
	unsigned int port;
	struct iphdr *ip_hdr;
	struct udphdr *udp_hdr;

	ip_hdr = (struct iphdr *)(&full_packet.content.msg);
	udp_hdr = (struct udphdr *)((void *)ip_hdr+sizeof(struct iphdr));

	port = ntohs(udp_hdr->dest);

	printf("Index: %d\n", CURRENT_QUERY);
	printf("Port: %d\n", port);
	printf("Time to live: %d\n", ip_hdr->ttl);
	// printf("Total: %d\n", g_data->tqueries);
	// printf("Adddr: %s\n", inet_ntoa(((struct sockaddr_in *)&receiver)->sin_addr));
	// printf("Port: %d\n", ((struct sockaddr_in *)&receiver)->sin_port);

	/* TODO: Must find the right index by looking for port */

	ft_strncpy(
		g_data->queries[CURRENT_QUERY].ipv4,
		inet_ntoa(((struct sockaddr_in *)&receiver)->sin_addr),
		INET_ADDRSTRLEN
	);
	g_data->queries[CURRENT_QUERY].status = RECEIVED;
	return 0;
}

static int receive_packet(t_data *g_data, int rsocket)
{
	struct packet	rec_packet;
	struct sockaddr receiver;
	socklen_t		receiver_len;
	/* default timeout (seconds) */
	struct timeval timeout = {5, 0};

	/* Set receive timeout */
	if (setsockopt(rsocket, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)&timeout, sizeof(timeout)) != 0) {
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
				0,
				&receiver,
				&receiver_len) <= 0)
	{
		fprintf(stderr, "Failed to receive packet\n");
		return -1;
	}

	queries_informations(g_data, rec_packet, (struct sockaddr_in *)&receiver);

	(void)print_packet;
	// print_packet(g_data, rec_packet, (struct sockaddr_in *)&receiver);

	return 0;
}

static int	 create_sockets(t_data *g_data)
{
	unsigned int i = 0;
	FD_ZERO(&g_data->udpfds);
	FD_ZERO(&g_data->icmpfd);

	while (i < g_data->squeries) {
		/* UDP socket */
		if ((g_data->udp_sockets[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		{
			fprintf(stderr, "Failed to create sender's socket\n");
			return -1;
		}
		FD_SET(g_data->udp_sockets[i], &g_data->udpfds);
		g_data->maxfd = g_data->udp_sockets[i] > g_data->maxfd ?
			g_data->udp_sockets[i] : g_data->maxfd;
		i++;
	}
	/* ICMP socket */
	if ((g_data->icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		fprintf(stderr, "Failed to create receiver's socket\n");
		return -1;
	}
	FD_SET(g_data->icmp_socket, &g_data->icmpfd);
	g_data->maxfd = g_data->icmp_socket > g_data->maxfd ?
		g_data->icmp_socket : g_data->maxfd;
	return 0;
}

static void	clear_sockets(t_data *g_data)
{
	unsigned int i = 0;

	while (i < g_data->squeries) {
		/* UDP socket */
		FD_CLR(g_data->udp_sockets[i], &g_data->udpfds);
		close(g_data->udp_sockets[i]);
		i++;
	}
	/* ICMP socket */
	FD_CLR(g_data->icmp_socket, &g_data->icmpfd);
	close(g_data->icmp_socket);
}

static int	udp_iterate(t_data *g_data)
{
	unsigned int i = 0;
	while (i < g_data->squeries) {
		/* Sent all packets */
		if (g_data->port - g_data->sport >= g_data->tqueries)
			return 1;
		if (FD_ISSET(g_data->udp_sockets[i], &g_data->udpfds)) {
			send_packet(g_data, g_data->udp_sockets[i]); /* TODO: Error check */
			g_data->port++;
			g_data->ttl = g_data->sttl+1 + ((g_data->port - g_data->sport) / 3);
		}
		i++;
	}
	return 0;
}

static int	icmp_receive(t_data *g_data)
{
	unsigned int i = 0;

	if (FD_ISSET(g_data->icmp_socket, &g_data->icmpfd)) {
		while (i < g_data->squeries) {
			receive_packet(g_data, g_data->icmp_socket); /* TODO: Error check */
			i++;
		}
	}
	return 0;
}

static int print_everything(t_data *g_data)
{
	(void)g_data;
	if (CURRENT_QUERY >= g_data->tqueries)
		return 1; /* TODO: Remove, for debug until implemented */
	return 0;
}

static int monitor_packet(t_data *g_data)
{
	printf("[*] Iteration\n");

	/* TODO: Potential infinite selects, set timeout */
	if (select(g_data->maxfd+1, NULL,
		&g_data->udpfds, NULL, NULL)) {
		udp_iterate(g_data); /* TODO: Error check */
	}

	if (select(g_data->maxfd+1, &g_data->icmpfd,
		NULL, NULL, NULL)) {
		icmp_receive(g_data); /* TODO: Error check */
	}
	/* TODO: Print stop condition */
	return print_everything(g_data);
}

void traceroute_loop(t_data *g_data)
{
	int ret = 0;

	printf("traceroute to %s (%s), %d hops max, %ld bytes packets\n",
		g_data->address, g_data->ipv4, g_data->hops,
		sizeof(struct iphdr)+sizeof(struct udphdr)+g_data->size);

	create_sockets(g_data); /* TODO: Error check */

	while (!(ret = monitor_packet(g_data))) {
	}

	clear_sockets(g_data);
}
