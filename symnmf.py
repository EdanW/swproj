# imports
import asdf
import sys
import numpy as np

# default values - max iter, eps?

def main():
    np.random.seed(0)
    argv = sys.argv
    # no need to validate, assumption 2
    k = int(argv[1])
    goal = argv[2]
    path = argv[3]
    points = np.genfromtxt(path, delimiter=',').tolist()
    
    resMat = []
    # switch case for goal
    match goal: # TODO replace with actual functions
        # TODO check if returned null and throw error
        case "symnmf":
            print("symnmf")
            w = asdf.norm(points) # should we wrap with np.array?
            n = 100 # TODO make a GET N func
            resMat = pysymnmf(n, k, w) # add n, k
            print(resMat)
        case "sym":
            resMat = np.array(asdf.sym(points))
        case "ddg":
            resMat = np.array(asdf.ddg(points))
        case "norm":
            resMat = np.array(asdf.norm(points))
        case _:
            throwOfficialError()
    print (resMat) #TODO special print resMat


def pysymnmf(n, k, w):
    m = np.mean(w)
    upperBound = 2 * (np.sqrt(m/k))
    h = np.random.uniform(low=0, high=upperBound, size=(n,k))
    H = asdf.symnmf(h, w, n, k)
    return H


def throwOfficialError():
    print("An Error Has Occurred")
    sys.exit() #TODO how to terminate properly??


if __name__ == "__main__":
    main()