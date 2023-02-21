#include "malloc.h"

// gaseste blocul corespunzator zonei de memorie data de pointerul care e returnat prin malloc

t_block *find_block(t_zone *zone, void *alloc)
{
	t_block *ptr;

	ptr = zone->blocks;
	while (ptr && ((void *)ptr + sizeof(t_block)) != alloc)
		ptr = ptr->next;
	return (ptr);
}

//creeaza si adauga blocul de memorie 

void *add_block(void *addr, size_t block_size, void *next)
{
	t_block *block;

	block = addr;
	block->size = block_size;
	block->next = next;
	return (block);
}

//insereaza un bloc la inceputul, interiorul sau sfarsitul unei zone depinzand de nr de bytes pe care trebuie sa-i alocam 

void *insert_block(t_zone *zone, size_t size)
{
	t_block *block;

	if (((void *)zone + sizeof(t_zone)) < zone->blocks && (size_t)(zone->blocks - ((void *)zone + sizeof(t_zone))) >= size)
	{
		block = add_block((void *)zone + sizeof(t_zone), size, zone->blocks);
		zone->blocks = block;
		return (block);
	}
	block = zone->blocks;
	while (block->next)
	{
		if ((size_t)(block->next - ((void *)block + block->size)) >= size)
		{
			return ((block->next = add_block((void *)block + block->size, size,block->next)));
		}
		block = block->next;
	}
	if (zone->size - (((void *)block + block->size) - (void *)zone) > size)
	{
		return ((block->next = add_block((void *)block + block->size, size , NULL)));
	}
	return (NULL);
}


//adauga blocul in zona, creeaza primul bloc sau returneaza null daca nu poate fi alocat


void	*add_block_to_zone(t_zone *zone, size_t size)
{
	size_t	block_size;
	int	size_taken;
	t_block *block;

	block = NULL;
	block_size = size + sizeof(t_block);
	if (!zone->blocks)
	{
		block = add_block((void *)zone + sizeof(t_zone), block_size, NULL);
		zone->blocks = block;
	}
	else
	{
		size_taken = get_size_taken_zone(zone);
		if (zone->size - size_taken > block_size)
			block = insert_block(zone, block_size);
	}
	if (!block)
		return (NULL);
	return ((void *)block + sizeof(t_block));
}

//sterge toata zona de memorie folosind munmap 

void	remove_zone(t_zone *zone)
{
	size_t	page_size;
	size_t	size;
	t_zone	*prev;
	t_zone	*tmp;

	prev = NULL;
	tmp = g_zones;
	while (tmp && tmp != zone)
	{
		prev = tmp;
		tmp = tmp->next;
	}
	page_size = getpagesize();
	size = (((SMALL_ALLOC + sizeof(t_block)) * (NB_ALLOC + 1)) / page_size)* page_size;
	if (prev || tmp->next || zone->size < size)
	{
		if (!prev)
			g_zones = tmp->next;
		else
			prev->next = tmp->next;
		munmap(zone, zone->size);
	}
}


//sterge un block din zona si reatribuie prev/next 


void remove_block(t_zone *zone, void *block)
{
	t_block *prev;
	t_block *ptr;

	prev = NULL;
	ptr = zone->blocks;
	while (ptr && ((void *)ptr + sizeof(t_block)) != block)
	{
		prev = ptr;
		ptr = ptr->next;
	}
	if (!prev)
		zone->blocks = ptr->next;
	else
		prev->next = ptr->next;
}

//dezaloca blocul de memorie pointat de ptr

void free(void *ptr)
{
	t_zone	*zone;

	if (!ptr)
		return ;
	pthread_mutex_lock(&g_mutex);
	zone = find_zone(ptr);
	pthread_mutex_unlock(&g_mutex);
	if (!zone)
		return ;
	pthread_mutex_lock(&g_mutex);
	remove_block(zone, ptr);
	if (!zone->blocks)
		remove_zone(zone);
	pthread_mutex_unlock(&g_mutex);
}
t_zone	*g_zones = NULL;


pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;


//aloca memorie si returneaza pointerul blocului memoriei


void *malloc(size_t size)
{
	void	*alloc;
	t_zone	*last;
	t_zone	*ptr;
	size_t	zone_size;

	alloc = NULL;
	pthread_mutex_lock(&g_mutex);
	zone_size = get_zone_size(size);
	if (!g_zones)
		g_zones = create_zone(size);
	ptr = g_zones;
	while (!alloc && ptr)
	{
		if (zone_size == ptr->size)
			alloc = add_block_to_zone(ptr, size);
		last = ptr;
		ptr = ptr->next;
	}
	if (!alloc)
	{
		last->next = create_zone(size);
		alloc = add_block_to_zone(last->next, size);
	}
	pthread_mutex_unlock(&g_mutex);
	return (alloc);
}

