#ifndef _DEFS_H_
#define _DEFS_H_

typedef struct {
  const char *team;
  const char *name1, *number1, *email1;
  const char *name2, *number2, *email2;
} team_t;

extern team_t team;

#endif /* _DEFS_H_ */

