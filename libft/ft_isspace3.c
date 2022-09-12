#include "libft.h"

int	ft_isspace3(char a)
{
	if ((a >= 9 && a <= 13) || a == ' ')
		return (1);
	return (0);
}
