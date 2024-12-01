# Usage
run the compiled binary (with root permissions if required)

The programs just reads the TCP queue details from /proc/net/sockstat and stores in a circular queue.
The idea is to see for sudden fluctuations in number of syn packets over a time period (prevents false flagging when hardcoded thresholds are used).
