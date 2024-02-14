import os
import sys
import glob
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt
import subprocess


input_dir = sys.argv[1] if len(sys.argv) > 1 else 'data'

# utility
input_path_pattern = f'{input_dir}/*.tsp'
modes = ['rst', 'rpc', 'rnf']

data_file = lambda mode: f'report/data/data_{mode}.csv'
cycle_file = lambda input_file: f'report/cycle/{os.path.basename(input_file)[:-3]}csv'


# init
for mode in modes:
    with open(data_file(mode), 'w') as file:
        file.write('input_data,num_vertices,mst_weight,avg_cycle_weight,min_cycle_weight,avg_improvements\n')

process = subprocess.run('make', stdout=subprocess.PIPE, text=True)
print(process.stdout)


# run
def sort_key(file_name):
    base_name = os.path.basename(file_name)
    return int(base_name.split('.')[0][3:])

files = glob.glob(input_path_pattern)
files = sorted(files, key=sort_key)

for file_path in files:
    print(f'running: {file_path}')
    for mode in modes:
        print(f'{mode = }')
        save_best = f'-b {cycle_file(file_path)}' if mode == 'rst' else ''
        process = subprocess.run(
            f'./tsp -f {file_path} -m {mode} -s {data_file(mode)} {save_best}'.split(),
            stdout=subprocess.PIPE, text=True
        )
        print(f'{process.stdout}')
    print()


# clean
process = subprocess.run('make clean'.split(), stdout=subprocess.PIPE, text=True)
print(process.stdout)