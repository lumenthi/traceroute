#include "traceroute.h"

int print_help()
{
	printf("Usage\n"
		"  traceroute [options] <destination>\n\n"
		"Options:\n"
		"  -f <first_ttl>     Start from the first_ttl hop (default is 1) [1-250]\n"
		"  -h                 print help and exit\n"
		"  -m <max_ttl>       Set the max number of hops (default is 30) [1-255]\n"
		"  -N <squeries>      Set the number of probes to be tried simultaneously (default is 16) [1-100]\n"
		"  -n                 Do not resolve IP addresses to their domain names\n"
		"  -p <port>          Set the destination port to use (default is 33434) [1-65535]\n"
		"  -q <nqueries>      Set the number of probes per each hop (default is 3) [1-10]\n");
	return 1;
}

static char *get_hostname(t_data *g_data, char *ipv4)
{
	/* Bonus */
	struct sockaddr_in sa;
	socklen_t len;

	ft_memset(&sa, 0, sizeof(struct sockaddr_in));

	/* For IPv4*/
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(ipv4);
	len = sizeof(struct sockaddr_in);

	if (getnameinfo((struct sockaddr *)&sa, len, g_data->host, NI_MAXHOST,
		NULL, 0, NI_NAMEREQD))
		return ipv4;
	return g_data->host;
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

static void print_query(t_query querry, unsigned int counter, unsigned int probe)
{
	long int sec = querry.end_time.tv_sec - querry.start_time.tv_sec;
	long int usec = querry.end_time.tv_usec - querry.start_time.tv_usec;

	long long total_usec = sec*1000000+usec;

	// printf("[*] Print query");

	if (querry.status == SENT) {
		printf("*");
		if (counter < probe-1)
			printf(" ");
	}
	else {
		printf(" %lld.%03lld ms", total_usec/1000, total_usec%1000);
		if (counter < probe-1)
			printf(" ");
	}
}

int print_everything(t_data *g_data)
{
	t_query *queries = g_data->queries;
	unsigned int i = 0;

	sort_queries(g_data);

	while (i < g_data->tqueries) {
		if (queries[i].status != DISPLAYED && queries[i].status != NOT_USED) {
			if (g_data->tprobe != queries[i].ttl) {
				if (g_data->cprobe != g_data->sttl)
					printf("\n");
				g_data->tprobe = queries[i].ttl;
				ft_strncpy(g_data->aprobe, queries[i].ipv4, INET_ADDRSTRLEN);
				if (g_data->cprobe < 10)
					printf(" %d  ", g_data->cprobe);
				else
					printf("%d  ", g_data->cprobe);
				if (queries[i].status != SENT) {
					if (g_data->args & 0x10)
						printf("%s ", g_data->aprobe);
					else
						printf("%s (%s) ", get_hostname(g_data, g_data->aprobe), g_data->aprobe);
				}
				g_data->cttl = 0;
				g_data->cprobe++;
			}
			else if (ft_strcmp(g_data->aprobe, queries[i].ipv4)) {
				ft_strncpy(g_data->aprobe, queries[i].ipv4, INET_ADDRSTRLEN);
				if (queries[i].status != SENT) {
					if (g_data->args & 0x10)
						printf("%s ", g_data->aprobe);
					else
						printf("%s (%s) ", get_hostname(g_data, g_data->aprobe), g_data->aprobe);
				}
			}
			if (g_data->cttl < g_data->probe) {
				print_query(queries[i], g_data->cttl, g_data->probe);
				if (queries[i].status == RECEIVED_END && g_data->cttl == g_data->probe-1) {
					printf("\n");
					return 1;
				}
				g_data->cttl++;
			}
			queries[i].status = DISPLAYED;
		}
		i++;
	}
	/* TODO: Remove void casts */
	if (CURRENT_QUERY >= g_data->tqueries || g_data->reached) {
		printf("\n");
		return 1;
	}
	return 0;
}
