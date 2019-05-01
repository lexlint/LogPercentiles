//
//  LogPercentiles.h
//  LogPercentiles
//
//  Created by lexlin on 2019/5/1.
//  Copyright © 2019 com.lexlin. All rights reserved.
//

#ifndef LogPercentiles_h
#define LogPercentiles_h

// error code of func load_log
enum ERR_LOAD_LOG{
    ERR_LL_SUCCESS          = 0,    // no error
    ERR_LL_INVALID_PARAM    = -1,   // one or more params are invalid
    ERR_LL_OPEN_DIR_FAILED  = -2,   // open dir failed, see log for the detail errno
    ERR_LL_NO_LOG_FILE      = -3,   // there is no log files in the param dir
    ERR_LL_NO_LOG_RECORD    = -4,   // there are some log file, but no log record is found is the log file
};

/**
 * Load all data from log files & Count the number of requests in different time periods
 * time-complexity  : O(n)
 * space-complexity : O(1)
 * @param log_dir       log dir
 * @param total_count   total log record count
 * @param record_count  is used to save the count of records in different time periods
 *          The size is fixed at [3][100], corresponding to 0ms - 100s.
 *          Store the longer time counts by lower accuracy for better performance and lower storage cost,
 *          because of the longer time used, the lower accuracy we care about
 *          ATTENTION:
 *          For simplicity of calculation, not all element in the array are used!
 *          [0][0-99]  * 10ms  [0-1s)
 *          [1][10-99] * 100ms [1-10s)
 *          [2][10-99] * 1000ms[10-99s)
 * @return              error code, see ERR_LOAD_LOG.
 */
int load_log(const char* log_dir, uint64_t* total_count, uint64_t record_count[3][100]);

/**
 * calculates diffrent percentile response time
 * time-complexity  : O(1)
 * space-complexity : O(1)
 * @param record_count      the tmp result from load_log, see func load_log's comments for more information
 * @param percent_counts    array of diffrent percentiles counts
 * @param times             the percentile response times
 *                          return value, same size array with percent_counts
 * @param num               array size of percent_counts (and times)
 */
void calc_percentage_time(uint64_t record_count[3][100], int64_t percent_counts[], int times[], uint num);

/**
 * output report
 * @param times the 90%, 95% and 99% percentile response time
 */
void output_report(int times[3]);

#endif /* LogPercentiles_h */
