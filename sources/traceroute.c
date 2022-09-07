/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   traceroute.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lumenthi <lumenthi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/07 16:02:47 by lumenthi          #+#    #+#             */
/*   Updated: 2022/09/07 19:29:14 by lumenthi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "traceroute.h"

static struct addrinfo *resolve(char *host)
{
	struct addrinfo hints;
	struct addrinfo *res;

	if (!ft_memset(&hints, 0, sizeof(struct addrinfo)))
		return NULL;
	hints.ai_family = AF_INET;

	/* subject to any restrictions imposed by hints */
	if (getaddrinfo(host, NULL, &hints, &res) == -1)
		return NULL;

	return res;
}

static int host_informations(t_data *g_data_addr)
{
	t_data g_data = *g_data_addr;
	struct addrinfo *ret = g_data.host_info;

	if (!(inet_ntop(AF_INET,
					&((const struct sockaddr_in *)ret->ai_addr)->sin_addr,
					g_data.ipv4,
					sizeof(g_data.ipv4))))
	{
		return 1;
	}
	else {
		g_data.host_addr = ret->ai_addr;
		return 0;
	}
	return 0;
}

int print_help()
{
	printf("* Printing help *\n");
	return 1;
}

int ft_traceroute(char *destination, uint8_t args, char *path)
{
	t_data g_data = {0};
	struct timeval timeout;

	if (!destination || ARGS_H)
		return print_help();

	g_data.path = path;
	g_data.args = args;
	g_data.address = destination;

	/* default timeout (seconds) */
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	/* Socket creation RAW/DGRAM ? */
	if ((g_data.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
		fprintf(stderr, "%s: %s: Failed to create socket\n", path, destination);
		return 1;
	}
	/* Resolving host */
	if (!(g_data.host_info = resolve(destination))) {
		fprintf(stderr, "%s: %s: Name or service not known\n", path, destination);
		return 1;
	}
	/* g_data.host_info is allocated ! Must free it now */
	/* Getting informations about host */
	if (host_informations(&g_data)) {
		fprintf(stderr, "%s: %s: Failed to get informations about the host\n", path, destination);
		freeaddrinfo(g_data.host_info);
		return 1;
	}

	/* Setting timeout option */
	if (setsockopt(g_data.sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) != 0) {
		fprintf(stderr, "%s: %s: Failed to set timeout option\n", path, destination);
		freeaddrinfo(g_data.host_info);
		return 1;
	}

	traceroute_loop(g_data);

	return 0;
}
