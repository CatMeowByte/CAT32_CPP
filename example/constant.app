# basic constant declaration
con PI = 3.14159
con RADIUS = 10
con TWO = 2

# constant folding with other constants
con AREA = PI*RADIUS*RADIUS

# using constants in variable declaration
var x = RADIUS*TWO
var y = AREA+100

# using constant in stripe size
con SIZE = 8
stripe numbers SIZE

# using constant in array indexing
numbers[RADIUS-8] = PI*100
var value = numbers[TWO]

# constant in expressions
var result = RADIUS+TWO*3-AREA

# constant in function default arguments
func calculate(a,b:RADIUS,c:TWO)
 var sum = a+b+c
 return sum

# calling function with constant
var test = calculate(PI)

# scope shadowing - inner constant same name
func inner_scope()
 con RADIUS = 20
 var inner_x = RADIUS*TWO
 return inner_x

# using constant in string size
con TEXT_LEN = 15
string message TEXT_LEN

# error case - assignment to constant (should fail)
PI = 5

# edge case - constant name shadows after scope exit
var outer_radius = RADIUS