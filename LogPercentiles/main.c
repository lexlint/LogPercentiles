//
//  main.c
//  LogPercentiles
//
//  Created by lexlin on 2019/5/1.
//  Copyright © 2019 com.lexlin. All rights reserved.
//

#include <stdio.h>
#include "LogPercentiles.h"

const char* LOG_DIR = "/var/log/httpd";

int main(int argc, const char * argv[]) {

    float percents[3] = {0.90, 0.95, 0.99};
    int times[3] = {0};
    
    int ret = calc_percentage_time(LOG_DIR, percents, times, 3);
    if (ret != 0) {
        printf("calc_percentage_time failed! [%d]", ret);
        return ret;
    }
    
    output_report(percents, times, 3);
    
    return 0;
}
