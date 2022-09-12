#include "libft.h"

int	ft_isprint(int c)
{
	int	boolean;

	boolean = 1;
	if (c < ' ' || c >= 127)
		boolean = 0;
	return (boolean);
}
