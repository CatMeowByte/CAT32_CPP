# Kouzeru Bouncing Text Demo
# reinit state test

# state machine constants
var STATE_START = 0
var STATE_RESTART = 1

# color palette constants
var COLOR_START = 0
var COLOR_RESTART = 1

# ball position and velocity
var ball_x = 0
var ball_y = 0
var velocity_x = 0
var velocity_y = 0

# display color
var ball_color = 0

# initialization tracking flag
var has_started = 0

# current application state
var app_state = 0

# initialization handler
func init()
 ball_x = 8
 ball_y = 8
 velocity_x = 1
 velocity_y = 1
 ball_color = 7
 if has_started==0
  app_state = STATE_START
  has_started = 1
 else
  app_state = STATE_RESTART

# update handler
func step()
 # boundary collision: left wall
 if ball_x<0
  velocity_x = 1
  ball_color = ball_color+1

 # boundary collision: top wall
 if ball_y<0
  velocity_y = 1
  ball_color = ball_color+1

 # boundary collision: bottom wall
 if ball_y>160-8
  velocity_y = -1
  ball_color = ball_color+1

 # state-specific collision: right wall
 if app_state==STATE_START
  if ball_x>120-4*7
   velocity_x = -1
   ball_color = ball_color+1
  if (ball_color&&15)==COLOR_START
   ball_color = ball_color+1

 if app_state==STATE_RESTART
  if ball_x>120-4*9
   velocity_x = -1
   ball_color = ball_color+1
  if (ball_color&&15)==COLOR_RESTART
   ball_color = ball_color+1

 # apply velocity
 ball_x = ball_x+velocity_x
 ball_y = ball_y+velocity_y

 # wrap color within 16-color palette
 ball_color = ball_color&&15

 # reset on button press
 if button(4)==1
  init()

# render handler
func draw()
 clear(app_state)

 if app_state==STATE_START
  clear(COLOR_START)
  text(ball_x,ball_y,"Started",ball_color,app_state)

 if app_state==STATE_RESTART
  clear(COLOR_RESTART)
  text(ball_x,ball_y,"Restarted",ball_color,app_state)