#include "libft.h"

int	ft_isascii(int c)
{
	int	boolean;

	boolean = 1;
	if (c < 0 || c > 127)
		boolean = 0;
	return (boolean);
}
