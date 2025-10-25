# test bracket offset syntax

stripe data = 10 20 30 40 50

# literal index
see(data[0])
see(data[2])

# variable index
var i = 1
see(data[i])

# expression index
see(data[i+1])
see(data[i*2+1])

# complex expression
var offset = 0
see(data[i+offset])

# nested expression
see(data[(i+1)*2])

# expected output:
# SEE(10) = 000A.0000 | 655360
# SEE(30) = 001E.0000 | 1966080
# SEE(20) = 0014.0000 | 1310720
# SEE(30) = 001E.0000 | 1966080
# SEE(40) = 0028.0000 | 2621440
# SEE(20) = 0014.0000 | 1310720
# SEE(50) = 0032.0000 | 3276800