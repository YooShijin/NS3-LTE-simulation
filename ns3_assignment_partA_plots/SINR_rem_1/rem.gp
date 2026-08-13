set terminal png size 1000,1000
set view map;
set title "Radio Environment Map with 4 eNBs"
set term qt;
set xlabel "Distance"
set ylabel "Signal Strength (dBm)"
set cblabel "SINR (dB)"
plot "rem4eNB.out" using ($1):($2):(10*log10($4)) with image
