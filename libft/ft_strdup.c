#include "libft.h"

char	*ft_strdup(const char *src)
{
	char	*output_str;
	int		i;

	output_str = (char *) malloc(sizeof (*src) * (ft_strlen(src) + 1));
	if (!output_str)
		return (NULL);
	i = 0;
	while (src[i] != '\0')
	{
		output_str[i] = src[i];
		i++;
	}
	output_str[i] = '\0';
	return (output_str);
}
