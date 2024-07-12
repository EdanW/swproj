# imports
import symnmf
import sys
import numpy as np

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
    match goal:
        case "symnmf":
            w = symnmf.norm(points)
            n = len(points)
            resMat = pysymnmf(n, k, w)
        case "sym":
            resMat = np.array(symnmf.sym(points))
        case "ddg":
            resMat = np.array(symnmf.ddg(points))
        case "norm":
            resMat = np.array(symnmf.norm(points))
        case _:
            throwOfficialError()
    
    if resMat is None:
        throwOfficialError()
    officialPrint(resMat)


def pysymnmf(n, k, w):
    m = np.mean(w)
    upperBound = 2 * (np.sqrt(m/k))
    h = np.random.uniform(low=0, high=upperBound, size=(n,k)).tolist()
    H = symnmf.symnmf(h, w, n, k)
    return H

def officialPrint(mat):
    # Convert the numpy array to a list of strings
    rows_as_strings = [','.join(f"{x:.4f}" for x in row) for row in mat]
    csv_string = '\n'.join(rows_as_strings)
    print(csv_string)

def throwOfficialError():
    print("An Error Has Occurred")
    sys.exit(1)


if __name__ == "__main__":
    main()