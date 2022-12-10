from os import system

data = open("data.csv", "w")
data.write("b;number of records;number of runs;number of phases;record reads;record writes;block reads;block writes;record operations;block operations;expected number of phases;expected number of block operations\n")

for block in [2, 4, 8, 16, 32]:
	system("py input_generator.py")
	for nb_of_recs in [8, 29, 50, 135, 287, 798, 1297, 2500, 5000]:
		# number of records, method of filling (1 = FILL_FROM_FILE), mode of printing (0 = START_FINISH),
		# using block operations (1 = true), number of records in a block, name of the file with test data
		name_out = str(block) + "_" + str(nb_of_recs) + ".txt"
		if False:
			system("x64\Debug\BD_P1.exe " + \
				str(nb_of_recs) + " " + \
				str(1) + " " + \
				str(0) + " " + \
				str(1) + " " + \
				str(block) + " " + \
				"db" + str(nb_of_recs) + ".txt" + \
				" > " + name_out)
		row = ""
		out = open(name_out)
		s = out.read()
		out.close()
		s = s.split("statistics:\n")[1]
		s = s.split(" ")
		row += str(block) + ";"
		recs = int(s[0])
		row += str(recs) + ";"
		runs = int(s[2].split("(")[0])
		row += str(runs) + ";"
		ph = int(s[5])
		row += str(ph) + ";"
		recr = int(s[6].split("\n")[1])
		row += str(recr) + ";"
		recw = int(s[9])
		row += str(recw) + ";"
		blockr = int(s[12])
		row += str(blockr) + ";"
		blockw = int(s[15])
		row += str(blockw) + ";"
		recops = int(s[21])
		row += str(recops) + ";"
		blockops = int(s[26])
		row += str(blockops) + ";"
		exph = float(s[33].split("\n")[0].replace(",", "."))
		row += str(exph) + ";"
		exdiscops = float(s[39].split("\n")[0].replace(",", "."))
		row += str(exdiscops) + "\n"

		data.write(row)
		print(name_out + "\n")

data.close()