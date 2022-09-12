#include "libft.h"

int	ft_numberlen(char *str)
{
	int	len;

	len = 0;
	if (*str == '+' || *str == '-')
		str++;
	while (*str == '0')
		str++;
	while (ft_isdigit(*str))
	{
		len++;
		str++;
	}
	return (len);
}
