#include "traceroute.h"

static int get_f(int argc, char **argv, t_data *g_data, int i)
{
	if (i+1 < argc) {
		g_data->sttl = ft_atoi(argv[i+1]);
		if (g_data->sttl < 1 || g_data->sttl > 250) {
			fprintf(stderr, "%s: Invalid value for <first_ttl>\n", argv[0]);
			return 0;
		}
	}
	else {
		fprintf(stderr, "%s: Empty value for <first_ttl>\n", argv[0]);
		return 0;
	}
	return 1;
}

static int get_m(int argc, char **argv, t_data *g_data, int i)
{
	if (i+1 < argc) {
		g_data->hops = ft_atoi(argv[i+1]);
		if (g_data->hops < 1 || g_data->hops > 255) {
			fprintf(stderr, "%s: Invalid value for <max_ttl>\n", argv[0]);
			return 0;
		}
	}
	else {
		fprintf(stderr, "%s: Empty value for <max_ttl>\n", argv[0]);
		return 0;
	}
	return 1;
}

static int get_N(int argc, char **argv, t_data *g_data, int i)
{
	if (i+1 < argc) {
		g_data->squeries = ft_atoi(argv[i+1]);
		/* TODO: Set a positive value limit */
		if (g_data->squeries < 1 || g_data->squeries > 100) {
			fprintf(stderr, "%s: Invalid value for <squeries>\n", argv[0]);
			return 0;
		}
	}
	else {
		fprintf(stderr, "%s: Empty value for <squeries>\n", argv[0]);
		return 0;
	}
	return 1;
}

static int get_args(int argc, char **argv, uint8_t *args, t_data *g_data)
{
	int i = 1;
	int j = 0;
	int ret = 0;
	int next;

	*args = 0x00;
	while (i < argc) {
		next = 0;
		if (argv[i] && argv[i][0] == '-') {
			j = 0;
			while (argv[i][j]) {
				if (argv[i][j] == 'h')
					(*args) |= 0x01; // 0000 0001
				else if (argv[i][j] == 'f') {
					(*args) |= 0x02; // 0000 0010
					if (!get_f(argc, argv, g_data, i))
						return -1;
					next = 1;
				}
				else if (argv[i][j] == 'm') {
					(*args) |= 0x04; // 0000 0100
					if (!get_m(argc, argv, g_data, i))
						return -1;
					next = 1;
				}
				else if (argv[i][j] == 'N') {
					(*args) |= 0x08; // 0000 1000
					if (!get_N(argc, argv, g_data, i))
						return -1;
					next = 1;
				}
				else if (argv[i][j] != '-')
					(*args) = ARGS_INVALID;
				j++;
			}
		}
		else
			ret = i;
		i += next ? 2 : 1;
	}
	return ret;
}

int main(int argc, char **argv)
{
	int arg_index = 0;
	char *destination = NULL;
	uint8_t args = 0;
	t_data g_data = {0};

	arg_index = get_args(argc, argv, &args, &g_data);
	if (arg_index < 0)
		return print_help();
	if (arg_index > 0)
		destination = argv[arg_index];

	ft_traceroute(destination, args, argv[0], g_data);
	return 0;
}
