# Module validation tests

# Test 1: Module with all required arguments
clear(5)
# Expected: push 5, call clear - valid

# Test 2: Module with missing optional (uses default fill=1)
rect(10,20,30,40,7)
# Expected: push 10,20,30,40,7, push 1 (default), call rect - valid

# Test 3: Module with optional override (fill=0 instead of default 1)
rect(0,0,50,50,3,0)
# Expected: push 0,0,50,50,3,0, call rect - valid, uses provided 0 not default 1

# Test 4: Module with too few arguments (error)
# pixel(10,20)
# Expected: error - pixel needs 3, got 2

# Test 5: Module with too many arguments (error)
# flip(99)
# Expected: error - flip needs 0, got 1

# Test 6: Module zero-argument valid call
flip()
# Expected: call flip - valid

# User function validation tests

# Test 7: Define function with required only
func summary(a,b):
 return a+b

# Test 8: Define function with optional parameter
func multiply(x,y,z=10):
 return x*y*z

# Test 9: Define function with zero arguments
func getvalue():
 return 42

# Test 10: Call user function with exact required args
see(summary(3,5))
# Expected: push 3,5, subgo summary, call see - valid, result 8

# Test 11: Call user function missing optional (uses default z=10)
see(multiply(2,3))
# Expected: push 2,3, push 10 (default), subgo multiply, call see - valid, result 60

# Test 12: Call zero-argument function
see(getvalue())
# Expected: subgo getvalue, call see - valid, result 42

# Test 13: Call user function with too few required args (error)
# see(summary(7))
# Expected: error - summary needs 2, got 1

# Test 14: Call user function with too many args (error)
# see(multiply(1,2,3,4))
# Expected: error - multiply needs 3, got 4

# Test 15: Nested calls with modules
line(10,10,20,20,15)
# Expected: push 10,10,20,20,15, call line - valid

# Test 16: Check the safety of argument optional symbol in regular context
var apple = 5
var banana = 3*8
if banana>apple:
 text(64,64,"OK",1,12)

# all these unused symbols should not produce any token at all
$ ^ ; : ?

flip()