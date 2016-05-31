from random import randint

rfile=open("cat_"+str(randint(0, 100)), "wb+")

read_file = open("test", "rb");


rfile.write("AAAFAFAF") # + str(randint(0, 1000)));

rfile.close()
