#ifndef _NANDDEBUG_H_
#define _NANDDEBUG_H_

extern int nm_dbg_level;
extern int utils_dbg_level;
extern int libops_dbg_level;

void nd_dump_stack(void);
int __ndprint(const char *s, ...) __attribute__ ((format(printf, 1, 2)));

#endif /* _NANDDEBUG_H_ */
