set terminal png
set output "wiredTcpVeno.png"
set title "Throughput plot for TCP (Wired)Veno"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP Veno" with linespoints
40 1047.34
44 1054.3
48 1095.66
52 1183.37
60 1484.05
552 5875.65
576 5854.37
628 5818.88
1420 5637.78
1500 5583.03
e
