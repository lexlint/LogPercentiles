# README

## What's LogPercentiles?
LogPercentiles calculates the 90%, 95% and 99% percentile response time for READ API requests, based on ALL log files stored in /var/log/httpd/*.log.

The log files are rotated every day at midnight, so there are multiple days of logs in that directory. 

Example:
```
/var/log/httpd/2018-13-10.log ​← Contains logs for Oct 13, 2018 
/var/log/httpd/2018-12-10.log
/var/log/httpd/2018-11-10.log
...
```

Each log file contains multiple log lines, and the format for each log line is:
```
IP_ADDRESS [timestamp] "HTTP_VERB URI" HTTP_ERROR_CODE RESPONSE_TIME_IN_MILLISECONDS
```

Example:

```
10.2.3.4 [2018/13/10:14:02:39] "GET /api/playeritems?playerId=3" 200 1230 
10.3.4.5 [2018/13/10:14:02:41] "GET /api/playeritems?playerId=2" 200 4630
```

LogPercentiles calculates several percentile response time for READ API requests.

Example:
```shell
90% of requests return a response in X ms 
95% of requests return a response in Y ms 
99% of requests return a response in Z ms
```

## Building
Build passed on MacOS Mojave.

## Expansibility
* Number of percentiles can be set, see the comments of func 
```calc_percentage_time```
* Time statistics range is from 0 to 100 seconds, If want to expand the time range of coverage, modify the logic about ```record_count``` param of func ```load_log```.

## Unit Test
Run the TARGET ```TestLogPercentiles``` of XCode Project LogPercentiles.
* normal case | PASSED
* invalid log content case | PASSED
* custom percentile num case | PASSED