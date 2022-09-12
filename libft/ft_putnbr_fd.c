#include "libft.h"

void	ft_putnbr_fd(int n, int fd)
{
	if (n == -2147483648)
		ft_putstr_fd("-2147483648", fd);
	else
	{
		if (n < 0)
		{
			n = n * (-1);
			ft_putchar_fd('-', fd);
		}
		if (n < 10)
		{
			n = n + '0';
			ft_putchar_fd((char)n, fd);
		}
		else
		{
			ft_putnbr_fd((n / 10), fd);
			ft_putnbr_fd((n % 10), fd);
		}
	}
}
