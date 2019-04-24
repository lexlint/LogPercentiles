//
//  main.c
//  GenerateLog
//
//  Created by lexlin on 2019/4/20.
//  Copyright © 2019 com.lexlin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>

const int SECONDS_ONE_DAY = 60 * 60 * 24;
const char* LOG_DIR = "/var/log/httpd";

int main(int argc, const char * argv[]) {
    
    if (argc != 3) {
        printf("usage: GenerateLog days records-per-day");
        return -1;
    }
    
    int days = atoi(argv[1]);
    int records_per_day = atoi(argv[2]);
    
    if (days <= 0 || records_per_day <= 0){
        printf("invalid parameter! \nusage: GenerateLog days records-per-day");
        return -2;
    }
    
    struct stat sb;
    if (stat(LOG_DIR, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        int ret = mkdir(LOG_DIR, S_IRWXU | S_IRWXG | S_IRWXO);
        if (ret != 0) {
            printf("create log dir failed!");
            return -3;
        }
    }
    
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
        sprintf(file_name, "%s/%04d-%02d-%02d.log", LOG_DIR, 1900 + tm_day->tm_year, tm_day->tm_mday, tm_day->tm_mon + 1);
        
        FILE *fp = fopen(file_name, "w");
        for (int records_index = 0; records_index < records_per_day; records_index ++) {
            
            time_t time_record = time_day + (int)(sec_per_record * records_index);
            struct tm * tm_record = localtime(&time_record);
            
            char log_content[128] = {0};
            srand((int)time(NULL));
            
            // todo: rand() dosen't work well on mac OSX
            sprintf(log_content,
                    "10.2.3.4 [%04d/%02d/%02d:%02d:%02d:%02d] \"GET /api/playeritems?playerId=3\" 200 %d\n",
                    1900 + tm_record->tm_year, tm_record->tm_mday, tm_record->tm_mon + 1,
                    tm_record->tm_hour, tm_record->tm_min, tm_record->tm_sec,
                    (int)(pow(2, ((float)random() / RAND_MAX * 14)) / pow(2, 12) * 1000));
            fwrite(log_content, strnlen(log_content, 128), 1, fp);
        }
        fclose(fp);
    }
    
    return 0;
}
