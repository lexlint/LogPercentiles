//
//  test_common.h
//  LogPercentiles
//
//  Created by lexlin on 2019/5/1.
//  Copyright © 2019 com.lexlin. All rights reserved.
//

#ifndef test_common_h
#define test_common_h

#include <stdio.h>

#define GET_TIME(time, code) { \
(time) = GetMicrosecondCount(); \
code \
(time) = GetMicrosecondCount() - (time);\
}

int64_t GetMicrosecondCount(void);

#endif /* test_common_h */
