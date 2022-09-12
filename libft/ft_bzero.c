#include "libft.h"

void	ft_bzero(void *s, size_t n)
{
	unsigned char	*aux;

	aux = (unsigned char *)s;
	while (n-- > 0)
		*(aux++) = '\0';
}
