from random import randrange

rec_size = 30
range_of_chars = 122-97
max_len_of_rec = 30
tmp_len = 0

for nb_of_recs in [8, 29, 50, 135, 287, 798, 1297, 2500, 5000]:
	f = open("db" + str(nb_of_recs) + ".txt", "wb")

	for i in range(nb_of_recs):
		my_str = str(i)
		my_bytes = bytes()
		while len(my_str) < 4:
			my_str = "0" + my_str
		f.write(my_str.encode())
		my_str = ""
		length_of_rec = randrange(max_len_of_rec + 1)
		if length_of_rec < 5:
			length_of_rec = randrange(max_len_of_rec + 1)
		if i == 0:
			length_of_rec = 0
		for _ in range(max_len_of_rec - length_of_rec):
			my_bytes += bytes.fromhex('cc')
		f.write(my_bytes)
		for _ in range(length_of_rec):
			my_str += str(chr(97 + (randrange(range_of_chars))))
		
		f.write(my_str.encode())

f.close()