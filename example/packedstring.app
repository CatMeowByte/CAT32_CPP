# string system

# declaration full
string hello = "Hi, all!"

# declaration size
string buffer 20

# declaration implicit (in assignment)
var msg = "variant"

# assignment
buffer = "assigned"

# escape sequences
string escaped = "line1\nline2\ttab\"quote\\slash"

# empty
string empty = ""

# single character
string single = "A"

# long string
string long = "The quick brown fox jumps over the lazy dog and all that"

# function with string argument
func display()
 text(0,0,hello,10,0)
 text(0,8,buffer,11,0)
 text(0,16,escaped,12,0)
 text(0,24,empty,13,0)
 text(0,32,single,14,0)
 text(0,40,long,15,0)

func init()
 clear(0)
 display()
 flip()

 see(peek32(4))