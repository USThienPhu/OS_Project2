#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main(void) {
 printf("Note: Unix V6 was released in year %d\n", getyear());
 exit(0);
}