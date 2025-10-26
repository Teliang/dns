
loop_count=1;

while [ $loop_count -le 1000 ]; do

drill @127.0.0.1 google.com
drill @127.0.0.1 baidu.com
drill @127.0.0.1 bing.com
drill @127.0.0.1 reddit.com

loop_count=$((loop_count+1))

done
