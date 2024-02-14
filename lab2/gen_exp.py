import os
import sys
import glob


input_dir = sys.argv[1] if len(sys.argv) > 1 else 'data'

input_path_pattern = f'{input_dir}/*.tsp'
# modes = ['sa', 'ts']
modes = ['ts']

data_file = lambda mode: f'report/data/{input_dir}_{mode}.csv'
cycle_file = lambda input_file, mode: f'report/cycle/{os.path.basename(input_file)[:-4]}_{mode}.csv'


# init
for mode in modes:
    with open(data_file(mode), 'w') as file:
        file.write('input_data,num_vertices,avg_cycle_weight,min_cycle_weight\n')


# run
def graph_size(file_name):
    base_name = os.path.basename(file_name)
    return int(base_name.split('.')[0][3:])

files = glob.glob(input_path_pattern)
files = sorted(files, key=graph_size)

for mode in modes:
    for file_path in files:
        save_best = f'-b {cycle_file(file_path, mode)}'
        print(f'./tsp -f {file_path} -m {mode} -s {data_file(mode)} {save_best}')
    print()
