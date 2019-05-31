import pylab as plt
import numpy as np

t = []
alt = []
vz = []
vzf = []
ax = []
ay = []
az = []
a = []
dec = False
tpara = 0
filePath = "I:\data.txt"
dataF = open(filePath, "r")

with open(filePath) as f:
	for ligne in f:
	    lr = str(ligne).replace("'","").replace("b","").replace(r"\n", "").replace(r"\r", "")
	    ls = lr.split(";")
	    t.append(float(ls[0]))
	    alt.append(float(ls[1]))
	    vz.append(float(ls[2]))
	    vzf.append(float(ls[3]))
	    ax.append(float(ls[4]))
	    ay.append(float(ls[5]))
	    az.append(float(ls[6]))
	    a.append(np.sqrt(float(ls[4])**2+float(ls[5])**2+float(ls[6])**2))
	    if(int(ls[7]) == 1 and not dec):
	            	tpara = ls[0]
	            	print("Parachute déclenché à t = "+ls[0]+"s")
	            	dec = True

dataF.close()

plt.subplot(311)
plt.grid()
plt.axvline(x=float(tpara), ymin=0.0, ymax=1.0, color='r', label="Déploiement Parachute")
plt.plot(t,alt,"b", label="Altitude")
plt.legend(loc='upper right')
plt.subplot(312)
plt.grid()
plt.plot(t,vz,"r", label="Vitesse Verticale")
plt.plot(t,vzf,"g", label="Vitesse Verticale filtrée")
plt.legend(loc='upper right')
plt.subplot(313)
plt.plot(t,a,"r", label="Accélération")
plt.legend(loc='upper right')
plt.show()