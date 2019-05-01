//
//  main.c
//  LogPercentiles
//
//  Created by lexlin on 2019/4/20.
//  Copyright © 2019 com.lexlin. All rights reserved.
//

// time-complexity : O(n)
// space-complexity : O(1)

#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "LogPercentiles.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

const uint MAX_FILE_NAME_LENGTH = 64;
const uint MAX_IPV4_LENGTH = 16;
const uint MAX_TIMESTEMP_LENGTH = 32;
const uint MAX_API_LENGTH = 128;
const uint ITEMS_PER_LOG_LINE = 5;

int load_log(const char* log_dir, uint64_t* total_count, uint64_t record_count[3][100]){
    // check param
    if (log_dir == NULL || total_count == NULL || record_count == NULL) {
        printf("invalid param!");
        return ERR_LL_INVALID_PARAM;
    }
    
    DIR *dir;
    if ((dir=opendir(log_dir)) == NULL) {
        printf("open log dir error![err:%d]", errno);
        return ERR_LL_OPEN_DIR_FAILED;
    }
    
    int log_files = 0;  //log file counting
    int64_t invalid_log_lines = 0;  //invalid log line counting
    struct dirent *ptr;
    while ((ptr=readdir(dir)) != NULL) {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0) {
            continue;
        }
        else if(ptr->d_type == DT_REG) {
            log_files ++;
            char log_file[MAX_FILE_NAME_LENGTH] = {0};
            sprintf(log_file, "%s/%s", log_dir, ptr->d_name);
            FILE *fp = fopen(log_file, "r");
            if (fp) {
                char *line = NULL;
                size_t len = 0;
                do {
                    line = fgetln(fp, &len);
                    if (line) {
                        char ip[MAX_IPV4_LENGTH] = {0};
                        char timestamp[MAX_TIMESTEMP_LENGTH] = {0};
                        char url[MAX_API_LENGTH] = {0};
                        int ret_code = 0;
                        int time_span = 0;
                        //currect log line looks like this:
                        //10.2.3.4 [2018/13/10:14:02:39] "GET /api/playeritems?playerId=3" 200 1230
                        char line_buf[1024] = {0};
                        strncpy(line_buf, line, len);
                        int items = sscanf(line_buf, "%s %s \"GET %s %d %d", ip, timestamp, url, &ret_code, &time_span);
                        if (items == ITEMS_PER_LOG_LINE && time_span >= 0) {
                            (*total_count) ++;
                            //[0,1000) saved into [0,99]
                            if (time_span < 1000) {
                                record_count[0][time_span / 10] += 1;
                            }
                            //[1000, 10000) saved into [110, 199]
                            else if (time_span < 10000){
                                record_count[1][time_span / 100] += 1;
                            }
                            //[10000, 100000) saved into [210, 299]
                            else if (time_span < 100000){
                                record_count[2][time_span / 1000] += 1;
                            }
                            //[100000,infinite)
                            else{
                                char tmp[1024 + 1] = {0};
                                strncpy(tmp, line, MIN(len, 1024));
                                printf("this record used over 100 seconds!!!\n%s", tmp);
                            }
                        }
                        else {
                            invalid_log_lines ++;
                            char tmp[1024 + 1] = {0};
                            strncpy(tmp, line, MIN(len, 1024));
                            printf("invalid log line!\n%s", tmp);
                        }
                    }
                } while (line);
                
                fclose(fp);
            }
        }
    }
    closedir(dir);
    
    if (log_files == 0) {
        return ERR_LL_NO_LOG_FILE;
    }
    if (*total_count == 0) {
        return ERR_LL_NO_LOG_RECORD;
    }
    if (invalid_log_lines != 0) {
        printf("%lld lines of logs are invalid!\n", invalid_log_lines);
    }
    return ERR_LL_SUCCESS;
}

void calc_percentage_time(uint64_t record_count[3][100], int64_t percent_counts[], int times[], uint num){
    if (num == 0) {
        return;
    }
    memset(times, 0, sizeof(int) * num);
    
    int64_t count = 0;
    // because the 3 series of record_count saved by diffrent time accuracy
    // so the calculation process is a bit complicated
    // [0][0-99]  * 10ms  [0-1s)
    // [1][10-99] * 100ms [1-10s)
    // [2][10-99] * 1000ms[10-99s)
    for (int series = 0; series < 3; series ++) {
        // the first set is uesed from 0 index
        // the others are used from 10 index
        for (int index = (series == 0 ? 0 : 10); index < 100; index ++) {
            // accumulate record count
            count += record_count[series][index];
            // check if the percent count is reached
            for (int index_percent = 0; index_percent < num; index_percent ++) {
                if (times[index_percent] == 0 && count > percent_counts[index_percent]) {
                    times[index_percent] = index * pow(10, series + 1);
                }
            }
        }
    }
}

void output_report(int times[3]){
    if (times[0] != 0) {
        printf("90%% of requests return a response in %d ms\n", times[0]);
    }
    else{
        printf("over 10%% of requests return a response more than 100 secends!\n");
    }
    if (times[1] != 0) {
        printf("95%% of requests return a response in %d ms\n", times[1]);
    }
    else{
        printf("over 5%% of requests return a response more than 100 secends!\n");
    }
    if (times[2] != 0) {
        printf("99%% of requests return a response in %d ms\n", times[2]);
    }
    else{
        printf("over 1%% of requests return a response more than 100 secends!\n");
    }
}
