import glob
file_list = glob.glob("C:\\Users\\yido\\Desktop\\Technion\\HW_2\\outputs\\*")

def get_data(filename):
	f = file(filename,'r').read()
	u_index = f.find("first_end_time/second_end_time is:")
	u = f[u_index+34:].split('\n')[0]
	rt_index = f.find("runtime is: ")
	rt = str(float(f[rt_index+12:].split('\n')[0])) # time in sec
	return u, rt

txt = '"log number","prev pid","next pid","prev priority","next priority","prev policy","next policy","switch time","Number of tickets"\n'
b = file('C:\\Users\\yido\\Desktop\\Technion\\HW_2\\outputs\\out_100.txt','r').read()
for i in b.splitlines():
	if ('log_output' in i) and ('.' not in i):
		txt += i.replace("log_output[",'').replace("] is:",'')+','
	if ('prev_pid' in  i) or ('next_pid' in  i) or ('prev_priority' in  i) or ('next_priority' in  i) or ('prev_policy' in  i) or ('next_policy' in  i) or ('switch_time' in  i) or ('NT' in  i):
		txt += i.split(' ')[-1]+','
	if 'NT' in i:
		txt = txt[:-1]
		txt += '\n'

# txt = 'U,"Run Time"\n'
#
# for i in file_list:
# 	d = get_data(i)
# 	txt += d[0]+','+d[1]+'\n'
#
# file("C:\\Users\\yido\\Desktop\\Technion\\HW_2\\results.csv",'w').write(txt)