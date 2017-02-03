def costdis(x,y,x_start,y_start):
	import matplotlib.pyplot as plt
	print "in python code: "
	print "x: ", x
	print "y: ", y
	print "x_start: ",x_start
	print "y_start: ",y_start
	for i in range(len(x)):
		plt.plot(x[i],y[i]);
		plt.plot(x_start[i],y_start[i],'ro');
	plt.show();
	#print x
	return x

def cost_time(x):
	import matplotlib.pyplot as plt
	print "in python code: "
	print x
	plt.plot(x)
	plt.show()
	return x

