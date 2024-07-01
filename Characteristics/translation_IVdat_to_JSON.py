import numpy as np
import json
import os


dir_path = "./Characteristics/"
output_path = dir_path + "JSON/"
filter_paths = lambda path: path if path[-3:] == "dat" else None
paths = filter(None, map(filter_paths, os.listdir(dir_path)))
paths = list(paths)
print(paths[0])


for path in paths:

    with open(dir_path + path, "r") as file:
        lines = [line.split("\t") for line in file.readlines()]
        dictionary = {
            "voltage": [line[0] for line in lines],
            "current": [line[1] for line in lines],
            "density current": [line[2][:-2] for line in lines],
        }
    json_data = json.dumps(dictionary, indent=4)
    with open(output_path + path[:-4] + ".json", "w") as jsonfile:
        jsonfile.write(json_data)
