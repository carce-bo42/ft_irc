#include "libft.h"

char	*ft_strrchr(const char *s, int c)
{
	size_t	len;
	char	*aux;

	aux = (char *)s;
	len = ft_strlen(s) + 1;
	while (len > 0)
	{
		if (*(aux + --len) == (char)c)
			return (aux + len);
	}
	return (NULL);
}
