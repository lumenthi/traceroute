#include "traceroute.h"

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
					if (i+1 < argc) {
						next = 1;
						if (!ft_strisnum(argv[i+1])) {
							fprintf(stderr, "%s: Bad timing interval\n", argv[0]);
							return -1;
						}
						/* TODO: Negative/Too big value check */
						g_data->sttl = ft_atoi(argv[i+1]);
					}
					else {
						fprintf(stderr, "%s: Empty value for <first_ttl>\n", argv[0]);
						return -1;
					}
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
		return 1;
	if (arg_index > 0)
		destination = argv[arg_index];

	ft_traceroute(destination, args, argv[0], g_data);
	return 0;
}
