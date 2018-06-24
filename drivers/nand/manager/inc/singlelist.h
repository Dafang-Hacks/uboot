#ifndef _SINGLELIST_H_
#define _SINGLELIST_H_

#ifndef NULL
#define NULL (void*)0
#endif 

struct singlelist
{
    struct singlelist *next;
};

static inline void singlelist_init(struct singlelist *head){
	head->next = NULL;
}

static inline  void singlelist_add(struct singlelist *prev,struct singlelist *head){
	struct singlelist *next = NULL;
	if(prev->next){
		next = prev->next;
		prev->next = head;
	}else
		prev->next = head;
	head->next = next;
}
static inline  void singlelist_del(struct singlelist *top,struct singlelist *head){
	struct singlelist *next;
	if((top != NULL) && (head != NULL)){
		if(top == head){
			top = top->next;
			return;
		}
		while(top){
			next = top->next;
			if(next == head){
				top->next = next->next;
				return;
			}
			top = top->next;
		}

	}
}
static inline  void singlelist_add_tail(struct singlelist *top,struct singlelist *head){
	struct singlelist *next,*prev;		
	if(top){
		next = top->next;
		prev = top;
		while(next){
			prev = next;
			next = next->next;
		}
		prev->next = head;
	}
}
static inline  void singlelist_add_head(struct singlelist *top,struct singlelist *head){
	if(head){
		head->next = top;
	}
}
#define singlelist_offsetof(TYPE, MEMBER) ((unsigned int) &((TYPE *)0)->MEMBER)

#define singlelist_for_each(pos, head) \
	for (pos = (head); pos != NULL; pos = pos->next)

/*
#define singlelist_for_each_ext(pos, head, next)			\
	for (next = (head); pos = next, (next ? next = next->next : NULL), pos != NULL;)
*/

#define singlelist_container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - singlelist_offsetof(type,member) );})

#define singlelist_entry(ptr, type, member) \
	singlelist_container_of(ptr, type, member)

#endif /* _SINGLELIST_H_ */
