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
#include <stdint.h>

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

const char* LOG_DIR = "/var/log/httpd";

// g_record_count is used to save the count of records in a range of time
// store the longer time counts by lower accuracy for better performance and lower storage cost
// [0][0-99]  * 10ms  [0-1s)
// [1][10-99] * 100ms [1-10s)
// [2][10-99] * 1000ms[10-99s)
unsigned int64_t g_record_count[3][100] = {0};
unsigned int64_t g_total_count = 0;

int main(int argc, const char * argv[]) {
    
    DIR *dir;
    struct dirent *ptr;
    
    if ((dir=opendir(LOG_DIR)) == NULL) {
        printf("open log dir error!");
        return -1;
    }
    
    // step 1. load all data from log files
    // time-complexity : O(n)
    // space-complexity : O(1)
    int64_t invalid_log_lines = 0;
    while ((ptr=readdir(dir)) != NULL) {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0) {
            continue;
        }
        else if(ptr->d_type == DT_REG) {
            char log_file[64] = {0};
            sprintf(log_file, "%s/%s", LOG_DIR, ptr->d_name);
            //printf("%s\n", log_file);
            FILE *fp = fopen(log_file, "r");
            if (fp) {
                char *line = NULL;
                size_t len = 0;
                do {
                    line = fgetln(fp, &len);
                    if (line) {
                        char ip[16] = {0};
                        char timestamp[32] = {0};
                        char url[128] = {0};
                        int ret_code = 0;
                        int time_span = 0;
                        //10.2.3.4 [2018/13/10:14:02:39] "GET /api/playeritems?playerId=3" 200 1230
                        int items = sscanf(line, "%s %s \"GET %s %d %d", ip, timestamp, url, &ret_code, &time_span);
                        if (items == 5 && time_span >= 0) {
                            //printf("%d\n", time_span);
                            g_total_count ++;
                            if (time_span < 1000) {
                                g_record_count[0][time_span / 10] += 1;
                            }
                            else if (time_span < 10000){
                                g_record_count[1][time_span / 100] += 1;
                            }
                            else if (time_span < 100000){
                                g_record_count[2][time_span / 1000] += 1;
                            }
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
    
    // step 2. calc percentage
    // time-complexity : O(1)
    // space-complexity : O(1)
    int64_t percent90 = g_total_count * 0.9;
    int64_t percent95 = g_total_count * 0.95;
    int64_t percent99 = g_total_count * 0.99;
    
    int time90 = 0;
    int time95 = 0;
    int time99 = 0;

    int64_t count = 0;
    bool finished = false;
    for (int series = 0; series < 3; series ++) {
        for (int index = (series == 0 ? 0 : 10); index < 100; index ++) {
            count += g_record_count[series][index];
            if (time90 == 0 && count > percent90) {
                time90 = index * pow(10, series + 1);
            }
            if (time95 == 0 && count > percent95) {
                time95 = index * pow(10, series + 1);
            }
            if (time99 == 0 && count > percent99) {
                time99 = index * pow(10, series + 1);
                finished = true;
                break;
            }
        }
        if (finished) {
            break;
        }
    }
    
    // step 3. output
    if (invalid_log_lines != 0) {
        printf("%d lines of logs are invalid!\n", invalid_log_lines);
    }
    
    if (time90 != 0) {
        printf("90%% of requests return a response in %d ms\n", time90);
    }
    else{
        printf("over 10%% of requests return a response more than 100 secends!\n");
    }
    if (time95 != 0) {
        printf("95%% of requests return a response in %d ms\n", time95);
    }
    else{
        printf("over 5%% of requests return a response more than 100 secends!\n");
    }
    if (time99 != 0) {
        printf("99%% of requests return a response in %d ms\n", time99);
    }
    else{
        printf("over 1%% of requests return a response more than 100 secends!\n");
    }

    return 0;
}
