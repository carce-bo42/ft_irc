#include "libft.h"

static int	ft_recursive_power(int nb, int power)
{
	int	n;

	if (power != 0)
		n = nb * ft_recursive_power(nb, power - 1);
	else
		n = 1;
	return (n);
}

static int	nbr_dig(int n)
{
	int	digits;
	int	boolean;

	digits = 9;
	boolean = 0;
	if (n == 0)
		return (1);
	while (boolean == 0)
	{
		if (n / ft_recursive_power(10, digits) != 0)
		{
			digits = digits + 1;
			boolean = 1;
		}
		else
			digits--;
	}
	return (digits);
}

static void	ft_subitoa(int n, int digits, char *aux)
{
	unsigned int	n_aux;
	int				sign;

	sign = 1;
	if (n < 0)
		sign = -1;
	n_aux = (unsigned int)(n * sign);
	while (digits > 0)
	{
		if (digits > 1)
			*aux = (n_aux / ft_recursive_power(10, digits - 1)) + '0';
		else
			*aux = (n_aux % 10) + '0';
		n_aux = n_aux - (*aux++ - '0') * ft_recursive_power(10, digits-- - 1);
	}
	*aux = '\0';
}

char	*ft_itoa(int n)
{
	int		is_negative;
	int		digits;
	char	*out;
	char	*aux;

	is_negative = 0;
	if (n < 0)
		is_negative = 1;
	digits = nbr_dig(n);
	out = (char *)malloc(digits + is_negative + 1);
	if (!out)
		return (NULL);
	aux = out;
	if (is_negative == 1)
		*aux++ = '-';
	ft_subitoa(n, digits, aux);
	return (out);
}
