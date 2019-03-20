inp = open("Arduino_Input.txt", "r")
out = open("Python_Output.txt", "w")

out.write("Time [s]    Amp [A]\n")

line_cnt = 0
for line in inp:
    line_cnt += 1
    try:
        line_list = []
        line_list = line.strip().split()
        if len(line_list[0]) < 7 or len(line_list[0]) >15 or len(line_list[1]) < 3 or len(line_list[1]) >8:
            continue
        if float(line_list[0]) > 250:
            continue
        out.write(line_list[0]+"   "+line_list[1]+'\n')
    except:
        continue
print("Done!")
    
    
inp.close()
out.close()