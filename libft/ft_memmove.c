#include "libft.h"

void	*ft_memmove(void *dst, const void *src, size_t len)
{
	const unsigned char	*aux;
	unsigned char		*aux_dst;

	aux = (const unsigned char *)src;
	aux_dst = (unsigned char *)dst;
	if (!dst && !src)
		return (NULL);
	if (dst > src)
		while (len-- > 0)
			*(aux_dst + len) = *(aux + len);
	else
		while (len-- > 0)
			*(aux_dst++) = *(aux++);
	return (dst);
}
