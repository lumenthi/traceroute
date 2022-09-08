/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   traceroute.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lumenthi <lumenthi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/07 16:02:47 by lumenthi          #+#    #+#             */
/*   Updated: 2022/09/08 17:25:31 by lumenthi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

static int host_bind(t_data *g_data)
{
	/* IPv4 */
	g_data->servaddr.sin_family = AF_INET;
	g_data->servaddr.sin_addr.s_addr = INADDR_ANY;
	g_data->servaddr.sin_port = htons(g_data->port);

	g_data->host_addr = (struct sockaddr *)&g_data->servaddr;

	/* Bind with the server addr */
	if (!(bind(g_data->sockfd, g_data->host_addr,
		sizeof(g_data->servaddr))))
	{
		return 0;
	}
	return 1;
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

	g_data.port = 33434; /* Default traceroute port */
	g_data.path = path;
	g_data.args = args;
	g_data.address = destination;
	g_data.sequence = 0;
	g_data.hops = 30;

	/* default timeout (seconds) */
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	/* UDP socket */
	if ((g_data.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
		fprintf(stderr, "%s: %s: Failed to create socket\n", path, destination);
		return 1;
	}
	/* Resolving host */
	if (!(g_data.host_info = resolve(destination, &g_data))) {
		fprintf(stderr, "%s: %s: Name or service not known\n", path, destination);
		return 1;
	}
	/* g_data.host_info is allocated ! Must free it now */
	/* Getting informations about host */
	if (host_bind(&g_data)) {
		fprintf(stderr, "%s: %s: Failed to bind port %d\n",
			path, destination, g_data.port);
		freeaddrinfo(g_data.host_info);
		return 1;
	}

	/* Setting timeout option */
	if (setsockopt(g_data.sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) != 0) {
		fprintf(stderr, "%s: %s: Failed to set timeout option\n", path, destination);
		freeaddrinfo(g_data.host_info);
		return 1;
	}

	traceroute_loop(&g_data);

	return 0;
}
