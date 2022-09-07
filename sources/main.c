/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lumenthi <lumenthi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/20 11:15:03 by lumenthi          #+#    #+#             */
/*   Updated: 2022/09/07 17:39:42 by lumenthi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "traceroute.h"

static int		get_args(int argc, char **argv, uint8_t *args)
{
	int i = 1;
	int j = 0;
	int ret = 0;

	*args = 0x00;
	while (i < argc) {
		if (argv[i] && argv[i][0] == '-') {
			j = 0;
			while (argv[i][j]) {
				if (argv[i][j] == 'h')
					(*args) |= 0x01; // 0000 0001
				j++;
			}
		}
		else
			ret = i;
		i += 1;
	}
	return ret;
}

int main(int argc, char **argv)
{
	int arg_index = 0;
	char *destination = NULL;
	uint8_t args = 0;

	arg_index = get_args(argc, argv, &args);
	if (argc > 1 && arg_index)
		destination = argv[arg_index];

	ft_traceroute(destination, args, argv[0]);
	return 0;
}
