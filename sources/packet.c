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

	// printf("[*] Sending: Index: %d\n", CURRENT_QUERY);
	// printf("[*] Sending: Max Index: %d\n", g_data->tqueries);
	g_data->queries[CURRENT_QUERY].port = g_data->port;
	g_data->queries[CURRENT_QUERY].ttl = g_data->ttl;
	g_data->queries[CURRENT_QUERY].status = SENT;

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

static void debug_queries(t_query *queries, unsigned int limit)
{
	unsigned int i = 0;
	char *status[] = {"NOT_USED", "SENT", "RECEIVED", "RECEIVED_END",
		"TIMEOUT", "NOT_DISPLAYED", "DISPLAYED"};

	while (i < limit) {
		printf("+----------------\n");
		printf("| Index: %d\n", i);
		printf("| IPv4: %s\n", queries[i].ipv4);
		printf("| Port: %d\n", queries[i].port);
		printf("| Time to live: %d\n", queries[i].ttl);
		printf("| Status: %s\n", status[queries[i].status]);
		printf("+----------------\n");
		i++;
	}
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

	icmp_hdr = (struct icmphdr *)(&full_packet.content.hdr);
	ip_hdr = (struct iphdr *)(&full_packet.content.msg);
	udp_hdr = (struct udphdr *)((void *)ip_hdr+sizeof(struct iphdr));

	port = ntohs(udp_hdr->dest);
	type = icmp_hdr->type;

	index = get_packet_index(g_data->queries, port, g_data->tqueries);
	/* Invalid packet */
	if (index >= g_data->tqueries) {
		// printf("[*] Dropping packet\n");
		return 0;
	}

	/* Debug gathered data */
	// printf("Index: %d\n", index);
	// printf("Port: %d\n", port);
	// printf("Time to live: %d\n", ip_hdr->ttl);
	// printf("Total queries: %d\n", g_data->tqueries);
	// printf("Adddr: %s\n", inet_ntoa(((struct sockaddr_in *)&receiver)->sin_addr));
	// printf("Port: %d\n", ((struct sockaddr_in *)&receiver)->sin_port);
	// printf("Status: %d\n", icmp_hdr->type);

	ft_strncpy(
		g_data->queries[index].ipv4,
		inet_ntoa(receiver->sin_addr),
		INET_ADDRSTRLEN
	);

	if (type == ICMP_TIME_EXCEEDED)
		g_data->queries[index].status = RECEIVED;
	else if (type == ICMP_DEST_UNREACH) {
		g_data->queries[index].status = RECEIVED_END;
		g_data->reached = 1;
	}

	return 0;
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
		return -1;
	}

	queries_informations(g_data, rec_packet, (struct sockaddr_in *)&receiver);

	return 0;
}

static int create_sockets(t_data *g_data)
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

static void clear_sockets(t_data *g_data)
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
		if (CURRENT_QUERY < g_data->tqueries &&
			FD_ISSET(g_data->udp_sockets[i], &g_data->udpfds)) {
			send_packet(g_data, g_data->udp_sockets[i]); /* TODO: Error check */
			g_data->port++;
			g_data->ttl = g_data->sttl + ((CURRENT_QUERY) / 3);
			g_data->sent++;
		}
		i++;
	}
	return 0;
}

static int	icmp_receive(t_data *g_data)
{
	unsigned int i = 0;

	if (FD_ISSET(g_data->icmp_socket, &g_data->icmpfd)) {
		while (i < g_data->sent) {
			receive_packet(g_data, g_data->icmp_socket); /* TODO: Error check */
			i++;
		}
	}
	return 0;
}

static void sort_queries(t_data *g_data)
{
	t_query *queries = g_data->queries;
	t_query tmp;
	unsigned int i = 0;
	unsigned int j = 0;

	while(i < g_data->tqueries) {
		j = 0;
		while (j < g_data->tqueries - 1) {
			/* SWAP */
			if (queries[j].port > queries[j+1].port) {
				tmp = queries[j];
				queries[j] = queries[j+1];
				queries[j+1] = tmp;
			}
			j++;
		}
		i++;
	}
}

static void print_query(t_query querry)
{
	if (querry.status == SENT)
		printf("* ");
	else
		printf("0.00 ms  ");
}

static int print_everything(t_data *g_data)
{
	t_query *queries = g_data->queries;
	unsigned int i = 0;

	(void)debug_queries;
	(void)sort_queries;
	sort_queries(g_data);
	// debug_queries(g_data->queries, g_data->tqueries);

	while (i < g_data->tqueries) {
		if (queries[i].status != DISPLAYED && queries[i].status != NOT_USED) {
			if (g_data->tprobe != queries[i].ttl) {
				if (g_data->cprobe != 1)
					printf("\n");
				g_data->tprobe = queries[i].ttl;
				ft_strncpy(g_data->aprobe, queries[i].ipv4, INET_ADDRSTRLEN);
				if (g_data->cprobe < 10)
					printf(" %d  ", g_data->cprobe);
				else
					printf("%d  ", g_data->cprobe);
				if (queries[i].status != SENT)
					printf("%s (%s)  ", g_data->aprobe, g_data->aprobe);
				g_data->cttl = 0;
				g_data->cprobe++;
			}
			else if (ft_strcmp(g_data->aprobe, queries[i].ipv4)) {
				ft_strncpy(g_data->aprobe, queries[i].ipv4, INET_ADDRSTRLEN);
				if (queries[i].status != SENT)
					printf("%s (%s)  ", g_data->aprobe, g_data->aprobe);
			}
			if (g_data->cttl < 3) {
				print_query(queries[i]);
				if (queries[i].status == RECEIVED_END && g_data->cttl == 2) {
					return 1;
				}
				g_data->cttl++;
			}
			queries[i].status = DISPLAYED;
		}
		i++;
	}
	/* TODO: Remove void casts */
	if (CURRENT_QUERY >= g_data->tqueries || g_data->reached)
		return 1;
	return 0;
}

static int monitor_packet(t_data *g_data)
{
	g_data->drop = 0;
	g_data->sent = 0;
	/* TODO: Potential infinite selects, set timeout */
	if (CURRENT_QUERY < g_data->tqueries && !g_data->reached &&
		select(g_data->maxfd+1, NULL, &g_data->udpfds, NULL, NULL))
	{
		/* TODO: REMOVE UNWANTED COMMENTS */
		// printf("[*] UDP iteration\n");
		udp_iterate(g_data); /* TODO: Error check */
	}
	// printf("[*] ICMP iteration\n");
	icmp_receive(g_data); /* TODO: Error check */

	return print_everything(g_data);
}

void traceroute_loop(t_data *g_data)
{
	int ret = 0;

	printf("traceroute to %s (%s), %d hops max, %ld bytes packets\n",
		g_data->address, g_data->ipv4, g_data->hops,
		sizeof(struct iphdr)+sizeof(struct udphdr)+g_data->size);

	if ((create_sockets(g_data)) == -1)
		return;

	while (!(ret = monitor_packet(g_data)));

	printf("\n");

	clear_sockets(g_data);
}
