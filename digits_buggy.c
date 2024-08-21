#include<stdio.h>
#include<stdlib.h>
// returns the number of digits in a base-10 number
int numdigits (int n) {
int count = 0;
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

