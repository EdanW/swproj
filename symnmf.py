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
    
    # switch case for goal
    match goal: # TODO replace with actual functions
        # TODO check if returned null and throw error
        case "symnmf":
            print("symnmf") # c api with symnmf()
        case "sym":
            symMat = np.array(asdf.sym(points))
            print(symMat) # TODO cleanup delete line
        case "ddg":
            ddgMat = np.array(asdf.ddg(points))
            print(ddgMat)
        case "norm":
            normMat = np.array(asdf.norm(points))
            print(normMat)
        case _:
            throwOfficialError()

def pysymnmf(n, k, w):
    m = np.mean(w)
    upperBound = 2 * (np.sqrt(m/k))
    h = np.random.uniform(low=0, high=upperBound, size=(n,k))
    # new H = capi.calcSymnf(h)
    # prints new H accordingly


def throwOfficialError():
    print("An Error Has Occurred")
    sys.exit() #TODO how to terminate properly??




if __name__ == "__main__":
    main()