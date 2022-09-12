#include "libft.h"

size_t	ft_strlcpy(char *dest, const char *src, size_t size)
{
	size_t	len;

	if (!src || !dest)
		return (0);
	len = ft_strlen(src);
	if (size == 0)
		return (len);
	while (size-- > 1 && *src)
		*dest++ = *src++;
	*dest = '\0';
	return (len);
}
