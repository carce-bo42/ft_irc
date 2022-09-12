#include "libft.h"

void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	unsigned char		*aux_dst;
	const unsigned char	*aux_src;

	aux_dst = (unsigned char *)dst;
	aux_src = (const unsigned char *)src;
	if (n == 0 || (aux_src == aux_dst))
		return (dst);
	while (n-- > 0)
		*(aux_dst++) = *(aux_src++);
	return (dst);
}
