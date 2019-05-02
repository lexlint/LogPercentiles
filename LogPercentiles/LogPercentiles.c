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
#include <stdlib.h>
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
const uint MAX_LOG_LINE_LENGTH = 256;
const uint ITEMS_PER_LOG_LINE = 5;

int load_log(const char* log_dir, uint64_t* total_count, uint64_t record_count[3][100]){
    // check param
    if (log_dir == NULL || total_count == NULL || record_count == NULL) {
        printf("invalid param!");
        return ERR_INVALID_PARAM;
    }
    
    DIR *dir;
    if ((dir=opendir(log_dir)) == NULL) {
        printf("open log dir error![err:%d]", errno);
        return ERR_OPEN_DIR_FAILED;
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
                        // line string is not truncated, so copy it
                        char line_buf[MAX_LOG_LINE_LENGTH + 1] = {0};
                        strncpy(line_buf, line, MIN(len, MAX_LOG_LINE_LENGTH));
                        
                        // Discards lines of more than specified max length
                        if (len > MAX_LOG_LINE_LENGTH) {
                            invalid_log_lines ++;
                            printf("invalid log line!\n%s", line_buf);
                            continue;
                        }
                        
                        char ip[MAX_IPV4_LENGTH] = {0};
                        char timestamp[MAX_TIMESTEMP_LENGTH] = {0};
                        char url[MAX_API_LENGTH] = {0};
                        int ret_code = 0;
                        int time_span = 0;
                        //currect log line format:
                        //10.2.3.4 [2018/13/10:14:02:39] "GET /api/playeritems?playerId=3" 200 1230
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
                                printf("this record used over 100 seconds!!!\n%s", line_buf);
                            }
                        }
                        else {
                            invalid_log_lines ++;
                            printf("invalid log line!\n%s", line_buf);
                        }
                    }
                } while (line);
                
                fclose(fp);
                fp = NULL;
            }
        }
    }
    closedir(dir);
    dir = NULL;
    
    if (log_files == 0) {
        return ERR_NO_LOG_FILE;
    }
    if (*total_count == 0) {
        return ERR_NO_LOG_RECORD;
    }
    if (invalid_log_lines != 0) {
        printf("%lld lines of logs are invalid!\n", invalid_log_lines);
    }
    return ERR_SUCCESS;
}

void calc_time(uint64_t total_count, uint64_t record_count[3][100], float percents[], int times[], uint num){
    if (num == 0) {
        return;
    }
    memset(times, 0, sizeof(int) * num);
    
    int64_t* percent_counts = (int64_t*)malloc(sizeof(int64_t) * num);
    for (int index = 0; index < num; index ++) {
        percent_counts[index] = total_count * percents[index];
    }
    
    int64_t count = 0;
    // Using complicated algo with 3 series of record_count per different time accurancy for storage saving
    // [0][0-99]  * 10ms  [0-1s)
    // [1][10-99] * 100ms [1-10s)
    // [2][10-99] * 1000ms[10-99s)
    for (int series = 0; series < 3; series ++) {
        // the first set starts from index 0
        // the others start from index 10
        for (int index = (series == 0 ? 0 : 10); index < 100; index ++) {
            // accumulate record count
            count += record_count[series][index];
            // check if the percentage count is reached
            for (int index_percent = 0; index_percent < num; index_percent ++) {
                if (times[index_percent] == 0 && count > percent_counts[index_percent]) {
                    times[index_percent] = index * pow(10, series + 1);
                }
            }
        }
    }
    free(percent_counts);
    percent_counts = NULL;
}

int calc_percentage_time(const char* log_dir, float percents[], int times[], uint num){
    // check param
    for (int index = 0; index < num; index ++) {
        if (percents[index] < 0.0 || percents[index] >= 1.0) {
            printf("invalid percents param!");
            return ERR_INVALID_PARAM;
        }
    }
    
    uint64_t record_count[3][100] = {0};
    uint64_t total_count = 0;
    int ret = load_log(log_dir, &total_count, record_count);
    if (ret != ERR_SUCCESS) {
        printf("Load log failed!");
        return ret;
    }
    
    calc_time(total_count, record_count, percents, times, num);
    
    return 0;
}

void output_report(float percents[], int times[], uint num){
    for (int index = 0; index < num; index ++) {
        if (percents[index] < 0.0 || percents[index] >= 1.0) {
            printf("invalid percents param at index %d!", index);
            return;
        }
        
        if (times[index] != 0) {
            printf("%d%% of requests return a response in %d ms\n",
                   (int)(percents[index] * 100), times[index]);
        }
        else{
            printf("over %d%% of requests return a response more than 100 secends!\n",
                   (int)((1.0 - percents[index]) * 100));
        }
    }
}
