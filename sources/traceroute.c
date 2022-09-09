#include "traceroute.h"

static struct addrinfo *resolve(char *host, t_data *g_data)
{
	struct addrinfo hints;
	struct addrinfo *res;

	if (!ft_memset(&hints, 0, sizeof(struct addrinfo)))
		return NULL;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	/* subject to any restrictions imposed by hints */
	if (getaddrinfo(host, NULL, &hints, &res) == -1)
		return NULL;

	struct sockaddr_in *tmp = (struct sockaddr_in *)(res->ai_addr);
	ft_strncpy(g_data->ipv4, inet_ntoa(tmp->sin_addr), sizeof(g_data->ipv4));

	return res;
}

int print_help()
{
	printf("* Printing help *\n");
	return 1;
}

int ft_traceroute(char *destination, uint8_t args, char *path)
{
	t_data g_data = {0};


	if (!destination || ARGS_H)
		return print_help();

	g_data.path = path;
	g_data.args = args;
	g_data.address = destination;
	g_data.sequence = 0;
	g_data.hops = 30;

	/* Resolving host */
	if (!(g_data.host_info = resolve(destination, &g_data))) {
		fprintf(stderr, "%s: %s: Name or service not known\n", path, destination);
		return 1;
	}
	/* g_data.host_info is allocated ! Must free it now */

	traceroute_loop(&g_data);

	return 0;
}
