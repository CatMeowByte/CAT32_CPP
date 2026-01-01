# A : OLD RULE ================
# strict spacing rule that must be followed to write a valid code

# all declaration types
var a_x = 5
con a_max = 100

# str as text
str a_name = "test"
str a_buffer[30] = "width limited content"

# str as numeric array
str a_data = 1, 2, 3
str a_values[10] = 5.5, -3, 99

# str as string array
str a_labels[] = "first", "second", "third"

# numbers: decimal, negative, hex, binary
var a_dec = 3.14
var a_neg = -10
var a_hex = 0xFF
var a_bin = 0b1010
var a_hexdec = 0xA.5
var a_bindec = 0b101.01

# escaped strings
str a_escaped = "line1\nline2\ttab\\backslash\"quote"

# declare variables for operator tests
var a_y = 2
var a_z = 3
var a_w = 4
var a_v = 5
var a_u = 6
var a_t = 7

# assignment
a_x = a_x+1
a_name = "new"
a_data[0] = 99

# capacity query
var a_cap = a_data[-1]
if a_cap==3
 see(a_cap)

# all control flow
if a_x>0 # inline comment
 a_x = a_x*2
else
 a_x = 0

while a_x<a_max
 a_x = a_x+1
 if a_x==64
  break
 if a_x%2==0
  continue

# function with recursion
func a_countdown(n)
 see(n)
 if n>0
  a_countdown(n-1)

# function with defaults
func a_process(a,b:10,c:20)
 var result = a+b*c%5
 return result

# all operators
var a_test = !a_x&a_y|a_z
var a_bits = ~~a_x&&a_y||a_z<<2>>1
var a_compare = a_x==a_y!=a_z<a_w>a_v<=a_u>=a_t

# three callable types: user function, module function, opcode
a_countdown(3)
video.text(0,0,a_name,7,1)
see(a_x)
see(a_hexdec)
see(a_bindec)

# module usage
use video
clear(1)

# memory opcodes
poke32(100,a_x)
var a_val = peek32(100)

# addressof
var a_addr = @a_x
poke32(a_addr,999)

# arrays
var a_index = a_data[1]+a_data[2]
a_data[a_index] = a_process(a_x,5,10)

# complex expression
if a_data[0]==99&a_x<=a_max|~~a_index<<2>>1
 a_labels[0] = "done"


# B : SPACE IGNORANT ==========
# spacing is ignored and the code should still be valid

# all declaration types
var  b_x=5
con b_max= 100

# str as text
str b_name ="test"
str b_buffer[ 30]= "width limited content"

# str as numeric array
str b_data= 1 ,2,  3
str b_values [10 ]= 5.5, -3  ,99

# str as string array
str b_labels[  ] ="first"  ,"second", "third"

# numbers: decimal, negative, hex, binary
var b_dec  =3.14
var  b_neg=-10
var b_hex=  0xFF
var  b_bin =0b1010
var b_hexdec= 0xA.5
var  b_bindec =0b101.01

# escaped strings
str b_escaped=  "line1\nline2\ttab\\backslash\"quote"

# declare variables for operator tests
var  b_y= 2
var b_z =3
var  b_w =4
var b_v= 5
var  b_u=6
var b_t  =7

# assignment
b_x=  b_x +1
b_name  = "new"
b_data[ 0]= 99

# capacity query
var b_cap= b_data[ -1]
if b_cap ==3
 see (b_cap)

# all control flow
if b_x >0# inline comment
 b_x =b_x*  2
else
 b_x  =0

while b_x< b_max
 b_x=  b_x +1
 if b_x ==64
  break
 if b_x %2== 0
  continue

# function with recursion
func b_countdown(n)
 see (n)
 if n> 0
  b_countdown (n -1)

# function with defaults
func b_process (a,b: 10,c :20)
 var result= a+b *c% 5
 return  result

# all operators
var b_test  =!b_x &b_y| b_z
var  b_bits=~~b_x&& b_y ||b_z<< 2>>1
var b_compare =b_x ==b_y!= b_z<b_w >b_v<= b_u>=b_t

# three callable types: user function, module function, opcode
b_countdown( 3)
video.text (0, 0,b_name ,7,1)
see( b_x)
see (b_hexdec)
see(b_bindec )

# module usage
clear (1)

# memory opcodes
poke32( 100, b_x)
var b_val =peek32 (100)

# addressof
var b_addr  =@b_x
poke32 (b_addr, 999)

# arrays
var b_index =b_data[ 1]+ b_data[2 ]
b_data[b_index ]=b_process (b_x, 5,10)

# complex expression
if b_data[ 0]== 99&b_x <=b_max |~~b_index<< 2>> 1
 b_labels[ 0]= "done"