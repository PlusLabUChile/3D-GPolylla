import random
import getopt, sys
import numpy as np
from math import sqrt

def write_points(xPoints, yPoints, zPoints):
    largo =len(xPoints)
    print("{} 3 0 0".format(largo))
    for i in range(0,largo):
        print('{0} {1} {2} {3}'.format(i, xPoints[i], yPoints[i], zPoints[i]))
    print('\n')

def generate_semi_uniform_points(nPoints, xPoints, yPoints, zPoints, dist):
    uwu = int(np.sqrt(nPoints))
    for i in range(uwu):
        for j in range(uwu):
            for z in range(uwu):
                xPoints.append(j + random.uniform(-dist, dist))
                yPoints.append(i + random.uniform(-dist, dist))
                zPoints.append(z + random.uniform(-dist, dist))

 #Input Largo inicial, incremento
def main():
    full_cmd_arguments = sys.argv
    argument_list = full_cmd_arguments[1:]

    nPoints = int(argument_list[0])

    random.seed(138)
    xPoints = []
    yPoints = []
    zPoints = []

    dist = nPoints*0.7
    #dist = 0
    generate_semi_uniform_points(nPoints, xPoints, yPoints, zPoints, dist)

    write_points(xPoints, yPoints, zPoints)
    

if __name__ == "__main__":
    main()
