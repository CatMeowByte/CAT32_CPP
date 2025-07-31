$ function declaration

func foo(a,b,c):
 var result = a+b
 return result+c

$ A
see(foo(1,2,3)-foo(4,5,6))

$ B
see(mul(2,3)+mul(3,4))

$ order
var video_w = 120
var video_h = 160

clear(0)
pixel(video_w/2,video_h/2,7)
flip()