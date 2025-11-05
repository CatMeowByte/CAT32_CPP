$ button state visualizer - 8 buttons with state-based text coloring

$ hardware io button state addresses
stripe button_addresses = -9632 -9636 -9640 -9644 -9648 -9652 -9656 -9660
$ button display names
stripe button_names = "up" "down" "left" "right" "ok" "cancel" "shoulder_left" "shoulder_right"

$ continuous update loop
while 1:
 clear()

 $ initial line position (vertical spacing: 20px per button)
 var line_y = 14
 var button_index = 0

 $ iterate through 8 buttons
 while button_index<8:
  $ read button state from hardware io memory
  var button_state = peek32(button_addresses[button_index])

  $ calculate line width from button state value
  var line_width = button_state
  $ cap line width at maximum (115 - 5 = 110px)
  if line_width>110:
   line_width = 110

  $ determine text color based on button state
  var text_color = 7
  var bg_color = 0
  if button_state>0:
   text_color = 12
  if button_state==1:
   text_color = 12
   bg_color = 7
  if button_state==-1:
   text_color = 0
   bg_color = 8

  $ draw button name text with state-based coloring
  text(5,line_y-8,button_names[button_index],text_color,bg_color)
  $ draw line with width proportional to button state
  line(5,line_y,5+line_width,line_y,7)

  $ advance to next button row
  line_y = line_y+20
  button_index = button_index+1

 $ update display
 flip()
 $ frame delay
 wait(1)