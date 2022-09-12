#include "libft.h"

int	ft_isalnum(int c)
{
	if (ft_isdigit(c) == 0 && ft_isalpha(c) == 0)
		return 0;
	return c;
}
