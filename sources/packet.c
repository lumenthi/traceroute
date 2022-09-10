#include "traceroute.h"

static void print_packet(struct packet full_packet, struct sockaddr_in *receiver, t_data *g_data)
{
	printf("ttl: %d\n", full_packet.ip_hdr.ttl);
	printf("type: %d\n", full_packet.content.hdr.type);
	printf("id: %d\n", full_packet.content.hdr.un.echo.id);

	printf(" %d  %s (%s)\n",
		g_data->sequence, inet_ntoa(receiver->sin_addr), inet_ntoa(receiver->sin_addr));
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

	/* TODO: ERROR CHECKING */
	rsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	bind(rsocket, g_data->host_addr, sizeof(g_data->servaddr));

	/* Set packet's TTL */
	setsockopt(rsocket, SOL_IP, IP_TTL, &ttl, sizeof(ttl));
	/* Real traceroute sets no flags in header set IP_PMTUDISC_DONT to allow fragmentation */
	setsockopt(rsocket, IPPROTO_IP, IP_MTU_DISCOVER, &frag, sizeof(frag));

	if (sendto(rsocket, buf, sizeof(buf), 0,
		(struct sockaddr *)&g_data->servaddr, sizeof(*(g_data->host_addr))) <= 0)
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
	unsigned int ttl = 1; /* Set to 1 */
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
