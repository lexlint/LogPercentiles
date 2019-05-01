//
//  test_main.c
//  LogPercentiles
//
//  Created by lexlin on 2019/5/1.
//  Copyright © 2019 com.lexlin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "LogPercentiles.h"
#include "test_common.h"

const int SECONDS_ONE_DAY = 60 * 60 * 24;
const char* LOG_DIR = "/var/log/httpd";

const char* INVALID_LOG_CONTENT[] = {
    "10.2.3.4 [2019/02/03:23:59:09] \"GET /api/playeritems?playerId=3\" 200\n",
    "10.2.3.4 [2019/02/03:23:59:09] \"GET /api/playeritems?playerId=3\" 1417\n",
    "10.2.3.4 \"GET /api/playeritems?playerId=3\" 200 1417\n",
    "10.2.3.4 /api/playeritems?playerId=3\" 200 1417\n",
    "[2019/02/03:23:59:09] \"GET /api/playeritems?playerId=3\" 200 1417\n",
    "aaabcwjaxocpqmdcbzicqnc\n",
    "   \n",
    "\0x10\0x11\n"
};

int clear_log_dir(const char* log_dir){
    DIR *dir;
    if ((dir=opendir(log_dir)) == NULL) {
        printf("open log dir error![err:%d]", errno);
        return -1;
    }
    
    struct dirent *ptr;
    while ((ptr=readdir(dir)) != NULL) {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0) {
            continue;
        }
        else if(ptr->d_type == DT_REG) {
            char log_file[64] = {0};
            sprintf(log_file, "%s/%s", log_dir, ptr->d_name);
            int ret = remove(log_file);
            if (ret != 0) {
                printf("remove log file[%s] error![err:%d]", ptr->d_name, errno);
            }
        }
    }
    closedir(dir);
    return 0;
}

