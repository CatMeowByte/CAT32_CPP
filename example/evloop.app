# simple cursor movement with event loop
# coordinates updated each step, rendered each draw

# cursor position
var cursor_x = 0
var cursor_y = 0

# movement speed multiplier (pixels per step)
var speed = 2

# hardware input addresses for directional buttons
var addr_up = -9632
var addr_down = -9636
var addr_left = -9640
var addr_right = -9644

func init()
 # initialize cursor at screen center
 cursor_x = 120/2
 cursor_y = 160/2

func step()
 # read button states from hardware io
 var up_state = peek32(addr_up)
 var down_state = peek32(addr_down)
 var left_state = peek32(addr_left)
 var right_state = peek32(addr_right)

 # update cursor position based on button states
 # positive state means button pressed
 if up_state>0:
  cursor_y = cursor_y-speed
 if down_state>0:
  cursor_y = cursor_y+speed
 if left_state>0:
  cursor_x = cursor_x-speed
 if right_state>0:
  cursor_x = cursor_x+speed

func draw()
 # clear screen for fresh frame
 clear(2)

 # draw 3x3 rectangle centered at cursor
 rect(cursor_x-1,cursor_y-1,3,3,7,0)

 # present frame to display
 flip()