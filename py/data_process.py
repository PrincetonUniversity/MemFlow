import matplotlib.pyplot as plt

def tem():
	f = open("temperature.txt",'r')
	tem = []
	for line in f:
	   tem.append(float(line))
	
	plt.plot(tem)
	plt.show()

def process():
	f = open("iteration_results.txt",'r')
	cost = []
	for line in f:
	   cost_t = [int(n) for n in line.split()]
	   cost.append(cost_t)

	cost_whole = []
	for cost_t in cost:
	   for cost_it in cost_t:
	      cost_whole.append(cost_it)

	dis_cost = []
	dis_p = []
	for cost_t in cost:
	   dis_t = {}
	   for cost_it in cost_t:
	      if cost_it in dis_t.keys():
	         dis_t[cost_it] += 1
	      else:
	         dis_t[cost_it] = 1
	   dis_cost_t = sorted(dis_t.keys())
	   dis_p_t = []
	   for key in dis_cost_t:
	      dis_p_t.append((float)(dis_t[key])/len(cost_t))
	   dis_cost.append(dis_cost_t)
	   dis_p.append(dis_p_t)	

	print cost[0]

	print dis_cost[0]
	print dis_p[0]

	#plt.plot(dis_cost[0], dis_p[0])
	plt.plot(cost_whole)
	plt.show()
	
	   

if __name__ == "__main__":
	process()



