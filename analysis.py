import symnmf
import sys
import numpy as np
import pandas as pd 
import sklearn.metrics as sk
import math



def main():
    if len(sys.argv) != 3:
        throwOfficialError()
    
    k = int(sys.argv[1])
    path = sys.argv[2]

    try:
        points = np.genfromtxt(path, delimiter=',').tolist()
        H = analSymnmf(points, len(points), k)
        

        # label of a point = which cluster it belongs to.
        symnmf_clusters = np.argmax(H, axis=1)
        kmeans_centroids = analKmeans(points, k)
        kmeans_clusters = [find_closest_centroid(point, kmeans_centroids) for point in points]

        symnmf_score = sk.silhouette_score(points, symnmf_clusters)
        kmeans_score = sk.silhouette_score(points, kmeans_clusters)

        print(kmeans_clusters)
        print(symnmf_clusters)
        print("nmf: {:.4f}".format(symnmf_score))
        print("kmeans: {:.4f}".format(kmeans_score))


    except Exception as e:
        throwOfficialError()


def analSymnmf(points, n, k):
    w = symnmf.norm(points)
    n = len(points)
    m = np.mean(w)
    upperBound = 2 * (np.sqrt(m/k))
    h = np.random.uniform(low=0, high=upperBound, size=(n,k)).tolist()
    return symnmf.symnmf(h, w, n, k)


def analKmeans(points, k):
    # taken from hw1 code
    EPSILON = 0.0001
    MAX_ITER = 300

    centroids = []
    for i in range(k):
        centroids.append(points[i])

    d = len(points[0])
    for i in range(MAX_ITER):
        clusters = [[] for i in range(k)]
        converged = True
        for p in points:
            clusters[find_closest_centroid(p, centroids)].append(p)
        for j in range(k):
            new_centroid = find_cluster_mean(clusters[j], d)
            if mydist(centroids[j], new_centroid) >= EPSILON:
                converged = False
            centroids[j] = new_centroid
        if converged:
            break
    
    return centroids


def throwOfficialError():
    print("An Error Has Occurred")
    sys.exit(1)


def find_closest_centroid(p, centroids):
    min_dist = mydist(p, centroids[0])
    min_dist_index = 0
    for i in range(1, len(centroids)):
        dist = mydist(p, centroids[i])
        if dist < min_dist:
            min_dist_index = i
            min_dist = dist
    return min_dist_index


def find_cluster_mean(points, d):
    mean = []
    num_of_points = len(points)
    for i in range(d):
        dim_sum = 0
        for p in points:
            dim_sum += p[i]
        mean.append(dim_sum / num_of_points)
    return mean


def mydist(point1, point2):
    if len(point1) != len(point2):
        print("An Error Has Occurred")
        exit()

    squared_distances = [(p1 - p2) ** 2 for p1, p2 in zip(point1, point2)]
    sum_of_squared_distances = sum(squared_distances)
    distance = math.sqrt(sum_of_squared_distances)
    return distance

if __name__ == "__main__":
    main()
