from random import randint

rfile=open("cat_"+str(randint(0, 100)), "wb+")

read_file = open("test", "rb");

for line in read_file:
    print line

rfile.write("AAAFAFAF" + str(randint(0, 1000)));

rfile.close()
