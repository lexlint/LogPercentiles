//
//  test_common.c
//  LogPercentiles
//
//  Created by lexlin on 2019/5/1.
//  Copyright © 2019 com.lexlin. All rights reserved.
//

#include "test_common.h"
#include <sys/time.h>

int64_t GetMicrosecondCount()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000000 + tv.tv_usec;
}
