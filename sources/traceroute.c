#include "traceroute.h"

static int resolve(char *host, t_data *g_data)
{
	struct addrinfo hints;

	if (!ft_memset(&hints, 0, sizeof(struct addrinfo)))
		return 1;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	/* subject to any restrictions imposed by hints */
	if (getaddrinfo(host, NULL, &hints, &g_data->host_info) == -1 ||
		g_data->host_info == NULL)
		return 1;

	g_data->host_addr = g_data->host_info->ai_addr;
	g_data->servaddr = *(struct sockaddr_in *)g_data->host_addr;

	ft_strncpy(g_data->ipv4, inet_ntoa(g_data->servaddr.sin_addr),
		sizeof(g_data->ipv4));

	return 0;
}

int end(t_data *g_data, int code)
{
	/* Freeing stuff */
	if (g_data->udp_sockets)
		free(g_data->udp_sockets);
	if (g_data->queries)
		free(g_data->queries);
	if (g_data->host_info)
		freeaddrinfo(g_data->host_info);

	return code;
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

static void traceroute_loop(t_data *g_data)
{
	int ret = 0;

	printf("traceroute to %s (%s), %d hops max, %ld bytes packets\n",
		g_data->address, g_data->ipv4, g_data->hops,
		sizeof(struct iphdr)+sizeof(struct udphdr)+g_data->size);

	if ((create_sockets(g_data)) == -1)
		return;

	while (!(ret = monitor_packet(g_data)));

	clear_sockets(g_data);
}

int ft_traceroute(char *destination, uint8_t args, char *path, t_data g_data)
{
	if (!destination) {
		fprintf(stderr, "%s: Empty hostname\n", path);
		return print_help();
	}

	/* Options */
	g_data.path = path;
	g_data.args = args;
	g_data.address = destination;
	g_data.size = 32; /* Packet's content size */
	if (!(ARGS_m))
		g_data.hops = 30; /* 30 */

	if (!(ARGS_p))
		g_data.sport = 33434; /* Starting port */
	g_data.port = g_data.sport; /* Port we will increment */

	if (!(ARGS_f))
		g_data.sttl = 1; /* Starting ttl */
	g_data.ttl = g_data.sttl; /* TTL we will increment */

	if (!(ARGS_q))
		g_data.probe = 3; /* Default probe per hop */

	if (g_data.hops < g_data.ttl) {
		fprintf(stderr, "%s: First hop out of range\n", path);
		return 1;
	}

	/* Total queries */
	g_data.tqueries = ((g_data.hops-g_data.sttl+1) * g_data.probe);

	/* Simultaneous queries calculation */
	if (!(ARGS_N))
		g_data.squeries = 16; /* 16 */
	g_data.squeries = g_data.tqueries < g_data.squeries ?
		g_data.tqueries : g_data.squeries;

	g_data.maxfd = 0;
	g_data.host_info = NULL;

	g_data.reached = 0; /* Set to 1 when the server is reached */

	g_data.tprobe = 0;
	g_data.pend = 0;
	g_data.cprobe = g_data.sttl; /* Display related, current probe number */
	ft_bzero(g_data.aprobe, sizeof(g_data.aprobe)); /* Display related, IPv4 of current probe */

	/* Default timeout for a query (5 seconds) */
	g_data.timeout.tv_sec = 5;
	g_data.timeout.tv_usec = 0;

	/* printf("Simultaneous queries: %d\n", g_data.squeries); */

	if (getuid() != 0) {
		fprintf(stderr, "%s: %s: Not allowed to create raw sockets, run as root\n",
			path, destination);
		return 1;
	}

	g_data.args = args; /* Assigning args */

	g_data.udp_sockets = (int *)malloc(sizeof(int) * g_data.squeries);
	g_data.queries = (t_query *)malloc(sizeof(t_query) * (g_data.tqueries));
	ft_memset(g_data.queries, 0, sizeof(t_query) * (g_data.tqueries));
	if (!g_data.udp_sockets || !g_data.queries) {
		fprintf(stderr, "%s: %s: Malloc error\n", path, destination);
		return end(&g_data, 1);
	}

	/* Resolving host */
	if (resolve(destination, &g_data)) {
		fprintf(stderr, "%s: %s: Name or service not known\n", path, destination);
		return end(&g_data, 1);
	} /* g_data.host_info is allocated ! Must free it now */

	traceroute_loop(&g_data);

	return end(&g_data, 0);
}
