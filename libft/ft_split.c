#include "libft.h"

static int	ft_wordcount(char *str, char delimiter)
{
	int	i;

	i = 0;
	if (!*str)
		return (0);
	if (*str != delimiter && *(str))
		i = 1;
	while (*(str + 1))
	{
		if (*str == delimiter && *(str + 1) != delimiter && *(str + 1) != '\0')
			i++;
		str++;
	}
	return (i);
}

static void	ft_subsplit(const char *s, char c, int n_words, char **out)
{
	int	i;
	int	j;
	int	k;
	int	l;

	i = 0;
	while (n_words-- > 0)
	{
		while (*(s + i) == c && *(s + i))
			i++;
		j = i;
		while (*(s + i) != c && *(s + i))
			i++;
		*out = (char *)malloc(sizeof(char) * (i - j + 1));
		k = 0;
		l = j;
		while (k + l < i)
		{
			*(*(out) + k++) = *(s + j++);
			*(*out + k) = '\0';
		}
		out++;
	}
	*out = NULL;
}

char	**ft_split(const char *s, char c)
{
	size_t		n_words;
	char		**out;

	if (!s)
		return (NULL);
	n_words = ft_wordcount((char *)s, c);
	out = (char **)malloc(sizeof(char *) * (n_words + 1));
	if (!out)
		return (NULL);
	ft_subsplit(s, c, n_words, out);
	return (out);
}
/*
int main()
{
	printf("|%s|\n", ft_split("", ' '));
	return 0;
}*/
