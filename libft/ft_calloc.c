#include "libft.h"

void	*ft_calloc(size_t count, size_t size)
{
	void	*ptr;
	void	*aux;
	size_t	len;

	len = count * size;
	ptr = malloc(len);
	if (!ptr)
		return (NULL);
	aux = ptr;
	while (len-- > 0)
		*(unsigned char *)aux++ = '\0';
	return (ptr);
}
