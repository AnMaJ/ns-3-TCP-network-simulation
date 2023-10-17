set terminal png
set output "wirelessTcpVegas.png"
set title "Throughput plot for TCP (Wireless)Vegas"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP Vegas" with linespoints
40 252.945
44 253.033
48 252.439
52 252.835
60 252.883
552 364.961
576 433.004
628 566.628
1420 2477.67
1500 2571.15
e
