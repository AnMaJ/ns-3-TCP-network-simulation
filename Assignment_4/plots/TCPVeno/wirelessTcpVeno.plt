set terminal png
set output "wirelessTcpVeno.png"
set title "Throughput plot for TCP (Wireless)Veno"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP Veno" with linespoints
40 252.945
44 253.033
48 252.439
52 252.835
60 252.883
552 365.179
576 433.004
628 652.034
1420 3490.4
1500 3600.09
e
