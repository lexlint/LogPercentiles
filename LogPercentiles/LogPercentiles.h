//
//  LogPercentiles.h
//  LogPercentiles
//
//  Created by lexlin on 2019/5/1.
//  Copyright © 2019 com.lexlin. All rights reserved.
//

#ifndef LogPercentiles_h
#define LogPercentiles_h

// error code
enum ERR_CODE{
    ERR_SUCCESS             = 0,    // no error
    ERR_INVALID_PARAM       = -1,   // one or more params are invalid
    ERR_OPEN_DIR_FAILED     = -2,   // open dir failed, see log for the detail errno
    ERR_CREATE_DIR_FAILED   = -3,   // create dir failed
    ERR_NO_LOG_FILE         = -4,   // there is no log files in the dir param
    ERR_NO_LOG_RECORD       = -5,   // there are some log files, but no log record is found in the log files
};

/**
 * Load all data from log files & Count the number of requests in different time periods
 * time-complexity  : O(n)
 * space-complexity : O(1)
 * @param log_dir           log dir
 * @param total_count       total log record count
 * @param record_count      is used to save the record counts in different time periods
 *          The size is fixed at [3][100], corresponding to 0ms - 100s.
 *          If want to expand the time range of coverage, modify the logic about record_count param.
 *          Store the longer time counts by lower accuracy for better performance and lower storage cost,
 *          because of the longer time used, the lower accuracy we care about
 *          ATTENTION:
 *          For simplicity of calculation, not all elements in the array are used!
 *          [0][0-99]  * 10ms  [0-1s)
 *          [1][10-99] * 100ms [1-10s)      "[1][0-9] are not used"
 *          [2][10-99] * 1000ms[10-99s)     "[2][0-9] are not used"
 * @return                  error code, see ERR_CODE.
 */
int load_log(const char* log_dir, uint64_t* total_count, uint64_t record_count[3][100]);

/**
 * calculates diffrent percentile response time using the return value of function "load_log"
 * time-complexity  : O(1)
 * space-complexity : O(1)
 * @param record_count      the tmp result from load_log, see func load_log's comments for more information
 * @param percents          array of diffrent percentiles
 * @param times             the percentile response times
 *                          return value, same size array with percent_counts
 * @param num               array size of percent_counts (and times)
 */
void calc_time(uint64_t total_count, uint64_t record_count[3][100], float percents[], int times[], uint num);

/**
 * calculates diffrent percentile response time from log dir
 * num of percentile is setable
 * @param log_dir           the log dir
 * @param percents          array of diffrent percentiles
 * @param times             the percentile response times
 *                          return value, same size array with percents
 * @param num               array size of percents (and times)
 */
int calc_percentage_time(const char* log_dir, float percents[], int times[], uint num);

/**
 * output report
 * @param percents          array of diffrent percentiles
 * @param times             the percentile response times, same size array with percents
 * @param num               array size of percents (and times)
 */
void output_report(float percents[], int times[], uint num);

#endif /* LogPercentiles_h */
