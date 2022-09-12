#include "libft.h"

char	*ft_strjoin(const char *s1, const char *s2)
{
	char	*str;
	char	*aux_str;

	if (!s1 || !s2)
		return (NULL);
	str = (char *)malloc(ft_strlen(s1) + ft_strlen(s2) + 1);
	if (!str)
		return (NULL);
	aux_str = str;
	while (*s1)
		*aux_str++ = *s1++;
	while (*s2)
		*aux_str++ = *s2++;
	*aux_str = '\0';
	return (str);
}
