stripe fox0 8
fox0#0 = 0x044A.AA44
fox0#1 = 0x04AA.AAA4
fox0#2 = 0x0FFC.AACF
fox0#3 = 0x0FAC.FFCA
fox0#4 = 0x0077.7770
fox0#5 = 0x0004.DD00
fox0#6 = 0x044A.AA00
fox0#7 = 0x044A.4200

stripe fox1 8
fox1#0 = 0x044A.AA44
fox1#1 = 0x04AA.AAA4
fox1#2 = 0x0FFC.AACF
fox1#3 = 0x0FAC.FFCA
fox1#4 = 0x0077.7770
fox1#5 = 0x0004.DD00
fox1#6 = 0x04AA.AAA0
fox1#7 = 0x04AA.4A20

stripe fox2 8
fox2#0 = 0x044A.AA44
fox2#1 = 0x04FC.AAC4
fox2#2 = 0x0FAC.FFCF
fox2#3 = 0x0F77.777A
fox2#4 = 0x0077.7770
fox2#5 = 0x004A.4A20
fox2#6 = 0x04AA.4A20
fox2#7 = 0x04AA.AA00

stripe fox3 8
fox3#0 = 0x044A.AA44
fox3#1 = 0x04FA.AAF4
fox3#2 = 0x0FAC.AACF
fox3#3 = 0x0FAC.FFCA
fox3#4 = 0x0077.7770
fox3#5 = 0x0004.DD00
fox3#6 = 0x04AA.AAA0
fox3#7 = 0x04AA.4A20

func sprite(ox,oy,data,sc):
 var y = 0
 while y<8:
  var x = 0
  var d = data#y
  while x<8:
   var p = d&&0xF000
   p = (p>>12)&&15
   d = d<<4
   var i = 0
   while i<sc:
    var j = 0
    while j<sc:
     var a = ox+x*sc+i
     var b = oy+y*sc+j
     pixel(a,b,p)
     j = j+1
    i = i+1
   x = x+1
  y = y+1
 return

clear(0)
sprite(5*3,8,fox0,3)
sprite(13*3,8,fox1,3)
sprite(21*3,8,fox2,3)
sprite(29*3,8,fox3,3)
flip()