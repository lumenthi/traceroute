#include "traceroute.h"

static void print_packet(struct packet full_packet, struct sockaddr_in *receiver, t_data *g_data)
{
	unsigned int i = 0;
	uint8_t *tmp = (uint8_t*)&full_packet;
	while (i < sizeof(full_packet))
	{
		printf("%x ", tmp[i]);
		i++;
	}

	printf("ttl: %d\n", full_packet.ip_hdr.ttl);
	printf("type: %d\n", full_packet.content.hdr.type);
	printf("id: %d\n", full_packet.content.hdr.un.echo.id);

	printf(" %d  %s (%s)\n",
		g_data->sequence, inet_ntoa(receiver->sin_addr), inet_ntoa(receiver->sin_addr));
}

static int send_packet(t_data *g_data, unsigned int ttl, unsigned int port)
{
	int		rsocket;
	char	buf[g_data->size];
	unsigned int i = 0;

	/* Packet content */
	while (i < g_data->size)
	{
		buf[i] = 0x40+i;
		i++;
	}

	g_data->servaddr.sin_family = AF_INET;
	g_data->servaddr.sin_addr.s_addr = INADDR_ANY;
	g_data->servaddr.sin_port = htons(port);

	g_data->host_addr = (struct sockaddr *)&g_data->servaddr;

	/* TODO: ERROR CHECKING */

	rsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	bind(rsocket, g_data->host_addr, sizeof(g_data->servaddr));
	// setsockopt(socket, SOL_IP, IP_MTU_DISCOVER, [0], 4) = 0
	// setsockopt(socket, SOL_SOCKET, SO_TIMESTAMP, [1], 4) = 0
	// setsockopt(3, SOL_IP, IP_RECVTTL, [1], 4) = 0
	// setsockopt(rsocket, SOL_IP, IP_RECVERR, [1], 4) = 0
	setsockopt(rsocket, SOL_IP, IP_TTL, &ttl, sizeof(ttl));
	if (sendto(rsocket, buf, sizeof(buf), 0, g_data->host_addr, sizeof(*(g_data->host_addr))) <= 0)
	{
		fprintf(stderr, "Failed to send packet\n");
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
	setsockopt(rsocket, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)&timeout, sizeof(timeout));

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

static int monitor_packet(t_data *g_data, unsigned int ttl, unsigned int port)
{
	int					socket;
	struct packet		rec_packet;
	struct sockaddr		receiver;

	if ((socket = send_packet(g_data, ttl, port)) < 0)
		return -1;

	/* Manual (debug) */
	/* ft_memset(&rec_packet, 0, sizeof(rec_packet));
	receiver_len = sizeof(receiver);
	ft_memset(&receiver, 0, receiver_len);
	if (recvfrom(socket,
				&rec_packet,
				sizeof(rec_packet),
				0,
				&receiver,
				&receiver_len) <= 0)
	{
		fprintf(stderr, "Failed to receive packet\n");
		return -1;
	} */

	// (void)receive_packet;
	if (receive_packet(&rec_packet, &receiver) < 0) {
		close(socket);
		return -1;
	}

	print_packet(rec_packet, (struct sockaddr_in *)&receiver, g_data);
	close(socket);
	return 0;
}

void traceroute_loop(t_data *g_data)
{
	unsigned int ttl = 64; /* Set to 1 */
	unsigned int lim = ttl+2;
	unsigned int port = 33434; /* Default port */
	int ret = 0;

	printf("traceroute to %s (%s), %d hops max, %ld bytes packets\n",
		g_data->address, g_data->ipv4, g_data->hops,
		sizeof(struct iphdr)+sizeof(struct udphdr)+g_data->size);

	while (ttl < lim) {
		ret = monitor_packet(g_data, ttl, port);
		if (ret < 0)
			break;
		ttl++;
		port++;
	}
}
