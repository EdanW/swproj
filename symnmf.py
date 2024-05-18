# imports
# import c module named symnmf
import sys

# default values - max iter, eps?

def main():
    argv = sys.argv
    ## no need to validate, assumption 2
    k = int(argv[1])
    goal = argv[2]
    path = argv[3]


    # switch case for goal
    match goal: # TODO replace with actual functions
        case "symnmf":
            print("symnmf") # c api with symnmf()
        case "sym":
            print("sym")
        case "ddg":
            print("ddg")
        case "norm":
            print("norm")
        case _:
            throwOfficialError()

def throwOfficialError():
    print("An Error Has Occurred")
    sys.exit() #TODO how to terminate properly??




if __name__ == "__main__":
    main()