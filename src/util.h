#ifndef UTIL_H
#define UTIL_H

#define ENV_LEN 2

#define DLDIR_TYPE_LIST 0
#define DLDIR_TYPE_RM 1

void env_clean();
int dl_action(int type);

#endif
