#include "libft.h"

char	*ft_strtrim(const char *s1, const char *set)
{
	int		start;
	int		end;
	size_t	len;

	if (!s1 || !set)
		return (NULL);
	start = 0;
	end = 0;
	len = ft_strlen(s1);
	while (ft_strchr(set, (int)*(s1 + start)) && *(s1 + start))
		start++;
	if (!*(s1 + start))
		return (ft_strdup(""));
	while (ft_strchr(set, (int)*(s1 + len - 1 - end)) && *(s1 + len - 1 - end))
		end++;
	return (ft_substr(s1, start, len - start - end));
}
