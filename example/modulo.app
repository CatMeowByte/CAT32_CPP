# modulo operator

func init()
 # basic positive modulo: 7 % 3 = 1 (7 divided by 3 is 2 with remainder 1)
 see(7%3)

 # negative dividend: -7 % 3 = 2 (floored: -7 / 3 = -3 floor, -7 - (-3 * 3) = 2)
 see(-7%3)

 # negative divisor: 7 % -3 = -2 (floored: 7 / -3 = -3 floor, 7 - (-3 * -3) = -2)
 see(7%-3)

 # both negative: -7 % -3 = -1 (floored: -7 / -3 = 2 floor, -7 - (2 * -3) = -1)
 see(-7%-3)

 # decimal modulo: 0.7 % 0.2 = 0.1 (fixed-point handles fractional remainder)
 see(0.7%0.25)

 # any number mod 1: 42 % 1 = 0 (always zero remainder)
 see(42%1)

 # zero mod anything: 0 % 7 = 0 (zero has no remainder)
 see(0%7)

 # exact division: 6 % 3 = 0 (divides evenly, no remainder)
 see(6%3)

 # operator precedence: 2 + 3 % 4 = 5 (modulo same precedence as multiply/divide, evaluated first: 2 + (3 % 4) = 2 + 3)
 see(2+3%4)

 # divide by zero guard: 7 % 0 = 0 (returns zero on division by zero)
 see(7%0)