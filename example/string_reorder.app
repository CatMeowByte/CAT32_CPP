use video

# original unsorted data
stripe original = "file10" "file2" "file1" "file20" "test" "testing" "data2" "data" "img001" "img100" "img010" "File5" "FILE3" "abc" "ABC" "photo1" "photo10" "doc" "document" "readme"

# working array that gets sorted
stripe working 20

# sorting state: 0=idle 1=sorting 2=complete
var state = 0

# current comparison position in bubble sort (-1 when inactive)
var compare_pos = -1

# tracks if any swaps occurred during current pass
var pass_had_swap = 0

func init()
 # copy original to working array
 var i = 0
 while i<20
  working[i] = original[i]
  i = i+1

 # reset state to idle
 state = 0
 compare_pos = -1

func step()
 # ok button starts automatic sorting
 if button(4)==1
  if state==0
   state = 1
   compare_pos = 0
   pass_had_swap = 0

 # automatic sorting progression when active
 if state==1
  # compare adjacent elements
  if string.order(working[compare_pos],working[compare_pos+1])>0
   # swap needed
   var temp = working[compare_pos]
   working[compare_pos] = working[compare_pos+1]
   working[compare_pos+1] = temp
   pass_had_swap = 1

  # advance to next comparison
  compare_pos = compare_pos+1

  # check if pass complete
  if compare_pos>=19
   # pass finished
   if pass_had_swap==1
    # need another pass
    compare_pos = 0
    pass_had_swap = 0
   else
    # sorting complete
    state = 2
    compare_pos = -1

 # cancel button resets everything
 if button(5)==1
  init()

func draw()
 # background color based on state
 var bg = 2
 if state==1
  bg = 1
 if state==2
  bg = 3
 clear(bg)

 # draw all strings
 var j = 0
 while j<20
  # determine text color
  var color = 7
  # highlight currently compared pair in blue
  if state==1
   if j==compare_pos
    color = 12
   if j==compare_pos+1
    color = 12

  text(0,j*8,working[j],color,bg)
  j = j+1