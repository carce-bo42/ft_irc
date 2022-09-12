#include "libft.h"

char	*ft_substr(const char *s, unsigned int start, size_t len)
{
	char	*aux_str;
	char	*str;

	if (!s)
		return (NULL);
	if (start > ft_strlen(s) - 1)
		return (ft_strdup(""));
	if (len > ft_strlen(s) - start - 1)
		len = ft_strlen(s) - start ;
	str = (char *)malloc((len + 1) * sizeof(char));
	if (!str)
		return (NULL);
	aux_str = str;
	while (len-- > 0 && *(s + start))
		*aux_str++ = *(s++ + start);
	*aux_str = '\0';
	return (str);
}
