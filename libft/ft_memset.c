#include "libft.h"

void	*ft_memset(void *b, int c, size_t len)
{
	unsigned char	a;
	unsigned char	*aux;

	aux = (unsigned char *)b;
	a = (unsigned char)c;
	while (len-- > 0)
		*(aux++) = a;
	return (b);
}
