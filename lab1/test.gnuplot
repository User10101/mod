set boxwidth 0.05 absolute
set style fill solid 1.0 noborder

bin_width = 0.3;

bin_number(x) = floor(x / bin_width);
rounded(x) = bin_width * (bin_number(x))
plot 'test.txt' using(rounded($1)):(1) smooth frequency with boxes, 1/2*sin(x)