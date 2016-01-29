#include <stdio.h>
#include <stdlib.h>
#include <linux>

/*
struct s {
	int a;
};

#define __INIT(name, n) \
{			\
	.a = n,		\
}
*/

struct com {
	struct list_head coms;
};
void main()
{
	//struct s * ss = (struct s *) malloc(sizeof(struct s));
	//struct s ss;
	
	
	//*ss = (struct s)  { .a = 2, };
	//*ss = (struct s )  __INIT(ss, 2);
	
	//printf("%d\n", ss->a);
}
