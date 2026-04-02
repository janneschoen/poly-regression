import matplotlib.pyplot as plt
import numpy as np
import sys


def main():

    inputs = np.loadtxt(sys.argv[1])
    responses = np.loadtxt(sys.argv[2])
    predictions = np.loadtxt(sys.argv[3])
    
    plt.scatter(inputs[:len(responses)], responses)
    plt.plot(inputs, predictions, color = "grey")
    plt.grid(True)

    plt.xlabel('Input')
    plt.ylabel('Response / Prediction')
    plt.title('Observations and Regression')

    plt.show()


if __name__ == "__main__":
    main()