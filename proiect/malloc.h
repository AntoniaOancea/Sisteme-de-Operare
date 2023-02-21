#ifndef MALLOC_H
# define MALLOC_H

# include <unistd.h>
# include <sys/mman.h>
# include <pthread.h>

# define NB_ALLOC	100
# define TINY_ALLOC	256
# define SMALL_ALLOC	4096

typedef struct s_block
{
	size_t	size;
	void	*next;
}t_block;

typedef struct	s_zone
{
	size_t	size;
	void	*blocks;
	void	*next;
}t_zone;

extern t_zone	*g_zones;
extern pthread_mutex_t	g_mutex;

void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

void remove_zone(t_zone *zone);
void remove_block(t_zone *zone, void *block);

size_t	get_alloc_size(size_t block_size);
size_t	get_zone_size(size_t block_size);
t_zone	*create_zone(size_t size);
size_t	get_size_taken_zone(t_zone *zone);

t_zone	*find_zone(void *alloc);
t_block *find_block(t_zone *zone, void *alloc);
void   *add_block_to_zone(t_zone *zone, size_t size);
//copiaza n caractere de la sursa la destinatie
void	*ft_memcpy(void *dst, const void *src, size_t n);

#endif