void free_unthread(void *ptr)
{
	t_zone	*zone;

	if (!ptr)
	     return ;
	zone = find_zone(ptr);
	if (!zone)
	    return ;
	remove_block(zone, ptr);
	if (!zone->blocks)
		remove_zone(zone);
}



void *malloc_unthread(size_t size)
{
	void	*alloc;
	t_zone	*last;
	t_zone	*ptr;
	size_t	zone_size;

	alloc = NULL;
	zone_size = get_zone_size(size);
	if (!g_zones)
		g_zones = create_zone(size);
	ptr = g_zones;
	while (!alloc && ptr)
	{
		if (zone_size == ptr->size)
			alloc = add_block_to_zone(ptr, size);
		last = ptr;
		ptr = ptr->next;
	}
	if (!alloc)
	{
		last->next = create_zone(size);
		alloc = add_block_to_zone(last->next, size);
	}
	return (alloc);
}


   //verifica daca marimea lui alloc corespunde vechei marimi, verifica daca noua alocare incape in acelasi loc, daca nu ne folosim de call malloc, copiem memoria si o stergem pe cea veche


void *realloc_unthread(void *ptr, size_t size)
{
	t_zone	*zone;
	t_block *block;
	void	*new;

	zone = find_zone(ptr);
	if (!zone)
		return (NULL);
	block = find_block(zone, ptr);
	if (get_alloc_size(block->size) == get_alloc_size(size + sizeof(t_block)) &&(block->size >= size + sizeof(t_block) || (block->next &&(size_t)(block->next - (void *)block) >= size + sizeof(t_block))))
	{
		block->size = size + sizeof(t_block);
		return (ptr);
	}
	new = malloc_unthread(size);
	if (!new)
		return (NULL);
	if (block->size - sizeof(t_block) <= size)
		new = ft_memcpy(new, ptr, block->size - sizeof(t_block));
	else
		new = ft_memcpy(new, ptr, size);
	free_unthread(ptr);
	return (new);
}

//realoca un pointer in memoria unui nou bloc din zona si copiaza datele anterioare si returneaza un nou pointer

void *realloc(void *ptr, size_t size)
{
	void *new;

	if (!ptr)
		return (malloc(size));
	pthread_mutex_lock(&g_mutex);
	new = realloc_unthread(ptr, size);
	pthread_mutex_unlock(&g_mutex);
	return (new);
}

size_t	get_alloc_size(size_t block_size)
{
	if (block_size <= TINY_ALLOC)
		return (TINY_ALLOC);
	else if (block_size <= SMALL_ALLOC)
		return (SMALL_ALLOC);
	return (block_size);
}


size_t	get_zone_size(size_t size)
{
	size_t	new_size;
	size_t	page_size;
	size_t	alloc_size;

	page_size = getpagesize();
	alloc_size = get_alloc_size(size + sizeof(t_block));
	if (size + sizeof(t_block) <= SMALL_ALLOC)
	{
		new_size = (NB_ALLOC / (page_size / alloc_size) + 1) * page_size;
		if (new_size - NB_ALLOC * alloc_size < sizeof(t_zone))
			new_size += page_size;
	}
	else
	{
		new_size = (alloc_size / page_size + 1) * page_size;
		if (new_size - alloc_size < sizeof(t_zone))
			new_size += page_size;
	}
	return (new_size);
}


t_zone	*create_zone(size_t size)
{
	t_zone		*new_zone;
	size_t		zone_size;

	zone_size = get_zone_size(size);
	new_zone = mmap(NULL, zone_size, PROT_READ | PROT_WRITE, MAP_PRIVATE |MAP_ANON, -1, 0);
	new_zone->size = zone_size;
	new_zone->blocks = NULL;
	new_zone->next = NULL;
	return (new_zone);
}

size_t	get_size_taken_zone(t_zone *zone)
{
	size_t	size;
	t_block *tmp;

	size = sizeof(t_zone);
	if (zone->blocks)
	{
		tmp = zone->blocks;
		while (tmp)
		{
			size += tmp->size;
			tmp = tmp->next;
		}
	}
	return (size);
}


t_zone	*find_zone(void *alloc)
{
	t_zone	*tmp;
	t_block *ptr;

	tmp = g_zones;
	while (tmp)
	{
		ptr = tmp->blocks;
		while (ptr)
		{
			if (((void *)ptr + sizeof(t_block)) == alloc)
				return (tmp);
			ptr = ptr->next;
		}
		tmp = tmp->next;
	}
	return (NULL);
}
//copiaza n caractere de la sursa la destinatie
void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	char		*pt_src;
	char		*pt_dst;

	pt_src = (char *)src;
	pt_dst = (char *)dst;
	while (n--)
		*pt_dst++ = *pt_src++;
	return (dst);
}
