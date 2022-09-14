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

int print_help()
{
	printf("Usage\n"
		"  traceroute [options] <destination>\n\n"
		"Options:\n"
		"  -f <first_ttl>     Start from the first_ttl hop (default is 1) [1-250]\n"
		"  -h                 print help and exit\n"
		"  -m <max_ttl>       Set the max number of hops (default is 30) [1-255]\n"
		"  -N <squeries>      Set the number of probes to be tried simultaneously (default is 16)\n"
		"  -n                 Do not resolve IP addresses to their domain names\n"
		"  -p <port>          Set the destination port to use (default is 33434)\n"
		"  -q <nqueries>      Set the number of probes per each hop (default is 3)\n");
	return 1;
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

int ft_traceroute(char *destination, uint8_t args, char *path, t_data g_data)
{
	if (args == ARGS_INVALID) {
		fprintf(stderr, "%s: Invalid argument detected\n", path);
		return print_help();
	}

	if (!destination) {
		fprintf(stderr, "%s: Empty hostname\n", path);
		return print_help();
	}

	if (ARGS_h)
		return print_help();


	/* Options */
	g_data.path = path;
	g_data.args = args;
	g_data.address = destination;
	g_data.size = 32; /* Packet's content size */
	if (!(ARGS_m))
		g_data.hops = 30; /* 30 */

	g_data.sport = 33434; /* Starting port */
	g_data.port = g_data.sport; /* Port we will increment */

	if (!(ARGS_f))
		g_data.sttl = 1; /* Starting ttl */
	g_data.ttl = g_data.sttl; /* TTL we will increment */

	if (g_data.hops < g_data.ttl) {
		fprintf(stderr, "%s: First hop out of range\n", path);
		return 1;
	}

	/* Total queries */
	g_data.tqueries = ((g_data.hops-g_data.sttl) * 3) + 1;

	/* Simultaneous queries calculation */
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
