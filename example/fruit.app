func fruit(value)
 var shift = value*8
 if value==0
  text(10,10+shift,"Apple",8)
 else
  if value==1
   text(15,20+shift,"Banana",10)
  else
   if value==2
    text(20,30+shift,"Cherry",14)
   else
    if value==3
     text(25,40+shift,"Dragonfruit",13)
    else
     text(30,50+shift,"Unknown fruit",5)
 return


clear(1)
flip()
wait(10)
var i = 0
while i<3+1+4
 wait(10)
 fruit(i)
 flip()
 see(i)
 i = i+1