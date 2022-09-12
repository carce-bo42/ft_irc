#include "libft.h"

/*La particularidad de este ejercicio, que en absoluto se menciona
 * en el enunciado, es cómo usar la función del. Creo que es al gusto. En mi
 * caso, la utilizo cuando la reserva de memoria para el nuevo elemento FALLA,
 * que entonces lo que hago es borrar la lista que estaba creando y devolverla
 * como NULL.*/
t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*new_element;
	t_list	*new_lst;

	if (!f || !lst)
		return (NULL);
	new_lst = NULL;
	while (lst)
	{
		new_element = ft_lstnew(f(lst->content));
		if (!new_element)
		{
			ft_lstclear(&new_lst, del);
			return (new_lst);
		}
		ft_lstadd_back(&new_lst, new_element);
		lst = lst->next;
	}
	return (new_lst);
}
