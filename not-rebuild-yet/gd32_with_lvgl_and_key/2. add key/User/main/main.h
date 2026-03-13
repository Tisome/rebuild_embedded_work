#ifndef MAIN_H
#define MAIN_H

#ifndef SYS_TEST
#define SYS_TEST 1
#endif

#if (SYS_TEST == 1)

#include "gd32f30x.h"
#include <stdio.h>

#include "RUF_X_demo.h"

#else

#include "gd32f30x.h"
#include "systick.h"
#include <stdio.h>

#endif

#endif /* MAIN_H */
