# test break and continue

clear()

func drawcol(c,h):
 var y = 0
 while y<h:
  pixel(c,y,15)
  y = y+1
 return

var x = 0
var col = 0
while x<20:
 x = x+1

 if x/3*3==x:
  continue

 if x>=16:
  break

 drawcol(col,x)
 col = col+1

flip()