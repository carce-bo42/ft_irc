#include "libft.h"

void	*ft_memchr(const void *s, int c, size_t n)
{
	unsigned char	*aux;

	aux = (unsigned char *)s;
	while (n-- > 0)
	{
		if (*aux == (unsigned char)c)
			return ((void *)aux);
		aux++;
	}
	return (NULL);
}
