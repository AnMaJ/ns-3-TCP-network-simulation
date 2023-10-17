set terminal png
set output "wiredTcpVegas.png"
set title "Throughput plot for TCP (Wired)Vegas"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP Vegas" with linespoints
40 206.599
44 210.801
48 210.784
52 215.994
60 220.363
552 1833.72
576 1898.64
628 2062.48
1420 3267.1
1500 3293.11
e
