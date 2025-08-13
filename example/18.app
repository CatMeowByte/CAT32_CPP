$ variable scope
$ expect compile error

var foo = -12.5

if 1
 var local = foo*2

 $ should be valid
 see(local)

$ should be invalid
see(local)