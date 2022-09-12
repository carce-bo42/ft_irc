#include "libft.h"

void	*ft_memccpy(void *dst, const void *src, int c, size_t n)
{
	const unsigned char	*aux_src;
	unsigned char		*aux_dst;

	aux_src = (const unsigned char *)src;
	aux_dst = (unsigned char *)dst;
	if ((!*aux_src && !*aux_dst) || n == 0 || dst == src)
		return (NULL);
	while (n-- > 0)
	{
		*(aux_dst) = *(aux_src);
		if (*aux_src == (unsigned char)c)
			return ((void *)++aux_dst);
		aux_dst++;
		aux_src++;
	}
	return (NULL);
}
