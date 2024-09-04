#include<stdio.h>
#include<stdlib.h>
// returns the number of digits in a base-10 number
int numdigits (int n) {
int count = 1;
while ((n /= 10) != 0) {
count++;
}
return count;
}
// calculates 10^exp
int powerof10 (int exp) {
int result = 1;
for (int i = 0; i < exp; i++) {
result *= 10;
}
return result;
}
// returns the digit from d at position tensplace
// e.g., digit_place(5678,2) returns 6 because
// 6 is at the 10^2, or 100s place in 5678
int digit_place (int d, int tensplace) {
int p10 = powerof10 (tensplace);
return (int) ((d % (p10 * 10)) / p10);
}
int main (int argc, char **argv) {
char *digit_str[] = { "zero", "one", "two", "three",
"four", "five", "six", "seven",
"eight", "nine"
};
int number = atoi (argv[1]); // command line arg
int ndigits = numdigits (number);
for (int i = ndigits - 1; i >= 0; i--) {
int digit = digit_place (number, i);
printf ("%s ", digit_str[digit]);
}
printf ("\n");
return 0;
}
