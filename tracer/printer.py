from random import randint

ff= open("f"+str(randint(1, 10)), "a")

ff.write( str(randint(1, 100)))

ff.close()
