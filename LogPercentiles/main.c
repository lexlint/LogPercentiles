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
    uint64_t record_count[3][100] = {0};
    uint64_t total_count = 0;
    
    int ret = load_log(LOG_DIR, &total_count, record_count);
    if (ret != ERR_LL_SUCCESS) {
        printf("Load log failed!");
        return ret;
    }
    
    int64_t percent_counts[3] = {total_count * 0.9, total_count * 0.95, total_count * 0.99};
    int times[3] = {0};
    
    calc_percentage_time(record_count, percent_counts, times, 3);
    
    output_report(times);
    
    return 0;
}
