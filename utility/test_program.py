from random import randint

rfile=open("cat_"+str(randint(0, 100)), "wb+")

rfile.write("AAAFAFAF");

rfile.close()
