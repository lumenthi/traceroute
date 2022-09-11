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
	printf("* Printing help *\n");
	return 1;
}

int	end(t_data *g_data, int code)
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

int ft_traceroute(char *destination, uint8_t args, char *path)
{
	t_data g_data = {0};

	if (!destination || ARGS_H)
		return print_help();

	/* Options */
	g_data.path = path;
	g_data.args = args;
	g_data.address = destination;
	g_data.size = 32; /* Packet's content size */
	g_data.hops = 3; /* 30 */

	/* Total queries */
	g_data.tqueries = g_data.hops * 3;

	/* Simultaneous queries calculation */
	g_data.squeries = 1; /* 16 */
	g_data.squeries = g_data.tqueries < g_data.squeries ?
		g_data.tqueries : g_data.squeries;

	g_data.sport = 33434; /* Starting port */
	g_data.port = g_data.sport; /* Port we will increment */

	g_data.sttl = 1; /* Starting ttl */
	g_data.ttl = g_data.sttl; /* TTL we will increment */

	g_data.maxfd = 0;
	g_data.host_info = NULL;

	/* printf("Simultaneous queries: %d\n", g_data.squeries); */

	g_data.udp_sockets = (int *)malloc(sizeof(int) * g_data.squeries);
	/* Starting at index 1 TODO: Remove + 1 to malloc ? */
	g_data.queries = (t_query *)malloc(sizeof(t_query) * (g_data.tqueries + 1));
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
