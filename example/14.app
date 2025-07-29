$ Hi, all!
$ this is supposed to be comment
$ and also mark the new era of tokenizer that process a better looking code
$ unfortunately, the limitation that expressions must be 1 token still apply
$ as a side effect, all opcode now must be written in lowercase
$ (to be fixed later with proper function call)

var foo = 8
var bar = -foo
var baz = -12

clear(0)
text(4,4,"Hi, all!",7)
text(2,16,"i am a $ clever tokenizer!",12) $ clever because it strip comment only if outside quote
flip()

$ operation order
see(-add(1,-(5-2)))