int create_log_dir_if_not_exist(const char* log_dir){
    struct stat sb;
    if (stat(log_dir, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        int ret = mkdir(log_dir, S_IRWXU | S_IRWXG | S_IRWXO);
        if (ret != 0) {
            printf("create log dir failed!");
            return -1;
        }
    }
    return 0;
}

void generate_log(const char* log_dir, int days, int records_per_day){
    time_t time_now;
    time(&time_now);
    struct tm * tm_now = localtime(&time_now);
    tm_now->tm_hour = 0;
    tm_now->tm_min = 0;
    tm_now->tm_sec = 0;
    
    time_t time_start = mktime(tm_now) - SECONDS_ONE_DAY * days;
    float sec_per_record = (float)SECONDS_ONE_DAY / records_per_day;
    
    for (int day_index = 0; day_index < days; day_index ++) {
        time_t time_day = time_start + SECONDS_ONE_DAY * day_index;
        struct tm * tm_day = localtime(&time_day);
        char file_name[32] = {0};
        sprintf(file_name, "%s/%04d-%02d-%02d.log", log_dir, 1900 + tm_day->tm_year, tm_day->tm_mday, tm_day->tm_mon + 1);
        
        FILE *fp = fopen(file_name, "w");
        for (int records_index = 0; records_index < records_per_day; records_index ++) {
            
            time_t time_record = time_day + (int)(sec_per_record * records_index);
            struct tm * tm_record = localtime(&time_record);
            
            char log_content[128] = {0};
            sprintf(log_content,
                    "10.2.3.4 [%04d/%02d/%02d:%02d:%02d:%02d] \"GET /api/playeritems?playerId=3\" 200 %d\n",
                    1900 + tm_record->tm_year, tm_record->tm_mday, tm_record->tm_mon + 1,
                    tm_record->tm_hour, tm_record->tm_min, tm_record->tm_sec,
                    (int)(pow(2, ((float)arc4random_uniform(RAND_MAX)/RAND_MAX * 14)) / pow(2, 12) * 1000));
            fwrite(log_content, strnlen(log_content, 128), 1, fp);
        }
        fclose(fp);
    }
}

void generate_invalid_log(const char* log_dir){
    time_t time_now;
    time(&time_now);
    struct tm * tm_now = localtime(&time_now);
    
    char file_name[32] = {0};
    sprintf(file_name, "%s/%04d-%02d-%02d.log", log_dir, 1900 + tm_now->tm_year, tm_now->tm_mday, tm_now->tm_mon + 1);
    
    FILE *fp = fopen(file_name, "w");
    for (int index = 0; index < sizeof(INVALID_LOG_CONTENT)/sizeof(INVALID_LOG_CONTENT[0]); index ++) {
        fwrite(INVALID_LOG_CONTENT[index], strlen(INVALID_LOG_CONTENT[index]), 1, fp);
    }
    fclose(fp);
}

int test_normal_case(int days, int records_per_day){
    printf("test_case days(%d), records_per_day(%d)\n", days, records_per_day);
    int ret = clear_log_dir(LOG_DIR);
    if (ret != 0) {
        printf("clear_log_dir failed!\n");
        return ret;
    }
    ret = create_log_dir_if_not_exist(LOG_DIR);
    if (ret != 0) {
        printf("create_log_dir failed!\n");
        return ret;
    }
    
    generate_log(LOG_DIR, days, records_per_day);
    
    int64_t time_load_file = 0;
    int64_t time_calc_percentage = 0;
    
    uint64_t record_count[3][100] = {0};
    uint64_t total_count = 0;
    
    GET_TIME
    (
     time_load_file,
     {
         ret = load_log(LOG_DIR, &total_count, record_count);
         if (ret != ERR_LL_SUCCESS) {
             printf("Load log failed! errcode:%d\n", ret);
             return ret;
         }
     }
     );
    
    int64_t percent_counts[3] = {total_count * 0.9, total_count * 0.95, total_count * 0.99};
    int times[3] = {0};
    
    GET_TIME
    (
     time_calc_percentage,
     {
         calc_percentage_time(record_count, percent_counts, times, 3);
     }
     );
    
    output_report(times);
    printf("performance:\ntime_load_file:%lld microseconds, time_calc_percentage:%lld microseconds\n\n",
           time_load_file, time_calc_percentage);
    
    return 0;
}

int test_invalid_log_format_case(){
    printf("test_invalid_log_format_case\n");
    int ret = clear_log_dir(LOG_DIR);
    if (ret != 0) {
        printf("clear_log_dir failed!\n");
        return ret;
    }
    ret = create_log_dir_if_not_exist(LOG_DIR);
    if (ret != 0) {
        printf("create_log_dir failed!\n");
        return ret;
    }
    
    generate_invalid_log(LOG_DIR);
    
    int64_t time_load_file = 0;
    int64_t time_calc_percentage = 0;
    
    uint64_t record_count[3][100] = {0};
    uint64_t total_count = 0;
    
    GET_TIME
    (
     time_load_file,
     {
         ret = load_log(LOG_DIR, &total_count, record_count);
         if (ret != ERR_LL_SUCCESS) {
             printf("Load log failed! errcode:%d\n", ret);
             return ret;
         }
     }
     );
    
    int64_t percent_counts[3] = {total_count * 0.9, total_count * 0.95, total_count * 0.99};
    int times[3] = {0};
    
    GET_TIME
    (
     time_calc_percentage,
     {
         calc_percentage_time(record_count, percent_counts, times, 3);
     }
     );
    
    output_report(times);
    printf("performance:\ntime_load_file:%lld microseconds, time_calc_percentage:%lld microseconds\n\n",
           time_load_file, time_calc_percentage);
    
    return 0;
}

void run_all_case(){
    test_invalid_log_format_case();
    test_normal_case(1, 100);
    test_normal_case(10, 100);
    test_normal_case(100, 10000);
}

int main(int argc, const char * argv[]) {
    
    run_all_case();
    
    return 0;
}
