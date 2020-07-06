import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as md
import datetime as dt

data = np.genfromtxt('db.csv', delimiter=',', names=['timestamps', 'temp_in', 'temp_out'], usecols=np.arange(0,4))


fig = plt.figure()

ax1 = fig.add_subplot(111)

xfmt = md.DateFormatter('%d.%m %H:%M')
ax1.xaxis.set_major_formatter(xfmt)

plt.xticks( rotation=25 )

ax1.set_title("Temperaturen im Hühnerhaus")    
ax1.set_xlabel('Zeit')
ax1.set_ylabel('Celsius')

x = [dt.datetime.fromtimestamp(ts) for ts in data['timestamps']]
temp_out = [(temp_out if temp_out > 0 else None) for temp_out in data['temp_out']]

ax1.plot(x,data['temp_in'], c='r', label='innen')
ax1.plot(x,temp_out, c='b', label='außen')

leg = ax1.legend()

plt.show()