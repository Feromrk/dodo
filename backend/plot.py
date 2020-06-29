import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as md
import datetime as dt

data = np.genfromtxt('db.csv', delimiter=',', names=['x', 'y'], usecols=np.arange(0,4))


fig = plt.figure()

ax1 = fig.add_subplot(111)

xfmt = md.DateFormatter('%d.%m %H:%M')
ax1.xaxis.set_major_formatter(xfmt)

plt.xticks( rotation=25 )

ax1.set_title("Temperaturen im Hühnerhaus")    
ax1.set_xlabel('Zeit')
ax1.set_ylabel('Celsius')

x = [dt.datetime.fromtimestamp(ts) for ts in data['x']]

ax1.plot(x,data['y'], c='r', label='innen')

leg = ax1.legend()

plt.show()