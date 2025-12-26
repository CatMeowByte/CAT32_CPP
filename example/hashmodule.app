# hash module test

video.clear(1)

func foo()
 use video
 pixel(2,2,7)
 rect(4,4,8,8,12,0)

foo()

# should invalid
novar = 32

# should invalid because no namespace
rect(20,20,30,30,3,0)

# global
button(0)
.button(0)